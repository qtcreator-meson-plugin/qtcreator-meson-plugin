#pragma once

#include <projectexplorer/buildinfo.h>

namespace MesonProjectManager {

class MesonBuildInfo : public ProjectExplorer::BuildInfo
{
public:
    MesonBuildInfo(const ProjectExplorer::IBuildConfigurationFactory *f);

    bool operator==(const BuildInfo &o) const;

    QString mesonPath;
};

}
