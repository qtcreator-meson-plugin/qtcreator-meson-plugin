#include "mesonfilenode.h"

namespace MesonProjectManager {

MesonFileNode::MesonFileNode(MesonProject *project, const Utils::FileName &filename) :
    ProjectExplorer::FolderNode(filename.parentDir()), partMgr(this, project, filename)
{
}

bool MesonFileNode::supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const
{
    Q_UNUSED(node);
    Q_UNUSED(action);
    return false;
}

} // namespace MesonProjectManager
