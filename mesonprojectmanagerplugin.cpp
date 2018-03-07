#include "mesonprojectmanagerplugin.h"
#include "mesonprojectmanagerconstants.h"
#include "mesonbuildconfigurationfactory.h"

#include <coreplugin/icore.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/fileiconprovider.h>

#include <projectexplorer/buildmanager.h>
#include <projectexplorer/session.h>
#include <projectexplorer/projectmanager.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/target.h>
#include <projectexplorer/projectexplorerconstants.h>

#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorconstants.h>

#include <utils/hostosinfo.h>
#include <utils/parameteraction.h>

#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

#include "constants.h"
#include "mesonproject.h"
#include "mesonprojectwizard.h"
#include "src/ninjamakestep.h"

using namespace MesonProjectManager;
using namespace Core;
using namespace ProjectExplorer;

namespace MesonProjectManager {
namespace Internal {

MesonProjectManagerPlugin::MesonProjectManagerPlugin()
{
    // Create your members
}

MesonProjectManagerPlugin::~MesonProjectManagerPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
}

bool MesonProjectManagerPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize function, a plugin can be sure that the plugins it
    // depends on have initialized their members.

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    const Context projectContext(PROJECT_ID);
    Context projecTreeContext(ProjectExplorer::Constants::C_PROJECT_TREE);

    Core::FileIconProvider::registerIconOverlayForFilename(":/projectexplorer/images/projectexplorer.png", "meson.build");
    Core::FileIconProvider::registerIconOverlayForFilename(":/projectexplorer/images/projectexplorer.png", "meson_options.txt");

    ProjectManager::registerProjectType<MesonProject>(PROJECT_MIMETYPE);

    IWizardFactory::registerFactoryCreator([] {
        return QList<IWizardFactory *> {
            new MesonProjectWizard
        };
    });

    addAutoReleasedObject(new MesonBuildConfigurationFactory());
    addAutoReleasedObject(new NinjaMakeStepFactory());

    //addAutoReleasedObject(new CustomWizardMetaFactory<CustomQmakeProjectWizard>
//                          (QLatin1String("qmakeproject"), IWizardFactory::ProjectWizard));

//    auto action = new QAction(tr("MesonProjectManager Action"), this);
//    Core::Command *cmd = Core::ActionManager::registerAction(action, Constants::ACTION_ID,
//                                                             Core::Context(Core::Constants::C_GLOBAL));
//    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
//    connect(action, &QAction::triggered, this, &MesonProjectManagerPlugin::triggerAction);

//    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::MENU_ID);
//    menu->menu()->setTitle(tr("MesonProjectManager"));
//    menu->addAction(cmd);
//    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    return true;
}

void MesonProjectManagerPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag MesonProjectManagerPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

void MesonProjectManagerPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::mainWindow(),
                             tr("Action Triggered"),
                             tr("This is an action from MesonProjectManager."));
}

} // namespace Internal
} // namespace MesonProjectManager
