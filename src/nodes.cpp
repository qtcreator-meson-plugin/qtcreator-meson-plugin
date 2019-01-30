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

bool MesonTargetNode::supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const
{
    Q_UNUSED(node);
    Q_UNUSED(action);
    return false;
}

MesonTargetNode::MesonTargetNode(MesonProject *project, const Utils::FileName &filename, const QVector<EditableList> &editableLists, const TargetInfo &target, const QString &buildDir) :
    ProjectExplorer::FolderNode(filename.parentDir(), ProjectExplorer::NodeType::VirtualFolder)
{
    static QMap<TargetType, QString> targetTypeNames{
        {TargetType::Executable, "Executable"},
        {TargetType::SharedModule, "Shared Module"},
        {TargetType::StaticLibrary, "Static Library"},
        {TargetType::DynamicLibrary, "Dynamic Library"},
    };

    TreeBuilder::addEditableFileLists(this, project, editableLists);
    setDisplayName(target.targetName + " ("+targetTypeNames.value(target.type)+")");
    setPriority(Priorities::Target);

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


    addGeneratedAndExtraFileNodes(target, buildDir);
}

void MesonTargetNode::addGeneratedAndExtraFileNodes(const TargetInfo &target, const QString &buildDir)
{
    QStringList generatedExtraFiles;

    auto extraFileNode = std::make_unique<ProjectExplorer::VirtualFolderNode>(Utils::FileName::fromString(target.definedIn.chopped(12)), Priorities::ExtraFiles);
    extraFileNode->setDisplayName("Extra Files");

    for(const auto &fname: target.extraFiles) {
        if (fname.isEmpty())
            continue;

        if (fname.startsWith(buildDir)) {
            generatedExtraFiles.append(fname);
            continue;
        }

        extraFileNode->addNestedNode(std::make_unique<ProjectExplorer::FileNode>(Utils::FileName::fromString(fname),
                                                                        ProjectExplorer::FileType::Source, false));
        const QStringList headers = getAllHeadersFor(fname);
        for (const QString &header: headers) {
            extraFileNode->addNestedNode(std::make_unique<ProjectExplorer::FileNode>(Utils::FileName::fromString(header),
                                                                            ProjectExplorer::FileType::Header, false));
        }
    }
    addNode(move(extraFileNode));

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

    const Utils::FileName baseDir = Utils::FileName::fromString(longestPrefix.join("/"));
    auto generatedFileNode = std::make_unique<ProjectExplorer::VirtualFolderNode>(baseDir, Priorities::GeneratedFiles);
    generatedFileNode->setDisplayName("Generated Files");

    auto addGeneratedFile = [&generatedFileNode](const QString &file) {
        generatedFileNode->addNestedNode(std::make_unique<ProjectExplorer::FileNode>(Utils::FileName::fromString(file),
                                                                                     ProjectExplorer::FileType::Source,
                                                                                     true));
        const QStringList headers = getAllHeadersFor(file);
        for (const QString &header: headers) {
            generatedFileNode->addNestedNode(std::make_unique<ProjectExplorer::FileNode>(Utils::FileName::fromString(header),
                                                                                         ProjectExplorer::FileType::Header, true));
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

MesonSubDirNode::MesonSubDirNode(const Utils::FileName &filename) :
    ProjectExplorer::FolderNode(filename.parentDir(), ProjectExplorer::NodeType::Folder)
{
    setPriority(Priorities::SubDir);
}

bool MesonSubDirNode::supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const
{
    Q_UNUSED(node);
    Q_UNUSED(action);
    return false;
}

MesonRootProjectNode::MesonRootProjectNode(MesonProject *project) :
    ProjectExplorer::ProjectNode(project->projectDirectory())
{
}

bool MesonRootProjectNode::supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const
{
    Q_UNUSED(node);
    Q_UNUSED(action);
    return false;
}


SubProjectsNode::SubProjectsNode(const Utils::FileName &folderPath, ProjectExplorer::NodeType nodeType, const QString &displayName, const QByteArray &id) :
    ProjectExplorer::FolderNode(folderPath, nodeType, displayName, id)
{
    setPriority(Priorities::Subproject);
}

} // namespace MesonProjectManager
