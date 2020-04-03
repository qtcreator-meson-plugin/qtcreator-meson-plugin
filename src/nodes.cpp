#include "nodes.h"
#include "constants.h"
#include "filelistnode.h"
#include "mesonfilesubfoldernode.h"
#include "treebuilder.h"

#include <coreplugin/documentmanager.h>
#include <coreplugin/fileiconprovider.h>
#include <coreplugin/icore.h>

#include <projectexplorer/projectnodes.h>

namespace MesonProjectManager {


MesonTargetNode::MesonTargetNode(MesonProject *project, const Utils::FilePath &filename, const QVector<EditableList> &editableLists, const TargetInfo &target, const QString &buildDir) :
    ProjectExplorer::VirtualFolderNode(filename.parentDir())
{
    setPriority(Priorities::Target);
    static QMap<TargetType, QString> targetTypeNames{
        {TargetType::Executable, "Executable"},
        {TargetType::SharedModule, "Shared Module"},
        {TargetType::StaticLibrary, "Static Library"},
        {TargetType::DynamicLibrary, "Dynamic Library"},
    };

    setDisplayName(target.targetName + " ("+targetTypeNames.value(target.type)+")");
    addGeneratedAndUngroupedFilesNodes(target, buildDir);

    QString iconResouce;
    switch(target.type) {
    case TargetType::Executable:
        iconResouce = "run.png";
        break;
    case TargetType::SharedModule:
    case TargetType::StaticLibrary:
    case TargetType::DynamicLibrary:
        iconResouce = "build.png";
        break;
    }
    setIcon(Core::FileIconProvider::directoryIcon(":/projectexplorer/images/"+iconResouce));

    TreeBuilder::addEditableFileLists(this, project, editableLists);
}

void MesonTargetNode::addGeneratedAndUngroupedFilesNodes(const TargetInfo &target, const QString &buildDir)
{
    QStringList generatedExtraFiles;

    auto ungroupedFilesNode = std::make_unique<ProjectExplorer::VirtualFolderNode>(Utils::FilePath::fromString(target.definedIn.chopped(12)));
    ungroupedFilesNode->setPriority(Priorities::ExtraFiles);
    if(target.editableLists.size()>0) {
        ungroupedFilesNode->setDisplayName("Other Sources");
    } else {
        ungroupedFilesNode->setDisplayName("Sources");
    }

    for(const auto &fname: target.extraFiles) {
        if (fname.isEmpty())
            continue;

        if (fname.startsWith(buildDir)) {
            generatedExtraFiles.append(fname);
            continue;
        }

        ungroupedFilesNode->addNestedNode(std::make_unique<ProjectExplorer::FileNode>(Utils::FilePath::fromString(fname),
                                                                        ProjectExplorer::FileType::Source));
        ungroupedFilesNode->setIsGenerated(false);
        const QStringList headers = getAllHeadersFor(fname);
        for (const QString &header: headers) {
            ungroupedFilesNode->addNestedNode(std::make_unique<ProjectExplorer::FileNode>(Utils::FilePath::fromString(header),
                                                                            ProjectExplorer::FileType::Header));
            ungroupedFilesNode->setIsGenerated(false);
        }
    }
    addNode(move(ungroupedFilesNode));

    QStringList longestPrefix;

    auto updateLongestPrefix = [&longestPrefix](const QString &file) {
        const QStringList absolutePath = QFileInfo(file).absolutePath().split("/");
        if(longestPrefix.isEmpty()) {
            longestPrefix = absolutePath;
        } else {
            for(int i=0; i<longestPrefix.size(); ++i) {
                if(i>=absolutePath.size() || longestPrefix.at(i)!=absolutePath.at(i)) {
                    longestPrefix.erase(longestPrefix.begin()+i, longestPrefix.end());
                    break;
                }
            }
        }
    };

    for(const SourceSetInfo &set: target.sourceSets) {
        for(const QString &file: set.generatedSources) {
            updateLongestPrefix(file);
        }
    }
    for(const QString &extraFile: generatedExtraFiles) {
        updateLongestPrefix(extraFile);
    }

    const Utils::FilePath baseDir = Utils::FilePath::fromString(longestPrefix.join("/"));
    auto generatedFileNode = std::make_unique<ProjectExplorer::VirtualFolderNode>(baseDir);
    generatedFileNode->setPriority(Priorities::GeneratedFiles);
    generatedFileNode->setDisplayName("Generated Files");

    auto addGeneratedFile = [&generatedFileNode](const QString &file) {
        auto node = std::make_unique<ProjectExplorer::FileNode>(Utils::FilePath::fromString(file),
                                                                ProjectExplorer::FileType::Source);
        node->setIsGenerated(true);
        generatedFileNode->addNestedNode(std::move(node));
        const QStringList headers = getAllHeadersFor(file);
        for (const QString &header: headers) {
            auto node = std::make_unique<ProjectExplorer::FileNode>(Utils::FilePath::fromString(header),
                                                                    ProjectExplorer::FileType::Header);
            node->setIsGenerated(true);
            generatedFileNode->addNestedNode(std::move(node));
        }
    };

    for(const SourceSetInfo &set: target.sourceSets) {
        for(const QString &file: set.generatedSources) {
            addGeneratedFile(file);
        }
    }
    for(const QString &extraFile: generatedExtraFiles) {
        addGeneratedFile(extraFile);
    }

    if(!generatedFileNode->isEmpty())
        addNode(move(generatedFileNode));
}

bool MesonTargetNode::supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const
{
    Q_UNUSED(node)
    Q_UNUSED(action)
    return false;
}

MesonSubDirNode::MesonSubDirNode(const Utils::FilePath &filename) :
    ProjectExplorer::FolderNode(filename.parentDir())
{
    setPriority(Priorities::SubDir);
}

bool MesonSubDirNode::supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const
{
    Q_UNUSED(node)
    Q_UNUSED(action)
    return false;
}

MesonRootProjectNode::MesonRootProjectNode(MesonProject *project) :
    ProjectExplorer::ProjectNode(project->projectDirectory())
{
}

bool MesonRootProjectNode::supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const
{
    Q_UNUSED(node)
    Q_UNUSED(action)
    return false;
}


SubProjectsNode::SubProjectsNode(const Utils::FilePath &folderPath, const QString &displayName) :
    ProjectExplorer::FolderNode(folderPath)
{
    setPriority(Priorities::Subproject);
    setDisplayName(displayName);
}

MesonSingleGroupTargetNode::MesonSingleGroupTargetNode(const Utils::FilePath &folderPath, MesonProject *project, const EditableList &editableList, const TargetInfo &target, const QString &buildDir)
    : ProjectExplorer::VirtualFolderNode(folderPath),
      FileListNode(editableList.parser, &editableList.parser->fileList(editableList.name), folderPath, Priorities::Target, project),
      MesonTargetNode(project, Utils::FilePath(), {}, target, buildDir)
{
    TreeBuilder::processEditableFileList(this, editableList);
    setPriority(Priorities::Target);
}

bool MesonSingleGroupTargetNode::supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const
{
    return FileListNode::supportsAction(action, node);
}

} // namespace MesonProjectManager
