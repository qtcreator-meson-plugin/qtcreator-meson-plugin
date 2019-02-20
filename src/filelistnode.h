#pragma once

#include "mesonbuildfileparser.h"

#include <projectexplorer/projectnodes.h>

namespace MesonProjectManager {

class MesonProject;

class FileListNode : public virtual ProjectExplorer::VirtualFolderNode {
public:
    explicit FileListNode(std::shared_ptr<MesonBuildFileParser> parser, MesonBuildFileParser::ChunkInfo *chunk, const Utils::FileName &folderPath, int priority, MesonProjectManager::MesonProject *project);

    bool addFiles(const QStringList &filePaths, QStringList *notAdded) override;
    bool removeFiles(const QStringList &filePaths, QStringList *notRemoved) override;
    bool renameFile(const QString &filePath, const QString &newFilePath) override;
    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;

    //bool canRenameFile(const QString &filePath, const QString &newFilePath) override;

private:
    QString getRelativeFileName(const QString &filePath);

    std::shared_ptr<MesonBuildFileParser> parser;
    MesonBuildFileParser::ChunkInfo *chunk = nullptr;
    MesonProject *project = nullptr;
};

} // namespace MesonProjectManager
