#pragma once

#include "mesonbuildfileparser.h"
#include "mesonbuildconfiguration.h"
#include "nodes.h"
#include "pathresolver.h"

#include <cpptools/cppprojectupdater.h>

#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>

#include <QStringList>
#include <QFutureInterface>
#include <QTimer>
#include <QFuture>
#include <QJsonArray>

#include <memory>


namespace MesonProjectManager {

class CompileCommandInfo
{
public:
    QMap<QString, QString> defines;
    QStringList includes;
    QString cpp_std;
    QString id;

    QStringList simplifiedCompilerParameters;
    QStringList files;

    bool operator==(const CompileCommandInfo &o) const;
};

enum class TargetType {
    Executable,
    StaticLibrary,
    DynamicLibrary,
    SharedModule,
};

struct SourceSetInfo {
    QString language;
    QStringList compiler;
    QStringList parameters;
    QStringList sources;
    QStringList generatedSources;
};

struct EditableList {
    QString name;
    std::shared_ptr<MesonBuildFileParser> parser;
};

struct TargetInfo {
    QString targetName;
    QString targetId;
    TargetType type;
    QString definedIn;
    QVector<SourceSetInfo> sourceSets;
    QVector<EditableList> editableLists;
    QSet<QString> sourcesInTarget;
    QSet<QString> extraFiles;
};

struct IntroSubProject
{
    QString name;
    QString version;
    QStringList buildsystemFiles;
    QVector<TargetInfo> targets;
    QString baseDir;
};

struct IntroProject: public IntroSubProject
{
    QString subprojectsDir;
    QVector<IntroSubProject> subprojects;
};

class MesonProject : public ProjectExplorer::Project
{
    Q_OBJECT

public:
    explicit MesonProject(const Utils::FileName &proFile);
    ~MesonProject() override;

    bool setupTarget(ProjectExplorer::Target *t) override;
    QStringList filesGeneratedFrom(const QString &sourceFile) const override;
    bool needsConfiguration() const override;
    void configureAsExampleProject(const QSet<Core::Id> &platforms) override;
    //bool requiresTargetPanel() const override;
    ProjectExplorer::ProjectImporter *projectImporter() const override;

    IntroProject mesonIntrospectProjectInfoFromSource(const MesonBuildConfiguration *cfg);
    const QVector<CompileCommandInfo> parseCompileCommands(const MesonBuildConfiguration &cfg, const QVector<TargetInfo> &targetInfos) const;
    QVector<CompileCommandInfo> rewritePaths(const PathResolver::DirectoryInfo &base, const QVector<CompileCommandInfo> &input) const;
    QVector<TargetInfo> readMesonInfoTargets(const MesonBuildConfiguration &cfg);

    void refreshCppCodeModel(const QVector<CompileCommandInfo> &);

    bool canConfigure();
    void editOptions();

    PathResolver pathResolver;

    static Utils::FileName findDefaultMesonExecutable();
    static void regenerateProjectFiles(MesonBuildFileParser *parser);

private:
    static QStringList jsonArrayToStringList(const QJsonArray arr);
    MesonBuildConfiguration *activeBuildConfiguration();
    const Utils::FileName filename;
    CppTools::CppProjectUpdater *cppCodeModelUpdater = nullptr;
    std::vector<std::unique_ptr<ProjectExplorer::ProjectDocument>> projectDocuments;

protected:
    RestoreResult fromMap(const QVariantMap &map, QString *errorMessage) override;

public slots:
    void refresh();
};

QStringList getAllHeadersFor(const QString &fname);

}
