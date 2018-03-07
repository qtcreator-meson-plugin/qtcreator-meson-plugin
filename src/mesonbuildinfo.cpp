#include "mesonbuildinfo.h"

namespace MesonProjectManager {

MesonBuildInfo::MesonBuildInfo(const ProjectExplorer::IBuildConfigurationFactory *f): ProjectExplorer::BuildInfo(f)
{
}

bool MesonBuildInfo::operator==(const ProjectExplorer::BuildInfo &o) const
{
    if (!ProjectExplorer::BuildInfo::operator==(o))
        return false;

    auto other = static_cast<const MesonBuildInfo *>(&o);
    return mesonPath == other->mesonPath;
}

}
