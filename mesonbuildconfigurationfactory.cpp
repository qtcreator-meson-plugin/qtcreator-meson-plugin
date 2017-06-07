#include "mesonbuildconfigurationfactory.h"
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/target.h>
#include <projectexplorer/project.h>
#include <projectexplorer/buildinfo.h>
#include <qtsupport/qtkitinformation.h>

#include <QDebug>

xxxMeson::MesonBuildConfigurationFactory::MesonBuildConfigurationFactory(QObject *parent):
    IBuildConfigurationFactory(parent)
{

}

int xxxMeson::MesonBuildConfigurationFactory::priority(const ProjectExplorer::Target *parent) const
{
    return 0;
}

ProjectExplorer::BuildInfo *xxxMeson::MesonBuildConfigurationFactory::createBuildInfo(const ProjectExplorer::Kit *k,
                                                                                      const QString &projectPath,
                                                                                      ProjectExplorer::BuildConfiguration::BuildType type) const
{
    ProjectExplorer::BuildInfo *info = new ProjectExplorer::BuildInfo(this);
    info->displayName = tr("Debug");
    QString suffix = tr("Debug", "Shadow build directory suffix");

    info->typeName = info->displayName;
    // Leave info->buildDirectory unset;
    info->kitId = k->id();

    info->buildType = type;
    return info;
}


QList<ProjectExplorer::BuildInfo *> xxxMeson::MesonBuildConfigurationFactory::availableBuilds(const ProjectExplorer::Target *parent) const
{
    QList<ProjectExplorer::BuildInfo *> result;

    const QString projectFilePath = parent->project()->projectFilePath().toString();

    ProjectExplorer::BuildInfo *info = createBuildInfo(parent->kit(), projectFilePath, ProjectExplorer::BuildConfiguration::Debug);
    info->displayName.clear(); // ask for a name
    info->buildDirectory.clear(); // This depends on the displayName
    result << info;
    return result;
}

int xxxMeson::MesonBuildConfigurationFactory::priority(const ProjectExplorer::Kit *k, const QString &projectPath) const
{
    return 0;
}

QList<ProjectExplorer::BuildInfo *> xxxMeson::MesonBuildConfigurationFactory::availableSetups(const ProjectExplorer::Kit *k, const QString &projectPath) const
{
    QList<ProjectExplorer::BuildInfo *> result;
    qDebug()<<__PRETTY_FUNCTION__<<" TODO";
    return result;
}

ProjectExplorer::BuildConfiguration *xxxMeson::MesonBuildConfigurationFactory::create(ProjectExplorer::Target *parent, const ProjectExplorer::BuildInfo *info) const
{
    qDebug()<<__PRETTY_FUNCTION__<<" TODO";
    // TODO
    return nullptr;
}

bool xxxMeson::MesonBuildConfigurationFactory::canRestore(const ProjectExplorer::Target *parent, const QVariantMap &map) const
{
    qDebug()<<__PRETTY_FUNCTION__<<" TODO";
    return true;
}

ProjectExplorer::BuildConfiguration *xxxMeson::MesonBuildConfigurationFactory::restore(ProjectExplorer::Target *parent, const QVariantMap &map)
{
    qDebug()<<__PRETTY_FUNCTION__<<" TODO";
    return nullptr;
}

bool xxxMeson::MesonBuildConfigurationFactory::canClone(const ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product) const
{
    qDebug()<<__PRETTY_FUNCTION__<<" TODO";
    return false;
}

ProjectExplorer::BuildConfiguration *xxxMeson::MesonBuildConfigurationFactory::clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product)
{
    qDebug()<<__PRETTY_FUNCTION__<<" TODO";
    return nullptr;
}
