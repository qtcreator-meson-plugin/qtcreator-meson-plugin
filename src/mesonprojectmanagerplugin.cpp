#include "mesonprojectmanagerplugin.h"

#include "constants.h"
#include "mesonproject.h"
#include "mesonprojectwizard.h"

#include <coreplugin/fileiconprovider.h>

#include <projectexplorer/projectmanager.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/projectexplorer.h>

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>

#include <utils/parameteraction.h>

namespace MesonProjectManager {

bool MesonProjectManagerPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    Core::FileIconProvider::registerIconOverlayForFilename(":/mesonprojectmanager/images/ui_overlay_meson.png", "meson.build");
    Core::FileIconProvider::registerIconOverlayForFilename(":/mesonprojectmanager/images/ui_overlay_meson.png", "meson_options.txt");

    ProjectExplorer::ProjectManager::registerProjectType<MesonProject>(PROJECT_MIMETYPE);

    Core::IWizardFactory::registerFactoryCreator([] {
        return QList<Core::IWizardFactory *> { new MesonProjectWizard };
    });

    Core::ActionContainer *mproject = Core::ActionManager::actionContainer(ProjectExplorer::Constants::M_PROJECTCONTEXT);

    auto configureProjectAction = new QAction(tr("Configure..."), this);
    const Core::Context projectContext(MesonProjectManager::MESONPROJECT_ID);

    auto command = Core::ActionManager::registerAction(configureProjectAction, "Meson.BuildTargetContextMenu", projectContext);
    command->setAttribute(Core::Command::CA_Hide);

    mproject->addAction(command, ProjectExplorer::Constants::G_PROJECT_FILES);
    connect(configureProjectAction, &QAction::triggered, [] {
        auto projecttree = ProjectExplorer::ProjectTree::instance();
        MesonProject *mp = dynamic_cast<MesonProject*>(projecttree->currentProject());
        if(mp) {
            mp->editOptions();
        }
    });

    connect(ProjectExplorer::ProjectTree::instance(), &ProjectExplorer::ProjectTree::currentNodeChanged, [=] {
        auto projecttree = ProjectExplorer::ProjectTree::instance();

        MesonProject *mp = dynamic_cast<MesonProject*>(projecttree->currentProject());
        const bool canConfigure = mp!=nullptr && mp->canConfigure();
        configureProjectAction->setVisible(canConfigure);
        configureProjectAction->setEnabled(canConfigure);
    });

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
