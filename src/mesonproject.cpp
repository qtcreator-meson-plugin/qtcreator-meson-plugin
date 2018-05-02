#include "mesonproject.h"
#include "constants.h"
#include "filelistnode.h"
#include "mesonfilenode.h"
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
#include <cpptools/cppprojectupdater.h>

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

namespace MesonProjectManager {

quint64 qHash(const CompileCommandInfo &info)
{
    return qHash(info.includes)^qHash(info.cpp_std);
}

bool CompileCommandInfo::operator==(const CompileCommandInfo &o) const
{
    return std::tie(defines, includes, cpp_std, id) == std::tie(o.defines, o.includes, o.cpp_std, id);
}

MesonProject::MesonProject(const Utils::FileName &filename):
    Project(PROJECT_MIMETYPE, filename), filename(filename), cppCodeModelUpdater(new CppTools::CppProjectUpdater(this))
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

    // Stuff stolen from genericproject::refresh
    auto root = new MesonProjectNode(this, filename);

    if (cfg) {
        // TODO: fall back to just scan sub directories for meson files if !cfg?
        mesonIntrospectBuildsytemFiles(*cfg, root);
    }

    QHash<CompileCommandInfo, QStringList> codeModelInfo;
    if (cfg) {
        mesonIntrospectProjectInfo(*cfg);
        codeModelInfo = parseCompileCommands(*cfg);

        QSet<QString> allFiles;
        for (const QStringList &list: codeModelInfo.values()) {
            allFiles += QSet<QString>::fromList(list);
        }
        const QSet<QString> extraFiles = allFiles - filesInEditableGroups;

        const Utils::FileName projectBase = filename.parentDir();
        QString buildDirectory = cfg->buildDirectory().toString();
        if (!buildDirectory.endsWith('/')) buildDirectory += "/";
        auto extraFileNode = new ProjectExplorer::VirtualFolderNode(projectBase, 0);
        extraFileNode->setDisplayName("Extra Files");
        auto generatedFileNode = new ProjectExplorer::VirtualFolderNode(cfg->buildDirectory(), 0);
        generatedFileNode->setDisplayName("Generated Files");
        for(const auto &fname: extraFiles) {
            if (fname.isEmpty())
                continue;

            ProjectExplorer::VirtualFolderNode *node = extraFileNode;
            bool generated = false;
            if (fname.startsWith(buildDirectory)) {
                node = generatedFileNode;
                generated = true;
            }

            node->addNestedNode(new ProjectExplorer::FileNode(Utils::FileName::fromString(fname),
                                                                       ProjectExplorer::FileType::Source, generated));
            const QStringList headers = getAllHeadersFor(fname);
            for (const QString &header: headers) {
                node->addNestedNode(new ProjectExplorer::FileNode(Utils::FileName::fromString(header),
                                                                           ProjectExplorer::FileType::Header, generated));
            }
        }
        root->addNode(extraFileNode);
        root->addNode(generatedFileNode);
    }
    setRootProjectNode(root);

    refreshCppCodeModel(codeModelInfo);

    emitParsingFinished(true);
}

class SubProjectsNode : public ProjectExplorer::FolderNode
{
public:
    explicit SubProjectsNode(const Utils::FileName &folderPath, ProjectExplorer::NodeType nodeType = ProjectExplorer::NodeType::Folder,
                        const QString &displayName = QString(), const QByteArray &id = {}) :
        ProjectExplorer::FolderNode(folderPath, nodeType, displayName, id)
    {
        setPriority(-100000);
    }
};

void MesonProject::mesonIntrospectBuildsytemFiles(const MesonBuildConfiguration &cfg, MesonProjectNode *root)
{
    Utils::SynchronousProcess proc;
    proc.setTimeoutS(100);
    auto response = proc.run(cfg.mesonPath(), {"introspect", cfg.buildDirectory().toString(), "--buildsystem-files"});
    if (response.exitCode!=0) {
        ProjectExplorer::TaskHub::addTask(ProjectExplorer::Task::Error,
                                          QStringLiteral("Can't introspect buildsystem-files list. %1")
                                          .arg(response.exitMessage(cfg.mesonPath(), proc.timeoutS())),
                                          ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM,
                                          cfg.buildDirectory());

        Core::MessageManager::write(response.allOutput());
        return;
    }

    ProjectExplorer::FolderNode *subprojects = nullptr;

    QMap<QString, MesonFileNode*> subdirectories;

    QJsonArray json = QJsonDocument::fromJson(response.allRawOutput()).array();
    for (QJsonValue v: json) {
        QString file = v.toString();
        if (file == "meson.build") continue;

        ProjectExplorer::FolderNode *baseNode = root;
        Utils::FileName fn = Utils::FileName::fromString(filename.parentDir().appendPath(file).toString());
        QString dn = file;

        int longestMatch = 0;
        for (QString subdir : subdirectories.keys()) {
            if (file.startsWith(subdir)) {
                baseNode = subdirectories[subdir];
                longestMatch = subdir.size();
            }
        }
        if (longestMatch) {
            dn = dn.mid(longestMatch);
        }

        if (file.startsWith("subprojects/")) {
            if (!subprojects) {
                Utils::FileName fnSubprojects = Utils::FileName::fromString(filename.parentDir().appendPath("subprojects").toString());
                subprojects = new SubProjectsNode(fnSubprojects, ProjectExplorer::NodeType::Folder, "subprojects");
                subprojects->setIcon(Core::FileIconProvider::directoryIcon(":/projectexplorer/images/fileoverlay_qt.png"));
                root->addNode(subprojects);
            }
            if (baseNode == root) {
                baseNode = subprojects;
                dn = dn.mid(12);
            }
        }

        if (file.endsWith("/meson.build")) {
            dn = dn.mid(0, dn.size() - 12);
            auto mfn = new MesonFileNode(this, fn);
            mfn->setDisplayName(dn);
            baseNode->addNode(mfn);
            subdirectories[file.mid(0, file.size() - 11)] = mfn;
        } else {
            baseNode->addNestedNode(new ProjectExplorer::FileNode(fn, ProjectExplorer::FileType::Project, false),
            {}, [](const Utils::FileName &fn) {
                ProjectExplorer::FolderNode *node = new ProjectExplorer::FolderNode(fn);
                //node->setIcon(Core::FileIconProvider::directoryIcon(":/projectexplorer/images/fileoverlay_ui.png"));
                return node;
            });
        }
    }
}

