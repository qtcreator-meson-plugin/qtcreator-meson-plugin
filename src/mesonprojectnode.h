#pragma once

#include "mesonprojectpartmanager.h"
#include "pathresolver.h"

#include <projectexplorer/projectnodes.h>

namespace MesonProjectManager {

class MesonProjectPartManager;
class MesonProject;

class MesonProjectNode: public ProjectExplorer::ProjectNode
{
public:
    MesonProjectNode(MesonProject *project, const Utils::FileName &filename);
    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;
    PathResolver::DirectoryInfo getBaseDirectoryInfo() const;

private:
    MesonProjectPartManager partMgr;
};


} // namespace MesonProjectManager
