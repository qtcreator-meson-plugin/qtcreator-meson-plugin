#include "filelistnode.h"

#include "mesonproject.h"

namespace MesonProjectManager {

FileListNode::FileListNode(MesonProjectPartManager *projectPartManager, MesonBuildFileParser::ChunkInfo *chunk, const Utils::FileName &folderPath, int priority) :
    ProjectExplorer::VirtualFolderNode(folderPath, priority), projectPartManager(projectPartManager), chunk(chunk)
{
}

bool FileListNode::addFiles(const QStringList &filePaths, QStringList *notAdded)
{
    for(const auto &fp: filePaths) {
        QString relative_fn = getRelativeFileName(fp);
        if(!chunk->file_list.contains(relative_fn))
            chunk->file_list.append(relative_fn);
    }
    //    addNestedNode(new ProjectExplorer::FileNode(Utils::FileName::fromString(fp), ProjectExplorer::FileType::Source, false));

    projectPartManager->regenerateProjectFiles();
    projectPartManager->project->refresh();
    return true;
}

bool FileListNode::removeFiles(const QStringList &filePaths, QStringList *notRemoved)
{
    for(const auto &fp: filePaths) {
        QString relative_fn = getRelativeFileName(fp);
        chunk->file_list.removeOne(relative_fn);
    }

    projectPartManager->regenerateProjectFiles();
    projectPartManager->project->refresh();
    return true;
}

bool FileListNode::renameFile(const QString &filePath, const QString &newFilePath)
{
    // TODO this needs to be global across the whole project (all file lists of all meson files)
    if(chunk->file_list.removeOne(getRelativeFileName(filePath)))
        chunk->file_list.append(getRelativeFileName(newFilePath));

    projectPartManager->regenerateProjectFiles();
    projectPartManager->project->refresh();
    return true;
}

bool FileListNode::supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const
{
    Q_UNUSED(node);
    return action == ProjectExplorer::AddNewFile
            || action == ProjectExplorer::AddExistingFile
            || action == ProjectExplorer::AddExistingDirectory
            || action == ProjectExplorer::RemoveFile
            || action == ProjectExplorer::Rename;
}

QString FileListNode::getRelativeFileName(const QString &filePath)
{
    auto fn = Utils::FileName::fromString(filePath);
    QString relative_fn = fn.relativeChildPath(Utils::FileName::fromString(projectPartManager->parser->getProject_base())).toString();

    return relative_fn;
}

} // namespace MesonProjectManager