void MesonProject::mesonIntrospectProjectInfo(const MesonBuildConfiguration &cfg)
{
    Utils::SynchronousProcess proc;
    proc.setTimeoutS(100);
    auto response = proc.run(cfg.mesonPath(), {"introspect", cfg.buildDirectory().toString(), "--projectinfo"});
    if (response.exitCode!=0) {
        ProjectExplorer::TaskHub::addTask(ProjectExplorer::Task::Error,
                                          QStringLiteral("Can't introspect projectinfo. %1")
                                          .arg(response.exitMessage(cfg.mesonPath(), proc.timeoutS())),
                                          ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM,
                                          cfg.buildDirectory());

        Core::MessageManager::write(response.allOutput());
        return;
    }

    QJsonObject json = QJsonDocument::fromJson(response.allRawOutput()).object();
    if (json.contains("name")) {
        setDisplayName(json.value("name").toString());
    }
}

const QHash<CompileCommandInfo, QStringList> MesonProject::parseCompileCommands(const MesonBuildConfiguration &cfg) const
{
    QHash<CompileCommandInfo, QStringList> fileCodeCompletionHints;

    Utils::FileName compileCommandsFileName = cfg.buildDirectory().appendPath("compile_commands.json");
    QFile compileCommandsFile(compileCommandsFileName.toString());
    if (!compileCommandsFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    QJsonArray json = QJsonDocument::fromJson(compileCommandsFile.readAll()).array();
    for (const QJsonValue& value: json) {
        QJsonObject obj = value.toObject();
        if (!obj.contains("directory") || !obj.contains("command") || !obj.contains("file"))
            continue;
        const QString dir = obj.value("directory").toString();
        const QString cmd = obj.value("command").toString();
        const QString file = obj.value("file").toString();
        const QString real_file = QFileInfo(dir+"/"+file).canonicalFilePath();

        QStringList parts;
        int wstart=0, ofs=0;
        bool in_quote=false;
        while (ofs<cmd.size()) {
            const QChar c=cmd.at(ofs);
            if (c==' ' && !in_quote) {
                if (ofs-wstart>0)
                    parts.append(cmd.mid(wstart, ofs-wstart));
                ofs++;
                wstart=ofs;
                continue;
            } else if (c=='\'') {
                if (in_quote) {
                    in_quote=false;
                    parts.append(cmd.mid(wstart+1, ofs-wstart-1));
                    ofs++;
                    wstart=ofs;
                    continue;
                } else {
                    in_quote=true;
                }
            }
            ofs++;
        }

        CompileCommandInfo info;
        info.includes.append("/usr/include/");

        bool nextIsOutput = false;

        for (const QString &part: parts) {
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
                info.defines.insert(value.section("=", 0, 1), value.section("=", 1, -1));
            } else if (part.startsWith("-U")) {
                info.defines.remove(part.mid(2));
            } else if (part.startsWith("-I")) {
                QString idir = part.mid(2);
                if (!idir.startsWith("/")) {
                    idir = dir + "/" + idir;
                }
                if (!idir.endsWith("/"))
                    idir.append("/");
                info.includes.append(idir);
            } else if (part.startsWith("-std")) {
                info.cpp_std = part.mid(5);
            } else if (part == "-o") {
                nextIsOutput = true;
            }
        }

        fileCodeCompletionHints[info].append(real_file);
    }

    return fileCodeCompletionHints;
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

bool MesonProject::requiresTargetPanel() const
{
    return false;
}

void MesonProject::refreshCppCodeModel(const QHash<CompileCommandInfo, QStringList> &fileCodeCompletionHints)
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
    for (const CompileCommandInfo &info: fileCodeCompletionHints.keys()) {
        const QStringList &files = fileCodeCompletionHints.value(info);

        CppTools::RawProjectPart rpp;
        rpp.setDisplayName(info.id + QStringLiteral(" Part %1").arg(QString::number(i)));
        rpp.setProjectFileLocation(projectFilePath().toString());
        rpp.setQtVersion(activeQtVersion);
        rpp.setIncludePaths(info.includes);
        QVector<ProjectExplorer::Macro> macros;
        for (const auto &key: info.defines.keys())
            macros.append(ProjectExplorer::Macro(key.toUtf8(), info.defines[key].toUtf8()));
        rpp.setMacros(macros);
        rpp.setFiles(files);

        parts.append(rpp);
        i++;
    }

    const CppTools::ProjectUpdateInfo projectInfoUpdate(this, cToolChain, cxxToolChain, k, parts);
    cppCodeModelUpdater->update(projectInfoUpdate);
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
    const QStringList *exts;
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
    if (!base.isEmpty()) {
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
