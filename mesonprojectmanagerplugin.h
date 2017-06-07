#pragma once

#include "mesonprojectmanager_global.h"

#include <extensionsystem/iplugin.h>

namespace MesonProjectManager {
namespace Internal {

class MesonProjectManagerPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "MesonProjectManager.json")

public:
    MesonProjectManagerPlugin();
    ~MesonProjectManagerPlugin();

    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
    ShutdownFlag aboutToShutdown();

private:
    void triggerAction();
};

} // namespace Internal
} // namespace MesonProjectManager
