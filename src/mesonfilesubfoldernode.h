#pragma once

#include <projectexplorer/projectnodes.h>

namespace MesonProjectManager {

class MesonFileSubFolderNode: public ProjectExplorer::FolderNode
{
public:
    MesonFileSubFolderNode(const Utils::FileName &filename);

    bool addFiles(const QStringList &filePaths, QStringList *notAdded) override;
    bool removeFiles(const QStringList &filePaths, QStringList *notRemoved) override;
    bool renameFile(const QString &filePath, const QString &newFilePath) override;
    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;

private:
    ProjectExplorer::FolderNode *getFileListNode() const;
};

} // namespace MesonProjectManager
