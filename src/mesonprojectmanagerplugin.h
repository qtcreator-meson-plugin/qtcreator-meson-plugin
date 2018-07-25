#pragma once

#include <extensionsystem/iplugin.h>

#include "mesonbuildconfiguration.h"
#include "ninjamakestep.h"

namespace MesonProjectManager {

class MesonProjectManagerPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "MesonProjectManager.json")

public:
    bool initialize(const QStringList &arguments, QString *errorString) override;
    void extensionsInitialized() override;
    ShutdownFlag aboutToShutdown() override;
    MesonBuildConfigurationFactory mesonBuildConfigurationFactory;
    NinjaMakeAllStepFactory ninjaMakeAllStepFactory;
    NinjaMakeCleanStepFactory ninaMakeCleanStepFactory;
};

}
