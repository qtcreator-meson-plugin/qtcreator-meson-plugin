#pragma once

#include "mesonbuildfileparser.h"
#include "mesonprojectnode.h"
#include "mesonbuildconfiguration.h"
#include "pathresolver.h"

#include <cpptools/cppprojectupdater.h>

#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>

#include <QStringList>
#include <QFutureInterface>
#include <QTimer>
#include <QFuture>

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

    bool operator==(const CompileCommandInfo &o) const;
};

class MesonFileNode;

class MesonProject : public ProjectExplorer::Project
{
    Q_OBJECT

public:
    explicit MesonProject(const Utils::FileName &proFile);
    virtual ~MesonProject();

    bool setupTarget(ProjectExplorer::Target *t) override;
    QStringList filesGeneratedFrom(const QString &sourceFile) const override;
    bool needsConfiguration() const override;
    void configureAsExampleProject(const QSet<Core::Id> &platforms) override;
    //bool requiresTargetPanel() const override;
    ProjectExplorer::ProjectImporter *projectImporter() const override;

    void mesonIntrospectBuildsytemFiles(const MesonBuildConfiguration &cfg, MesonProjectNode *root);
    void mesonIntrospectProjectInfo(const MesonBuildConfiguration &cfg);
    bool mesonIntrospectProjectInfoFromSource(const MesonBuildConfiguration *cfg, MesonProjectNode *root);
    const QHash<CompileCommandInfo, QStringList> parseCompileCommands(const MesonBuildConfiguration &cfg) const;
    QHash<CompileCommandInfo, QStringList> rewritePaths(const PathResolver::DirectoryInfo &base, const QHash<CompileCommandInfo, QStringList> &input) const;

    void refreshCppCodeModel(const QHash<CompileCommandInfo, QStringList> &);

    bool canConfigure();
    void editOptions();

    QSet<QString> filesInEditableGroups;
    PathResolver pathResolver;

    MesonProjectManager::MesonFileNode* createMesonFileNode(ProjectExplorer::FolderNode *parentNode, QString parentRelativeName, Utils::FileName absoluteFileName);
    void createOtherBuildsystemFileNode(ProjectExplorer::FolderNode *parentNode, Utils::FileName absoluteFilename);
    ProjectExplorer::FolderNode *createSubProjectsNode(ProjectExplorer::FolderNode *parentNode);
    void addNestedNodes(ProjectExplorer::FolderNode *root, const QJsonObject &json, int displayNameSkip);

    static Utils::FileName findDefaultMesonExecutable();

private:
    MesonBuildConfiguration *activeBuildConfiguration();
    const Utils::FileName filename;
    CppTools::CppProjectUpdater *cppCodeModelUpdater = nullptr;

protected:
    RestoreResult fromMap(const QVariantMap &map, QString *errorMessage) override;

public slots:
    void refresh();
};

QStringList getAllHeadersFor(const QString &fname);

}
