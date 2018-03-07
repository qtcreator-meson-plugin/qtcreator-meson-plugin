#pragma once

#include "mesonprojectpartmanager.h"

#include <projectexplorer/projectnodes.h>

namespace MesonProjectManager {

class MesonProject;

class MesonFileNode: public ProjectExplorer::FolderNode
{
public:
    MesonFileNode(MesonProject *project, const Utils::FileName &filename);
    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;

private:
    MesonProjectPartManager partMgr;
};


} // namespace MesonProjectManager
