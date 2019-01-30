#pragma once

#include "mesonproject.h"

#include <projectexplorer/projectnodes.h>

namespace MesonProjectManager {

class MesonProject;
struct EditableList;
struct TargetInfo;

class MesonTargetNode: public ProjectExplorer::FolderNode
{
public:
    MesonTargetNode(MesonProject *project, const Utils::FileName &filename, const QVector<EditableList> &editableLists, const TargetInfo &target, const QString &buildDir);
    void addGeneratedAndExtraFileNodes(const TargetInfo &target, const QString &buildDir);

    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;
};

class MesonSubDirNode: public ProjectExplorer::FolderNode
{
public:
    MesonSubDirNode(const Utils::FileName &filename);

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
    explicit SubProjectsNode(const Utils::FileName &folderPath, ProjectExplorer::NodeType nodeType = ProjectExplorer::NodeType::Folder,
                        const QString &displayName = QString(), const QByteArray &id = {});
};



} // namespace MesonProjectManager
