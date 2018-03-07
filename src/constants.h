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
}

