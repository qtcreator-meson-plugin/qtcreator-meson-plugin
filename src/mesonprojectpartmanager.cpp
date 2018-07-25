#include "mesonprojectpartmanager.h"

#include "constants.h"
#include "filelistnode.h"
#include "mesonfilesubfoldernode.h"
#include "mesonproject.h"

#include <coreplugin/documentmanager.h>
#include <coreplugin/fileiconprovider.h>
#include <coreplugin/icore.h>

namespace MesonProjectManager {

MesonProjectPartManager::MesonProjectPartManager(ProjectExplorer::FolderNode *node, MesonProject *project, const Utils::FileName &filename)
    : project(project)
{
    meson_build = std::make_unique<ProjectExplorer::ProjectDocument>(MesonProjectManager::PROJECT_MIMETYPE, filename, [project] {
        project->refresh();
    });

    node->addNode(std::make_unique<ProjectExplorer::FileNode>(Utils::FileName::fromString(filename.toString()), ProjectExplorer::FileType::Project, false));

    parser.reset(new MesonBuildFileParser(filename.toString()));
    parser->parse();

    for(const auto &listName: parser->fileListNames())
    {
        auto listNode = std::make_unique<FileListNode>(this, &parser->fileList(listName), Utils::FileName::fromString(parser->getProject_base()),1);
        listNode->setIcon(Core::FileIconProvider::directoryIcon(":/projectexplorer/images/fileoverlay_ui.png"));
        listNode->setDisplayName(listName);
        for(const auto &fname: parser->fileListAbsolute(listName)) {
            listNode->addNestedNode(std::make_unique<ProjectExplorer::FileNode>(Utils::FileName::fromString(fname), ProjectExplorer::FileType::Source, false),
            {}, [](const Utils::FileName &fn) {
                auto node = std::make_unique<MesonFileSubFolderNode>(fn);
                node->setIcon(Core::FileIconProvider::directoryIcon(":/projectexplorer/images/fileoverlay_ui.png"));
                return node;
            });
            project->filesInEditableGroups.insert(fname);

            const QStringList headers = getAllHeadersFor(fname);
            for(const QString &header: headers) {
                listNode->addNestedNode(std::make_unique<ProjectExplorer::FileNode>(Utils::FileName::fromString(header), ProjectExplorer::FileType::Header, false),
                {}, [](const Utils::FileName &fn) {
                    auto node = std::make_unique<MesonFileSubFolderNode>(fn);
                    node->setIcon(Core::FileIconProvider::directoryIcon(":/projectexplorer/images/fileoverlay_ui.png"));
                    return node;
                });
            }
        }
        node->addNode(move(listNode));
    }
}

void MesonProjectPartManager::regenerateProjectFiles()
{
    Core::FileChangeBlocker changeGuard(parser->filename);
    QByteArray out=parser->regenerate();
    Utils::FileSaver saver(parser->filename, QIODevice::Text);
    if(!saver.hasError()) {
        saver.write(out);
    }
    saver.finalize(Core::ICore::mainWindow());
}

} // namespace MesonProjectManager
