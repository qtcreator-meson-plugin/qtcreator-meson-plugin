#include "filelistnode.h"

#include "mesonproject.h"

namespace MesonProjectManager {

FileListNode::FileListNode(std::shared_ptr<MesonBuildFileParser> parser, MesonBuildFileParser::ChunkInfo *chunk, const Utils::FileName &folderPath, int priority, MesonProject *project) :
    ProjectExplorer::VirtualFolderNode(folderPath, priority), parser(parser), chunk(chunk), project(project)
{
}

bool FileListNode::addFiles(const QStringList &filePaths, QStringList *notAdded)
{
    QStringList implicitFiles;
    for (const QString &str: chunk->file_list) {
        implicitFiles += getAllHeadersFor(parser->getProject_base() + "/" + str);
    }
    for (const QString &str: filePaths) {
        implicitFiles += getAllHeadersFor(str);
    }
    for(const auto &fp: filePaths) {
        QString relative_fn = getRelativeFileName(fp);
        if(!chunk->file_list.contains(relative_fn) && !implicitFiles.contains(fp))
            chunk->file_list.append(relative_fn);
    }

    MesonProject::regenerateProjectFiles(parser.get());
    project->refresh();
    return true;
}

bool FileListNode::removeFiles(const QStringList &filePaths, QStringList *notRemoved)
{
    for(const auto &fp: filePaths) {
        QString relative_fn = getRelativeFileName(fp);
        chunk->file_list.removeOne(relative_fn);
    }

    MesonProject::regenerateProjectFiles(parser.get());
    project->refresh();
    return true;
}

bool FileListNode::renameFile(const QString &filePath, const QString &newFilePath)
{
    // TODO: this needs to be global across the whole project (all file lists of all meson files)
    if(chunk->file_list.removeOne(getRelativeFileName(filePath)))
        chunk->file_list.append(getRelativeFileName(newFilePath));

    MesonProject::regenerateProjectFiles(parser.get());
    project->refresh();
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
    QString relative_fn = fn.relativeChildPath(Utils::FileName::fromString(parser->getProject_base())).toString();

    return relative_fn;
}

} // namespace MesonProjectManager
