#pragma once

namespace MesonProjectManager {

const char PROJECT_ID[] = "Meson.MesonProject";
const char PROJECT_MIMETYPE[] = "application/vnd.meson.build";
const char MESONPROJECT_ID[] = "MesonProjectManager.MesonProject";

const char NINJA_MS_ID[] = "MesonProjectManager.NinjaMakeStep";
const char BUILD_TARGETS_KEY[] = "MesonProjectManager.NinjaMakeStep.BuildTargets";
const char NINJA_ARGUMENTS_KEY[] = "MesonProjectManager.NinjaMakeStep.NinjaArguments";
const char NINJA_COMMAND_KEY[] = "MesonProjectManager.NinjaMakeStep.NinjaCommand";

const char MESON_BC_ID[] = "MesonProjectManager.MesonBuildConfiguration";
const char MESON_BC_MESON_PATH[] = "MesonProjectManager.MesonBuildConfiguration.MesonPath";

const char MESON_BI_MESON_PATH[] = "MesonProjectManager.BuildInfo.MesonPath";

namespace Priorities {
    //const int MesonBuildFile: DefaultProjectFilePriority (500000)
    const int SubDir = 9;
    const int EditableGroup = 8;
    const int Target = 7;
    const int ExtraFiles = -100;
    const int GeneratedFiles = -101;
    const int Subproject = -1000;
}

}

