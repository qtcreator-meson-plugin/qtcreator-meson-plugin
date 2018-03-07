#include "mesonfilesubfoldernode.h"

#include "filelistnode.h"

namespace MesonProjectManager {

MesonFileSubFolderNode::MesonFileSubFolderNode(const Utils::FileName &filename) :
    ProjectExplorer::FolderNode(filename)
{
}

bool MesonFileSubFolderNode::addFiles(const QStringList &filePaths, QStringList *notAdded)
{
    ProjectExplorer::FolderNode *ret = getFileListNode();
    if (!ret) return false;
    return ret->addFiles(filePaths, notAdded);
}

bool MesonFileSubFolderNode::removeFiles(const QStringList &filePaths, QStringList *notRemoved)
{
    ProjectExplorer::FolderNode *ret = getFileListNode();
    if (!ret) return false;
    return ret->removeFiles(filePaths, notRemoved);
}

bool MesonFileSubFolderNode::renameFile(const QString &filePath, const QString &newFilePath)
{
    ProjectExplorer::FolderNode *ret = getFileListNode();
    if (!ret) return false;
    return ret->renameFile(filePath, newFilePath);
}

bool MesonFileSubFolderNode::supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const
{
    ProjectExplorer::FolderNode *ret = getFileListNode();
    if (!ret) return false;
    return ret->supportsAction(action, node);
}

ProjectExplorer::FolderNode *MesonFileSubFolderNode::getFileListNode() const
{
    ProjectExplorer::FolderNode *ret = parentFolderNode();
    do {
        if (dynamic_cast<FileListNode*>(ret))
            return ret;
        if (ret->asProjectNode())
            return nullptr;

        ret = ret->parentFolderNode();
    } while (ret);
    return ret;
}

} // namespace MesonProjectManager
