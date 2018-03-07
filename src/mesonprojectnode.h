#pragma once

#include "mesonprojectpartmanager.h"

#include <projectexplorer/projectnodes.h>

namespace MesonProjectManager {

class MesonProjectPartManager;
class MesonProject;

class MesonProjectNode: public ProjectExplorer::ProjectNode
{
public:
    MesonProjectNode(MesonProject *project, const Utils::FileName &filename);
    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;

private:
    MesonProjectPartManager partMgr;
};


} // namespace MesonProjectManager
