#include "mesonproject.h"
#include "nodes.h"

#include <QVector>

#pragma once

namespace MesonProjectManager
{

class TreeBuilder
{
public:
    TreeBuilder(MesonProject *mesonProject, IntroProject &project);
    IntroProject &_project;
    void build(MesonRootProjectNode *root, const QString &buildDir);

    static void addEditableFileLists(ProjectExplorer::FolderNode *node, MesonProject *project, const QVector<EditableList> &editableLists);
    static void processEditableFileList(ProjectExplorer::FolderNode *listNode, const EditableList &list);

private:
    void buildProject(ProjectExplorer::FolderNode *parent, IntroSubProject& project, const QString &buildDir);
    void addNestedNodes(ProjectExplorer::FolderNode *root, const QVector<EditableList> &editableLists, IntroSubProject &project, const QString &buildDir);
    void setupMesonFileNode(ProjectExplorer::FolderNode *node, Utils::FileName absoluteFileName, const QVector<EditableList> &editableLists);
    MesonSubDirNode *createMesonSubDirNode(ProjectExplorer::FolderNode *parentNode, QString parentRelativeName, Utils::FileName absoluteFileName);
    void createOtherBuildsystemFileNode(ProjectExplorer::FolderNode *parentNode, Utils::FileName absoluteFilename);
    ProjectExplorer::FolderNode *createSubProjectsNode(ProjectExplorer::FolderNode *parentNode);

    MesonProject *mesonProject;
};

}
