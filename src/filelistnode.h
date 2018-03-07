#pragma once

#include "mesonbuildfileparser.h"

#include <projectexplorer/projectnodes.h>

namespace MesonProjectManager {

class MesonProjectPartManager;

class FileListNode : public ProjectExplorer::VirtualFolderNode {
public:
    explicit FileListNode(MesonProjectPartManager* projectPartManager, MesonBuildFileParser::ChunkInfo *chunk, const Utils::FileName &folderPath, int priority);

    bool addFiles(const QStringList &filePaths, QStringList *notAdded) override;
    bool removeFiles(const QStringList &filePaths, QStringList *notRemoved) override;
    bool renameFile(const QString &filePath, const QString &newFilePath) override;
    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;

    //bool canRenameFile(const QString &filePath, const QString &newFilePath) override;

private:
    QString getRelativeFileName(const QString &filePath);

    MesonProjectPartManager* projectPartManager = nullptr;
    MesonBuildFileParser::ChunkInfo *chunk = nullptr;
};

} // namespace MesonProjectManager
