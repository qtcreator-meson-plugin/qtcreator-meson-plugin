#include "mesonprojectmanagerplugin.h"

#include "constants.h"
#include "ninjamakestep.h"
#include "mesonbuildconfiguration.h"
#include "mesonproject.h"
#include "../mesonprojectwizard.h"

#include <coreplugin/fileiconprovider.h>

#include <projectexplorer/projectmanager.h>

namespace MesonProjectManager {

bool MesonProjectManagerPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    Core::FileIconProvider::registerIconOverlayForFilename(":/projectexplorer/images/projectexplorer.png", "meson.build");
    Core::FileIconProvider::registerIconOverlayForFilename(":/projectexplorer/images/projectexplorer.png", "meson_options.txt");

    ProjectExplorer::ProjectManager::registerProjectType<MesonProject>(PROJECT_MIMETYPE);

    Core::IWizardFactory::registerFactoryCreator([] {
        return QList<Core::IWizardFactory *> { new MesonProjectWizard };
    });

    addAutoReleasedObject(new MesonBuildConfigurationFactory());
    addAutoReleasedObject(new NinjaMakeStepFactory());

    return true;
}

void MesonProjectManagerPlugin::extensionsInitialized()
{
}

ExtensionSystem::IPlugin::ShutdownFlag MesonProjectManagerPlugin::aboutToShutdown()
{
    return SynchronousShutdown;
}

} // namespace MesonProjectManager
