#include "mesonproject.h"
#include "constants.h"

#include <QSaveFile>
#include <memory>
#include <projectexplorer/projectimporter.h>
#include <projectexplorer/buildinfo.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectnodes.h>
#include <coreplugin/icontext.h>
#include "mesonbuildconfigurationfactory.h"
#include "mesonprojectimporter.h"

namespace xxxMeson {

class FileListNode : public ProjectExplorer::VirtualFolderNode {
public:
    explicit FileListNode(MesonBuildParser::ChunkInfo *chunk, const Utils::FileName &folderPath, int priority)
        : ProjectExplorer::VirtualFolderNode(folderPath, priority), chunk(chunk)
    {
    }

    // FolderNode interface
public:
    bool addFiles(const QStringList &filePaths, QStringList *notAdded) override {
        MesonProjectNode* projectNode = qobject_cast<MesonProjectNode*>(managingProject());
        if(projectNode)
        {

            for(const auto &fp: filePaths)
            {
                auto fn = Utils::FileName::fromString(fp);
                QString relative_fn = fn.relativeChildPath(Utils::FileName::fromString(projectNode->project->parser.getProject_base())).toString();
                if(!chunk->file_list.contains(relative_fn))
                    chunk->file_list.append(relative_fn);
            }
            //    addNestedNode(new ProjectExplorer::FileNode(Utils::FileName::fromString(fp), ProjectExplorer::FileType::Source, false));

            projectNode->project->regenerateProjectFile();
            projectNode->project->refresh();
            return true;
        } else {
            return false;
        }
    }
//    bool removeFiles(const QStringList &filePaths, QStringList *notRemoved) override;
//    bool deleteFiles(const QStringList &filePaths) override;
//    bool canRenameFile(const QString &filePath, const QString &newFilePath) override;
//    bool renameFile(const QString &filePath, const QString &newFilePath) override;
private:
    MesonBuildParser::ChunkInfo *chunk;
};

MesonProject::MesonProject(const Utils::FileName &filename):
    Project(PROJECT_MIMETYPE, filename), filename(filename), parser{filename.toString()}
{
    setId(MESONPROJECT_ID);
    setProjectContext(Core::Context(MESONPROJECT_ID));
    setProjectLanguages(Core::Context(ProjectExplorer::Constants::CXX_LANGUAGE_ID));
    setDisplayName(filename.toFileInfo().completeBaseName());

    parser.parse();
    refresh();
}

void MesonProject::refresh()
{
    // Stuff stolen from genericproject::refresh
    auto root = new MesonProjectNode(this);
    root->addNode(new ProjectExplorer::FileNode(Utils::FileName::fromString(filename.toString()), ProjectExplorer::FileType::Project, false));

    for(const auto &listName: parser.fileListNames())
    {
        auto listNode = new FileListNode(&parser.fileList(listName), Utils::FileName::fromString(parser.getProject_base()),1);
        listNode->setDisplayName(listName);
        //listNode->addFiles(parser.fileList(listName).file_list);
        for(const auto &fname: parser.fileListAbsolute(listName))
        {
            listNode->addNestedNode(new ProjectExplorer::FileNode(Utils::FileName::fromString(fname), ProjectExplorer::FileType::Source, false));
        }
        root->addNode(listNode);
    }
    setRootProjectNode(root);
    emit parsingFinished();
}

void MesonProject::regenerateProjectFile()
{
    QByteArray out=parser.regenerate();
    QSaveFile file(filename.toString());
    file.open(QFile::WriteOnly);
    file.write(out);
    file.commit();
}

bool MesonProject::supportsKit(ProjectExplorer::Kit *k, QString *errorMessage) const
{
    Q_UNUSED(k)
    Q_UNUSED(errorMessage)
    return true;
}

QStringList MesonProject::filesGeneratedFrom(const QString &sourceFile) const
{
    return {};
}

bool MesonProject::needsConfiguration() const
{
    return false;
}

void MesonProject::configureAsExampleProject(const QSet<Core::Id> &platforms)
{

}

bool MesonProject::requiresTargetPanel() const
{
    return false;
}

ProjectExplorer::ProjectImporter *MesonProject::projectImporter() const
{
    return new MesonProjectImporter(projectFilePath());
}

ProjectExplorer::Project::RestoreResult MesonProject::fromMap(const QVariantMap &map, QString *errorMessage)
{
    RestoreResult result = Project::fromMap(map, errorMessage);
    if (result != RestoreResult::Ok)
        return result;
    return RestoreResult::Ok;
}

MesonProjectNode::MesonProjectNode(MesonProject *project): ProjectExplorer::ProjectNode(project->projectDirectory()), project(project)
{

}

bool MesonProjectNode::supportsAction(ProjectExplorer::ProjectAction action, ProjectExplorer::Node *node) const
{
    Q_UNUSED(node);
    return action == ProjectExplorer::AddNewFile
        || action == ProjectExplorer::AddExistingFile
        || action == ProjectExplorer::AddExistingDirectory
        || action == ProjectExplorer::RemoveFile
        || action == ProjectExplorer::Rename;
}

}
