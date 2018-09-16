#pragma once

#include "mesonbuildfileparser.h"
#include "pathresolver.h"

#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>

#include <memory>

namespace MesonProjectManager {

class MesonProject;

class MesonProjectPartManager
{
public:
    MesonProjectPartManager(ProjectExplorer::FolderNode *node, MesonProject *project, const Utils::FileName &filename);

public:
    void regenerateProjectFiles();

    MesonProject *project = nullptr;
    std::unique_ptr<ProjectExplorer::ProjectDocument> meson_build;
    std::unique_ptr<MesonBuildFileParser> parser;
    PathResolver::DirectoryInfo projectBaseDirectoryInfo;
};


} // namespace MesonProjectManager
