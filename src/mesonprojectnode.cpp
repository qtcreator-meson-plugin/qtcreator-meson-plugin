#include "mesonprojectnode.h"

#include "mesonproject.h"

namespace MesonProjectManager {

MesonProjectNode::MesonProjectNode(MesonProject *project, const Utils::FileName &filename) :
    ProjectExplorer::ProjectNode(project->projectDirectory()), partMgr(this, project, filename)
{
}

bool MesonProjectNode::supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const
{
    Q_UNUSED(node);
    Q_UNUSED(action);
    return false;
}

} // namespace MesonProjectManager
