#include "nodes.h"
#include "treebuilder.h"
#include <coreplugin/fileiconprovider.h>
#include "constants.h"
#include "filelistnode.h"
#include "mesonfilesubfoldernode.h"

#include <iostream>

namespace MesonProjectManager
{

TreeBuilder::TreeBuilder(MesonProject *mesonProject, IntroProject &project): _project(project), mesonProject(mesonProject)
{
}

void TreeBuilder::build(MesonRootProjectNode *root, const QString &buildDir)
{
    buildProject(root, _project, buildDir);
    if(!_project.subprojects.isEmpty()) {
        ProjectExplorer::FolderNode *subprojectsNode = createSubProjectsNode(root);
        for(IntroSubProject &sub: _project.subprojects) {
            auto subprojectNode = std::make_unique<ProjectExplorer::VirtualFolderNode>(Utils::FilePath::fromString(sub.baseDir));
            subprojectNode->setPriority(0);
            subprojectNode->setDisplayName(sub.name);
            buildProject(subprojectNode.get(), sub, buildDir);
            subprojectsNode->addNode(move(subprojectNode));
        }
    }
}

void TreeBuilder::addEditableFileLists(ProjectExplorer::FolderNode *node, MesonProject *project, const QVector<EditableList> &editableLists)
{
    for(const auto &list: editableLists)
    {
        auto listNode = std::make_unique<FileListNode>(list.parser, &list.parser->fileList(list.name), Utils::FilePath::fromString(list.parser->getProject_base()), Priorities::EditableGroup, project);
        listNode->setIcon(Core::FileIconProvider::directoryIcon(":/projectexplorer/images/fileoverlay_ui.png"));
        listNode->setDisplayName(list.name);
        processEditableFileList(listNode.get(), list);
        node->addNode(move(listNode));
    }
}

void TreeBuilder::processEditableFileList(ProjectExplorer::FolderNode *listNode, const EditableList &list)
{
    const QStringList abs_file_paths = list.parser->fileListAbsolute(list.name);
    for(const auto &fname: abs_file_paths) {
        listNode->addNestedNode(std::make_unique<ProjectExplorer::FileNode>(Utils::FilePath::fromString(fname), ProjectExplorer::FileType::Source),
        {}, [](const Utils::FilePath &fn) {
            auto node = std::make_unique<MesonFileSubFolderNode>(fn);
            node->setIcon(Core::FileIconProvider::directoryIcon(":/projectexplorer/images/fileoverlay_ui.png"));
            return node;
        });

        const QStringList headers = getAllHeadersFor(fname);
        for(const QString &header: headers) {
            if(abs_file_paths.contains(header))
                continue;
            listNode->addNestedNode(std::make_unique<ProjectExplorer::FileNode>(Utils::FilePath::fromString(header), ProjectExplorer::FileType::Header),
            {}, [](const Utils::FilePath &fn) {
                auto node = std::make_unique<MesonFileSubFolderNode>(fn);
                node->setIcon(Core::FileIconProvider::directoryIcon(":/projectexplorer/images/fileoverlay_ui.png"));
                return node;
            });
        }
    }

}

void TreeBuilder::buildProject(ProjectExplorer::FolderNode *parent, IntroSubProject &project, const QString &buildDir)
{
    QVector<EditableList> unprocessedLists;

    for (const QString &file: project.buildsystemFiles) {
        if (!file.endsWith("/meson.build") && file != "meson.build")
            continue;

        std::shared_ptr<MesonBuildFileParser> parser = std::make_shared<MesonBuildFileParser>(file);
        parser->parse();
        for (const QString &group: parser->fileListNames()) {
            unprocessedLists.append({group, parser});
        }
    }

    for (TargetInfo &target: project.targets) {
        QSet<QString> sourcesInTarget;
        for (const SourceSetInfo &set: target.sourceSets) {
             sourcesInTarget += set.sources.toSet();
        }
        target.sourcesInTarget = sourcesInTarget;
        target.extraFiles = sourcesInTarget;
    }

    QVector<EditableList> unassignedLists;
    for (const EditableList &list: unprocessedLists) {
        QSet<QString> files;
        for(const QString &entry: list.parser->fileList(list.name).file_list) {
            files.insert(list.parser->getProject_base()+"/"+entry);
        }
        TargetInfo *candidate = nullptr;
        bool inMultipleTargets = false;
        for (TargetInfo &target: project.targets) {
            const QSet<QString> sourcesInTarget = target.sourcesInTarget;
            if ((files-sourcesInTarget).isEmpty()) {
                // files is a subset of sourcesSet
                if (candidate) {
                    inMultipleTargets = true;
                } else {
                    candidate = &target;
                }
            }
        }
        if (candidate && !inMultipleTargets) {
            candidate->extraFiles -= files;
            candidate->editableLists.append(list);
        } else {
            unassignedLists.append(list);
        }
    }
    addNestedNodes(parent, unassignedLists, project, buildDir);
}

void TreeBuilder::addNestedNodes(ProjectExplorer::FolderNode *root, const QVector<EditableList> &editableLists, IntroSubProject &project, const QString &buildDir)
{
    QStringList buildsystemFiles = project.buildsystemFiles;
    std::sort(buildsystemFiles.begin(), buildsystemFiles.end(), [](const QString &a, const QString &b) {
        return a.size()<b.size();
    });

    QMap<QString, ProjectExplorer::FolderNode*> createdNodes;
    for(const QString &absoluteFilename: buildsystemFiles) {
        QString relativeFilename = QDir(project.baseDir).relativeFilePath(absoluteFilename);
        ProjectExplorer::FolderNode *parentNode = root;
        QString parentRelativeFilename = relativeFilename;

        // TODO: Can we use ProjectExplorer::FolderNode::addNestedNode instead?

        QString probePath = relativeFilename.section("/", 0, -2);
        while (probePath.length()) {
            if(createdNodes.contains(probePath)) {
                parentRelativeFilename = relativeFilename.mid(probePath.length()+1);
                parentNode = createdNodes.value(probePath);
            }
            probePath = probePath.section("/", 0, -2);
        }
        if(relativeFilename.endsWith("/meson.build") || relativeFilename=="meson.build") {
            QVector<EditableList> localEditableLists;
            for (const EditableList &list: editableLists) {
                if(list.parser->filename == absoluteFilename) {
                    localEditableLists.append(list);
                }
            }

            if (relativeFilename.endsWith("/meson.build")) { // This is a subdir
                MesonSubDirNode *msn = createMesonSubDirNode(parentNode, parentRelativeFilename, Utils::FilePath::fromString(absoluteFilename));
                createdNodes[relativeFilename.mid(0, relativeFilename.size() - 12)] = msn; // Remove /meson.build suffix
                parentNode = msn;
            }
            setupMesonFileNode(parentNode, Utils::FilePath::fromString(absoluteFilename), localEditableLists);
        } else {
            createOtherBuildsystemFileNode(parentNode, Utils::FilePath::fromString(absoluteFilename));
        }

        for (const TargetInfo &target: project.targets) {
            if (target.definedIn != absoluteFilename)
                continue;

            bool anyUngroupedFiles = false;
            for(const QString &extraFile: target.extraFiles) {
                if(extraFile.startsWith(buildDir)) {
                    anyUngroupedFiles = true;
                    break;
                }
            }

            if(target.editableLists.size()==1 && !anyUngroupedFiles) {
                parentNode->addNode(std::make_unique<MesonSingleGroupTargetNode>(Utils::FilePath::fromString(absoluteFilename).parentDir(), mesonProject, target.editableLists.first(), target, buildDir));
            } else  {
                parentNode->addNode(std::make_unique<MesonTargetNode>(mesonProject, Utils::FilePath::fromString(absoluteFilename), target.editableLists, target, buildDir));
            }
        }
    }
}

void TreeBuilder::setupMesonFileNode(ProjectExplorer::FolderNode *node, Utils::FilePath absoluteFileName, const QVector<EditableList> &editableLists)
{
    addEditableFileLists(node, mesonProject, editableLists);

    // Populate path resolver lookup table for original directory names (to deal with structures reachable via dirs and symlinks at the same time)
    mesonProject->pathResolver.getForPath(QFileInfo(absoluteFileName.toString()).absolutePath());

    node->addNode(std::make_unique<ProjectExplorer::FileNode>(absoluteFileName, ProjectExplorer::FileType::Project));

}

MesonSubDirNode *TreeBuilder::createMesonSubDirNode(ProjectExplorer::FolderNode *parentNode, QString parentRelativeName, Utils::FilePath absoluteFileName)
{
    const QString displayName = parentRelativeName.mid(0, parentRelativeName.size() - 12); // Remove /meson.build suffix
    auto msn = std::make_unique<MesonSubDirNode>(absoluteFileName);
    msn->setDisplayName(displayName);
    MesonSubDirNode *out = msn.get();
    parentNode->addNode(move(msn));
    return out;
}

void TreeBuilder::createOtherBuildsystemFileNode(ProjectExplorer::FolderNode *parentNode, Utils::FilePath absoluteFilename)
{
    parentNode->addNestedNode(std::make_unique<ProjectExplorer::FileNode>(absoluteFilename, ProjectExplorer::FileType::Project),
    {}, [](const Utils::FilePath &fn) {
        auto node = std::make_unique<ProjectExplorer::FolderNode>(fn);
        //node->setIcon(Core::FileIconProvider::directoryIcon(":/projectexplorer/images/fileoverlay_ui.png"));
        return node;
    });
}

ProjectExplorer::FolderNode *TreeBuilder::createSubProjectsNode(ProjectExplorer::FolderNode *parentNode)
{
    Utils::FilePath fnSubprojects = Utils::FilePath::fromString(_project.baseDir+"/"+_project.subprojectsDir);
    ProjectExplorer::FolderNode *subprojects = new SubProjectsNode(fnSubprojects, "subprojects");
    subprojects->setIcon(Core::FileIconProvider::directoryIcon(":/mesonprojectmanager/images/ui_overlay_meson.png"));
    parentNode->addNode(std::unique_ptr<ProjectExplorer::FolderNode>(subprojects));
    return subprojects;
}

}
