#pragma once

#include "filelistnode.h"

#include <projectexplorer/projectnodes.h>

namespace MesonProjectManager {

class MesonProject;
struct EditableList;
struct TargetInfo;

class MesonTargetNode: public virtual ProjectExplorer::VirtualFolderNode
{
public:
    MesonTargetNode(MesonProject *project, const Utils::FilePath &filename, const QVector<EditableList> &editableLists, const TargetInfo &target, const QString &buildDir);
    void addGeneratedAndUngroupedFilesNodes(const TargetInfo &target, const QString &buildDir);

    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;
};

class MesonSingleGroupTargetNode: public FileListNode, public MesonTargetNode
{
public:
    MesonSingleGroupTargetNode(const Utils::FilePath &folderPath, MesonProjectManager::MesonProject *project, const EditableList &editableList, const TargetInfo &target, const QString &buildDir);
    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;
};

class MesonSubDirNode: public ProjectExplorer::FolderNode
{
public:
    MesonSubDirNode(const Utils::FilePath &filename);

    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;
};

class MesonRootProjectNode: public ProjectExplorer::ProjectNode
{
public:
    MesonRootProjectNode(MesonProject *project);
    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;
};

class SubProjectsNode : public ProjectExplorer::FolderNode
{
public:
    explicit SubProjectsNode(const Utils::FilePath &folderPath, const QString &displayName = QString());
};



} // namespace MesonProjectManager
