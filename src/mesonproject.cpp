#include "mesonproject.h"
#include "constants.h"
#include "filelistnode.h"
#include "nodes.h"
#include "mesonbuildconfiguration.h"
#include "mesonprojectimporter.h"

#include <coreplugin/documentmanager.h>
#include <coreplugin/fileiconprovider.h>
#include <coreplugin/icontext.h>
#include <coreplugin/icore.h>
#include <coreplugin/messagemanager.h>

#include <cpptools/cpptoolsconstants.h>
#include <cpptools/cppmodelmanager.h>
#include <cpptools/projectinfo.h>
#include <cpptools/cpprawprojectpart.h>
#include <cpptools/cppprojectupdater.h>
#include <cpptools/cppkitinfo.h>

#include <projectexplorer/projectimporter.h>
#include <projectexplorer/buildinfo.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/target.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/task.h>
#include <projectexplorer/taskhub.h>

#include <utils/synchronousprocess.h>

#include <QHash>
#include <QSaveFile>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <iostream>
#include <memory>

#include "mesonconfigurationdialog.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>

#include "treebuilder.h"

namespace MesonProjectManager {

quint64 qHash(const CompileCommandInfo &info)
{
    return qHash(info.includes)^qHash(info.cpp_std);
}

bool CompileCommandInfo::operator==(const CompileCommandInfo &o) const
{
    return std::tie(defines, includes, cpp_std, id, simplifiedCompilerParameters)
            == std::tie(o.defines, o.includes, o.cpp_std, id, o.simplifiedCompilerParameters);
}

MesonProject::MesonProject(const Utils::FileName &filename):
    Project(PROJECT_MIMETYPE, filename), filename(filename), cppCodeModelUpdater(new CppTools::CppProjectUpdater())
{
    setId(MESONPROJECT_ID);
    setProjectLanguages(Core::Context(ProjectExplorer::Constants::CXX_LANGUAGE_ID));
    setDisplayName(this->filename.fileName(1).section("/", 0, 0));

    connect(this, &ProjectExplorer::Project::activeTargetChanged, this, &MesonProject::refresh);

    refresh();
}

MesonProject::~MesonProject()
{
    delete cppCodeModelUpdater;
}

MesonBuildConfiguration *MesonProject::activeBuildConfiguration()
{
    ProjectExplorer::Target *active = activeTarget();
    if (!active)
        return nullptr;

    ProjectExplorer::BuildConfiguration *bc = active->activeBuildConfiguration();
    if (!bc)
        return nullptr;

    MesonBuildConfiguration *cfg = qobject_cast<MesonBuildConfiguration*>(bc);

    return cfg;
}

void MesonProject::refresh()
{
    if (isParsing()) {
        // file change handling is racy and actually calls refresh even on expected changes :/
        return;
    }
    emitParsingStarted();

    MesonBuildConfiguration *cfg = activeBuildConfiguration();

    auto root = std::make_unique<MesonRootProjectNode>(this);

    IntroProject introProject = mesonIntrospectProjectInfoFromSource(cfg);

    projectDocuments.clear();
    for(const QString &name: introProject.buildsystemFiles) {
        const QString file = introProject.baseDir + "/" + name;

        projectDocuments.emplace_back(std::make_unique<ProjectExplorer::ProjectDocument>(MesonProjectManager::PROJECT_MIMETYPE, filename, [this] {
            refresh();
        }));
    }

    QString buildDir;
    if (cfg) {
        QMap<QString, IntroSubProject*> lookup;
        for(const QString &f: introProject.buildsystemFiles) {
            lookup.insert(f, &introProject);
        }
        for(IntroSubProject &sub: introProject.subprojects) {
            for(const QString &f: sub.buildsystemFiles) {
                lookup.insert(f, &sub);
            }
        }

        QVector<TargetInfo> targetInfos = readMesonInfoTargets(*cfg);
        for(const TargetInfo &target: targetInfos) {
            if(!lookup.contains(target.definedIn))
                continue;

            IntroSubProject *p = lookup.value(target.definedIn);
            p->targets.append(target);
        }

        buildDir = cfg->buildDirectory().toString();
    }

    TreeBuilder builder(this, introProject);
    builder.build(root.get(), buildDir);

    QVector<CompileCommandInfo> codeModelInfos;
    if (cfg) {
        PathResolver::DirectoryInfo projectBaseDirectoryInfo = pathResolver.getForPath(filename.toFileInfo().absolutePath());
        QVector<TargetInfo> targetInfos = readMesonInfoTargets(*cfg);
        codeModelInfos = parseCompileCommands(*cfg, targetInfos);
        codeModelInfos = rewritePaths(projectBaseDirectoryInfo, codeModelInfos);
    }

    setRootProjectNode(move(root));

    refreshCppCodeModel(codeModelInfos);

    emitParsingFinished(true);
}

IntroProject MesonProject::mesonIntrospectProjectInfoFromSource(const MesonBuildConfiguration *cfg)
{
    IntroProject project;

    QString mesonPath;
    if(cfg)
        mesonPath = cfg->mesonPath();
    else
        mesonPath = findDefaultMesonExecutable().toString();

    Utils::SynchronousProcess proc;
    proc.setTimeoutS(100);
    auto response = proc.run(mesonPath, {"introspect", filename.toString(), "--projectinfo"});
    if (response.exitCode!=0) {
        ProjectExplorer::TaskHub::addTask(ProjectExplorer::Task::Error,
                                          QStringLiteral("Can't introspect projectinfo. %1")
                                          .arg(response.exitMessage(mesonPath, proc.timeoutS())),
                                          ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM,
                                          filename);

        Core::MessageManager::write(response.allOutput());
        return project;
    }
    QJsonObject json = QJsonDocument::fromJson(response.allRawOutput()).object();

    const QString projectName = json.value("descriptive_name").toString();
    if(projectName.length()) {
        setDisplayName(projectName);
    }

    auto addFolderPrefix = [](const QStringList &list, const QString &prefix) -> QStringList {
        QDir d(prefix);
        QStringList result;
        for (const QString &entry: list) {
            if(QFileInfo(entry).isAbsolute()) {
                result.append(entry);
            } else {
                result.append(d.filePath(entry));
            }
        }
        return result;
    };

    project.name = projectName;
    project.version = json.value("version").toString();
    project.baseDir = filename.toFileInfo().absolutePath();
    project.buildsystemFiles = addFolderPrefix(jsonArrayToStringList(json.value("buildsystem_files").toArray()), project.baseDir);
    project.subprojectsDir = json.value("subproject_dir").toString();

    const QJsonArray subprojectsArray = json.value("subprojects").toArray();
    for(const QJsonValue &value: subprojectsArray) {
        QJsonObject s = value.toObject();
        IntroSubProject sub;
        sub.name = s.value("name").toString();
        sub.version = s.value("version").toString();
        sub.baseDir = project.baseDir + "/" + project.subprojectsDir + "/" + sub.name;
        sub.buildsystemFiles = addFolderPrefix(jsonArrayToStringList(s.value("buildsystem_files").toArray()), project.baseDir);
        project.subprojects.append(sub);
    }

    return project;
}

Utils::FileName MesonProject::findDefaultMesonExecutable()
{
    Utils::FileName mesonBin = Utils::Environment::systemEnvironment().searchInPath("meson.py");
    if (mesonBin.isEmpty()) {
        mesonBin = Utils::Environment::systemEnvironment().searchInPath("meson");
        if (mesonBin.isEmpty()) {
            ProjectExplorer::TaskHub::addTask(ProjectExplorer::Task::Error,
                                              QStringLiteral("Meson execuatble not found in PATH"),
                                              ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM);
        }
    }
    return mesonBin;
}

void MesonProject::regenerateProjectFiles(MesonBuildFileParser *parser)
{
    Core::FileChangeBlocker changeGuard(parser->filename);
    QByteArray out=parser->regenerate();
    Utils::FileSaver saver(parser->filename, QIODevice::Text);
    if(!saver.hasError()) {
        saver.write(out);
    }
    saver.finalize(Core::ICore::mainWindow());
}

QStringList MesonProject::jsonArrayToStringList(const QJsonArray arr)
{
    QStringList strings;
    for (const QJsonValue val: arr) {
        strings.append(val.toString());
    }
    return strings;
}

const QVector<CompileCommandInfo> MesonProject::parseCompileCommands(const MesonBuildConfiguration &cfg, const QVector<TargetInfo> &targetInfos) const
{
    QVector<CompileCommandInfo> compileCommandInfos;

    for (const TargetInfo &info: targetInfos) {
        for (const SourceSetInfo &ssi: info.sourceSets) {
            CompileCommandInfo info;
            info.files = ssi.sources + ssi.generatedSources;

            bool nextIsOutput = false;

            for (const QString &part: ssi.parameters) {
                /*
                "c++  -Irule_system@exe -I. -I.. -I../ChaiScript-6.0.0/include -fdiagnostics-color=always
                -pipe -D_FILE_OFFSET_BITS=64 -Wall -Winvalid-pch -Wnon-virtual-dtor -O0 -g -std=c++14 -pthread
                -MMD -MQ 'rule_system@exe/main.cpp.o' -MF 'rule_system@exe/main.cpp.o.d'
                -o 'rule_system@exe/main.cpp.o' -c ../main.cpp"
                */

                if (nextIsOutput) {
                    QStringList pp = part.split('/');
                    for (const QString &p: pp) {
                        if (p.contains('@')) {
                            info.id = p;
                            break;
                        }
                    }
                    nextIsOutput = false;
                    continue;
                }

                if (!part.startsWith("-"))
                    continue;

                if (part.startsWith("-D")) {
                    const QString value = part.mid(2);
                    info.defines.insert(value.section("=", 0, 0), value.section("=", 1, -1));
                } else if (part.startsWith("-U")) {
                    info.defines.remove(part.mid(2));
                } else if (part.startsWith("-I")) {
                    QString idir = part.mid(2);
                    if (!idir.startsWith("/")) {
                        idir = cfg.buildDirectory().toString() + "/" + idir;
                    }
                    if (!idir.endsWith("/"))
                        idir.append("/");
                    info.includes.append(idir);
                } else if (part.startsWith("-std")) {
                    info.cpp_std = part.mid(5);
                } else if (part == "-o") {
                    nextIsOutput = true;
                }

                if (!part.startsWith("-o") // output file
                        && part != "-c" // compilation mode
                        && part != "-pipe" // noise
                        && !part.startsWith("-M")) { // file specific dependency output flags
                    info.simplifiedCompilerParameters.append(part);
                }
            }
            compileCommandInfos.append(info);
        }
    }

    return compileCommandInfos;
}

QVector<CompileCommandInfo> MesonProject::rewritePaths(const PathResolver::DirectoryInfo &base, const QVector<CompileCommandInfo> &input) const
{
    QVector<CompileCommandInfo> out;

    auto processStringList = [this,base](const QStringList &list)
    {
        QStringList out;

        for(const QString &str: list) {
            out.append(pathResolver.getIntendedFileName(base, str));
        }

        return out;
    };

    for (CompileCommandInfo cci: input) {
        cci.includes = processStringList(cci.includes);
        cci.files = processStringList(cci.files);
        out.append(cci);
    }

    return out;
}

QVector<TargetInfo> MesonProject::readMesonInfoTargets(const MesonBuildConfiguration &cfg)
{
    Utils::FileName mesonInfoTargetsFileName = cfg.buildDirectory().appendPath("meson-info/intro-targets.json");
    if (!mesonInfoTargetsFileName.exists()) {
        return {};
    }

    QFile data(mesonInfoTargetsFileName.toString());
    if (!data.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QMap<QString, TargetType> allowedTargetTypes = {
        {"executable", TargetType::Executable},
        {"shared library", TargetType::DynamicLibrary},
        {"static library", TargetType::StaticLibrary},
        {"shared module", TargetType::SharedModule},
    };

    QVector<TargetInfo> targets;
    QJsonArray rawTargets = QJsonDocument::fromJson(data.readAll()).array();
    for (const QJsonValue entry: rawTargets) {
        const QJsonObject target = entry.toObject();
        if (!allowedTargetTypes.contains(target.value("type").toString()))
            continue;
        TargetInfo t;
        t.targetName = target.value("name").toString();
        t.targetId = target.value("id").toString();
        t.type = allowedTargetTypes.value(target.value("type").toString());
        t.definedIn = target.value("defined_in").toString();

        QJsonArray sourceSets = target.value("target_sources").toArray();
        for(const QJsonValue entry: sourceSets) {
            const QJsonObject set = entry.toObject();
            SourceSetInfo s;
            s.language = set.value("language").toString();
            s.compiler = jsonArrayToStringList(set.value("compiler").toArray());
            s.parameters = jsonArrayToStringList(set.value("parameters").toArray());
            s.sources = jsonArrayToStringList(set.value("sources").toArray());
            s.generatedSources = jsonArrayToStringList(set.value("generated_sources").toArray());
            t.sourceSets.append(s);
        }
        targets.append(t);
    }

    return targets;
}

/*bool MesonProject::supportsKit(ProjectExplorer::Kit *k, QString *errorMessage) const
{
    Q_UNUSED(k)
    Q_UNUSED(errorMessage)
    return true;
}*/

bool MesonProject::setupTarget(ProjectExplorer::Target *t)
{
    return true;
}

QStringList MesonProject::filesGeneratedFrom(const QString &sourceFile) const
{
    return {};
}

bool MesonProject::needsConfiguration() const
{
    return false;
}

void MesonProject::configureAsExampleProject(const QSet<Core::Id> &platforms)
{

}

/*bool MesonProject::requiresTargetPanel() const
{
    return false;
}*/

void MesonProject::refreshCppCodeModel(const QVector<CompileCommandInfo> &fileCodeCompletionHints)
{
    const ProjectExplorer::Kit *k = nullptr;
    if (ProjectExplorer::Target *target = activeTarget())
        k = target->kit();
    else
        k = ProjectExplorer::KitManager::defaultKit();
    //QTC_ASSERT(k, return);

    ProjectExplorer::ToolChain *cToolChain
            = ProjectExplorer::ToolChainKitInformation::toolChain(k, ProjectExplorer::Constants::C_LANGUAGE_ID);
    ProjectExplorer::ToolChain *cxxToolChain
            = ProjectExplorer::ToolChainKitInformation::toolChain(k, ProjectExplorer::Constants::CXX_LANGUAGE_ID);

    cppCodeModelUpdater->cancel();

    CppTools::ProjectPart::QtVersion activeQtVersion = CppTools::ProjectPart::Qt5;

    QVector<CppTools::RawProjectPart> parts;
    int i=1;
    for (const CompileCommandInfo &info: fileCodeCompletionHints) {
        CppTools::RawProjectPart rpp;
        rpp.setDisplayName(info.id + QStringLiteral(" Part %1").arg(QString::number(i)));
        rpp.setProjectFileLocation(projectFilePath().toString());
        rpp.setQtVersion(activeQtVersion);
        rpp.setIncludePaths(info.includes);
        QVector<ProjectExplorer::Macro> macros;
        for (const auto &key: info.defines.keys())
            macros.append(ProjectExplorer::Macro(key.toUtf8(), info.defines[key].toUtf8()));
        rpp.setMacros(macros);
        CppTools::RawProjectPartFlags rppf{cxxToolChain, info.simplifiedCompilerParameters};
        rpp.setFlagsForCxx(rppf);
        rpp.setFlagsForC(rppf);
        rpp.setFiles(info.files);

        parts.append(rpp);
        i++;
    }

    CppTools::KitInfo kitInfo(this);
    kitInfo.cToolChain = cToolChain;
    kitInfo.cxxToolChain = cxxToolChain;

    const CppTools::ProjectUpdateInfo projectInfoUpdate(this, kitInfo, parts);
    cppCodeModelUpdater->update(projectInfoUpdate);
}

bool MesonProject::canConfigure()
{
    return activeBuildConfiguration()!=nullptr;
}

void MesonProject::editOptions()
{
    Utils::SynchronousProcess proc;
    proc.setTimeoutS(100);
    MesonBuildConfiguration *cfg_ptr = activeBuildConfiguration();
    if(!cfg_ptr) {
        return;
    }
    MesonBuildConfiguration &cfg = *cfg_ptr;
    const Utils::FileName buildDir = cfg.buildDirectory();
    const bool buildConfigured = Utils::FileName(buildDir).appendPath("meson-private").appendPath("coredata.dat").exists();
    const QString pathArg = buildConfigured ? buildDir.toString() : filename.toString();
    auto response = proc.run(cfg.mesonPath(), {"introspect", "--buildoptions", pathArg});
    if (response.exitCode!=0) {
        QMessageBox::warning(nullptr, tr("Can't get buildoptions"), response.exitMessage(cfg.mesonPath(), proc.timeoutS())+"\n"+response.stdOut()+"\n\n"+response.stdErr());
        return;
    }
    const QString json = response.stdOut();
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    MesonConfigurationDialog *dlg = new MesonConfigurationDialog(doc.array(), cfg.project()->displayName(), buildDir.toString(), !buildConfigured);
    dlg->show();

    connect(dlg, &QDialog::accepted, [this, dlg, buildConfigured]{
        MesonBuildConfiguration &cfg = *activeBuildConfiguration();
        QStringList args;
        if(buildConfigured) {
            args = QStringList{"configure", cfg.buildDirectory().toString()};
        } else {
            args = QStringList{"setup", filename.toFileInfo().absolutePath(), cfg.buildDirectory().toString()};
        }

        for(const QJsonObject &obj: dlg->getChangedValues()) {
            const QString name = obj.value("name").toString();
            const QString type = obj.value("type").toString();
            const QJsonValue value = obj.value("value");

            if (type == "array") {
                QStringList arrayArgs;
                for(const QJsonValue val: value.toArray()) {
                    arrayArgs.append(val.toString().replace("'", "'\\''"));
                }
                if(arrayArgs.isEmpty())
                    args.append(QStringLiteral("-D%1=").arg(name));
                else
                    args.append(QStringLiteral("-D%1='%2'").arg(name, arrayArgs.join("' '")));
            } else if (type == "boolean") {
                const QString valueStr = value.toBool()?QStringLiteral("true"):QStringLiteral("false");
                args.append(QStringLiteral("-D%1=%2").arg(name, valueStr));
            } else if (type == "combo") {
                args.append(QStringLiteral("-D%1=%2").arg(name, value.toString()));
            } else if (type == "integer") {
                args.append(QStringLiteral("-D%1=%2").arg(name, QString::number(value.toInt())));
            } else if (type == "string") {
                args.append(QStringLiteral("-D%1=%2").arg(name, value.toString()));
            }
        }

        Utils::SynchronousProcess proc;
        proc.setTimeoutS(100);
        auto response = proc.run(cfg.mesonPath(), args);
        if (response.exitCode!=0) {
            QMessageBox::warning(nullptr, tr("Can't configure meson build"), args.join("<->")+"\n\n"+response.exitMessage(cfg.mesonPath(), proc.timeoutS())+"\n"+response.stdOut()+"\n\n"+response.stdErr());
            return;
        }
    });
}

ProjectExplorer::ProjectImporter *MesonProject::projectImporter() const
{
    return new MesonProjectImporter(projectFilePath());
}

ProjectExplorer::Project::RestoreResult MesonProject::fromMap(const QVariantMap &map, QString *errorMessage)
{
    RestoreResult result = Project::fromMap(map, errorMessage);
    if (result != RestoreResult::Ok)
        return result;
    return RestoreResult::Ok;
}

QStringList getAllHeadersFor(const QString &fname)
{
    QStringList out;
    const QStringList *exts = nullptr;
    QString base = "";
    static const QStringList cppExts = {".h", "_p.h", ".hpp", ".hh"};
    static const QStringList cExts = {".h"};
    if (fname.endsWith(".cpp")) {
        exts = &cppExts;
        base = fname;
        base.chop(4);
    } else if (fname.endsWith(".c")) {
        exts = &cExts;
        base = fname;
        base.chop(2);
    }
    if (exts) {
        for (const QString &ext: *exts) {
            QString maybeHeader = base;
            maybeHeader.append(ext);

            if (QFile::exists(maybeHeader)) {
                out.append(maybeHeader);
            }
        }
    }
    return out;
}

}
