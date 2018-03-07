#pragma once

#include <memory>

#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>
//#include <projectexplorer/runconfiguration.h>
#include "mesonprojectparser.h"

#include <QStringList>
#include <QFutureInterface>
#include <QTimer>
#include <QFuture>

#include <cpptools/cppprojectupdater.h>

namespace xxxMeson {

class MesonProject;

class MesonProjectPartManager
{
public:
    MesonProjectPartManager(ProjectExplorer::FolderNode *node, MesonProject *project, const Utils::FileName &filename);

public:
    void regenerateProjectFiles();

    MesonProject *project = nullptr;
    std::unique_ptr<ProjectExplorer::ProjectDocument> meson_build;
    std::unique_ptr<MesonBuildParser> parser;
};


class MesonProjectNode: public ProjectExplorer::ProjectNode
{
public:
    MesonProjectNode(MesonProject *project, const Utils::FileName &filename);

    // Node interface
public:
    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;

    MesonProjectPartManager partMgr;
};

class MesonFileNode: public ProjectExplorer::FolderNode
{
public:
    MesonFileNode(MesonProject *project, const Utils::FileName &filename);

public:
    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;

    MesonProjectPartManager partMgr;
};

class MesonFileSubFolderNode: public ProjectExplorer::FolderNode
{
public:
    MesonFileSubFolderNode(const Utils::FileName &filename);

public:
    bool addFiles(const QStringList &filePaths, QStringList *notAdded) override;
    bool removeFiles(const QStringList &filePaths, QStringList *notRemoved) override;
    bool renameFile(const QString &filePath, const QString &newFilePath) override;

    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;

private:
    ProjectExplorer::FolderNode *getFileListNode() const;
};

struct CompileCommandInfo
{
    QMap<QString, QString> defines;
    QStringList includes;
    QString cpp_std;

    bool operator==(const CompileCommandInfo &o) const
    {
        return std::tie(defines, includes, cpp_std) == std::tie(o.defines, o.includes, o.cpp_std);
    }
};

class MesonProject : public ProjectExplorer::Project
{
    Q_OBJECT

public:
    explicit MesonProject(const Utils::FileName &proFile);
    virtual ~MesonProject();

    // Project interface
public:
    bool supportsKit(ProjectExplorer::Kit *k, QString *errorMessage) const override;
    bool setupTarget(ProjectExplorer::Target *t);
    QStringList filesGeneratedFrom(const QString &sourceFile) const override;
    bool needsConfiguration() const override;
    void configureAsExampleProject(const QSet<Core::Id> &platforms) override;
    bool requiresTargetPanel() const override;
    ProjectExplorer::ProjectImporter *projectImporter() const override;

    void mesonIntrospectBuildsytemFiles(MesonProjectNode *root);

    void mesonIntrospectProjectInfo();
    const QHash<CompileCommandInfo, QStringList> parseCompileCommands() const;

    const Utils::FileName filename;

    void refreshCppCodeModel(const QHash<CompileCommandInfo, QStringList> &);
    CppTools::CppProjectUpdater *m_cppCodeModelUpdater = nullptr;

protected:
    RestoreResult fromMap(const QVariantMap &map, QString *errorMessage) override;

public slots:
    void refresh();
};

}
