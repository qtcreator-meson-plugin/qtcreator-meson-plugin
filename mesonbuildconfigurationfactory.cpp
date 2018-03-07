#include "mesonbuildconfigurationfactory.h"
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/target.h>
#include <projectexplorer/project.h>
#include <projectexplorer/buildinfo.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <qtsupport/qtkitinformation.h>
#include <utils/qtcassert.h>

#include "mesonproject.h"
#include "src/ninjamakestep.h"

#include <QDebug>
#include <QWidget>

const char MESON_BC_ID[] = "MesonProjectManager.MesonBuildConfiguration";
const char MESON_BC_MESON_PATH[] = "MesonProjectManager.MesonBuildConfiguration.MesonPath";

MesonProjectManager::MesonBuildConfiguration::MesonBuildConfiguration(ProjectExplorer::Target *parent)
    : BuildConfiguration(parent, Core::Id(MESON_BC_ID)){

}

ProjectExplorer::NamedWidget *MesonProjectManager::MesonBuildConfiguration::createConfigWidget()
{
    auto w = new ProjectExplorer::NamedWidget();
    //w->setDisplayName(tr("Meson Manager"));
    return w;
}

ProjectExplorer::BuildConfiguration::BuildType MesonProjectManager::MesonBuildConfiguration::buildType() const
{
    return ProjectExplorer::BuildConfiguration::BuildType::Unknown;
}

bool MesonProjectManager::MesonBuildConfiguration::fromMap(const QVariantMap &map)
{
    if (!BuildConfiguration::fromMap(map))
        return false;

    m_mesonPath = map.value(MESON_BC_MESON_PATH).toString();

    return true;
}

QVariantMap MesonProjectManager::MesonBuildConfiguration::toMap() const
{
    QVariantMap map(BuildConfiguration::toMap());
    map.insert(MESON_BC_MESON_PATH, m_mesonPath);
    return map;
}

QString MesonProjectManager::MesonBuildConfiguration::mesonPath() const
{
    return m_mesonPath;
}

void MesonProjectManager::MesonBuildConfiguration::setMesonPath(const QString &mesonPath)
{
    m_mesonPath = mesonPath;
}

MesonProjectManager::MesonBuildConfigurationFactory::MesonBuildConfigurationFactory(QObject *parent):
    IBuildConfigurationFactory(parent)
{

}

int MesonProjectManager::MesonBuildConfigurationFactory::priority(const ProjectExplorer::Target *parent) const
{
    return correctProject(parent) ? 0 : -1;
}

ProjectExplorer::BuildInfo *MesonProjectManager::MesonBuildConfigurationFactory::createBuildInfo(const ProjectExplorer::Kit *k,
                                                                                      const QString &projectPath,
                                                                                      ProjectExplorer::BuildConfiguration::BuildType type) const
{
    ProjectExplorer::BuildInfo *info = new ProjectExplorer::BuildInfo(this);
    info->displayName = tr("Debug");
    QString suffix = tr("Debug", "Shadow build directory suffix");

    info->typeName = info->displayName;
    // Leave info->buildDirectory unset;
    info->kitId = k->id();

    //info->buildType = type;
    return info;
}

bool MesonProjectManager::MesonBuildConfigurationFactory::correctProject(const ProjectExplorer::Target *parent) const
{
    return qobject_cast<MesonProject*>(parent->project());
}


QList<ProjectExplorer::BuildInfo *> MesonProjectManager::MesonBuildConfigurationFactory::availableBuilds(const ProjectExplorer::Target *parent) const
{
    QList<ProjectExplorer::BuildInfo *> result;

    // These are the options in the add menu of the build settings page.

    /*const QString projectFilePath = parent->project()->projectFilePath().toString();

    ProjectExplorer::BuildInfo *info = createBuildInfo(parent->kit(), projectFilePath, ProjectExplorer::BuildConfiguration::Debug);
    info->displayName.clear(); // ask for a name
    info->buildDirectory.clear(); // This depends on the displayName
    result << info;*/
    return result;
}

int MesonProjectManager::MesonBuildConfigurationFactory::priority(const ProjectExplorer::Kit *k, const QString &projectPath) const
{
    return 0;
}

QList<ProjectExplorer::BuildInfo *> MesonProjectManager::MesonBuildConfigurationFactory::availableSetups(const ProjectExplorer::Kit *k, const QString &projectPath) const
{
    // Used in initial setup. Returning nothing breaks easy import in setup dialog.

    //qDebug()<<__PRETTY_FUNCTION__<<" TODO";
    QList<ProjectExplorer::BuildInfo *> result;
    ProjectExplorer::BuildInfo *info = createBuildInfo(k, projectPath, ProjectExplorer::BuildConfiguration::BuildType::Unknown);
    //: The name of the build configuration created by default for a generic project.
    info->displayName = tr("Default");
    result << info;
    return result;
}

ProjectExplorer::BuildConfiguration *MesonProjectManager::MesonBuildConfigurationFactory::create(ProjectExplorer::Target *parent, const ProjectExplorer::BuildInfo *info) const
{
    QTC_ASSERT(info->factory() == this, return 0);
    QTC_ASSERT(info->kitId == parent->kit()->id(), return 0);
    QTC_ASSERT(!info->displayName.isEmpty(), return 0);

    const MesonBuildInfo *mInfo = static_cast<const MesonBuildInfo *>(info);

    auto bc = new MesonBuildConfiguration(parent);
    bc->setBuildDirectory(info->buildDirectory);
    bc->setDisplayName(info->displayName);
    bc->setDefaultDisplayName(info->displayName);
    bc->setMesonPath(mInfo->mesonPath);

    ProjectExplorer::BuildStepList *buildSteps = bc->stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    ProjectExplorer::BuildStepList *cleanSteps = bc->stepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);

    Q_ASSERT(buildSteps);
    auto makeStep = new NinjaMakeStep(buildSteps);
    buildSteps->insertStep(0, makeStep);
    makeStep->setBuildTarget("all", /* on = */ true);

    Q_ASSERT(cleanSteps);
    auto cleanMakeStep = new NinjaMakeStep(cleanSteps);
    cleanSteps->insertStep(0, cleanMakeStep);
    cleanMakeStep->setBuildTarget("clean", /* on = */ true);

    return bc;
}

bool MesonProjectManager::MesonBuildConfigurationFactory::canRestore(const ProjectExplorer::Target *parent, const QVariantMap &map) const
{
    return correctProject(parent) && ProjectExplorer::idFromMap(map) == MESON_BC_ID;
}

ProjectExplorer::BuildConfiguration *MesonProjectManager::MesonBuildConfigurationFactory::restore(ProjectExplorer::Target *parent, const QVariantMap &map)
{
    if (!canRestore(parent, map)) {
        return nullptr;
    }
    std::unique_ptr<ProjectExplorer::BuildConfiguration> bc;
    bc = std::make_unique<MesonBuildConfiguration>(parent);
    if (!bc->fromMap(map)) {
        return nullptr;
    }
    return bc.release();
}

bool MesonProjectManager::MesonBuildConfigurationFactory::canClone(const ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product) const
{
    qDebug()<<__PRETTY_FUNCTION__<<" TODO";
    return false;
}

ProjectExplorer::BuildConfiguration *MesonProjectManager::MesonBuildConfigurationFactory::clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product)
{
    qDebug()<<__PRETTY_FUNCTION__<<" TODO";
    return nullptr;
}
