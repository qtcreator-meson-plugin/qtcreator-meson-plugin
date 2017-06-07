#pragma once
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/kit.h>

namespace xxxMeson {

class MesonBuildConfigurationFactory : public ProjectExplorer::IBuildConfigurationFactory
{
public:
    MesonBuildConfigurationFactory(QObject *parent = nullptr);

    // IBuildConfigurationFactory interface
public:
    int priority(const ProjectExplorer::Target *parent) const override;
    QList<ProjectExplorer::BuildInfo *> availableBuilds(const ProjectExplorer::Target *parent) const override;
    int priority(const ProjectExplorer::Kit *k, const QString &projectPath) const override;
    QList<ProjectExplorer::BuildInfo *> availableSetups(const ProjectExplorer::Kit *k, const QString &projectPath) const override;
    ProjectExplorer::BuildConfiguration *create(ProjectExplorer::Target *parent, const ProjectExplorer::BuildInfo *info) const override;
    bool canRestore(const ProjectExplorer::Target *parent, const QVariantMap &map) const override;
    ProjectExplorer::BuildConfiguration *restore(ProjectExplorer::Target *parent, const QVariantMap &map) override;
    bool canClone(const ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product) const override;
    ProjectExplorer::BuildConfiguration *clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product) override;
    ProjectExplorer::BuildInfo *createBuildInfo(const ProjectExplorer::Kit *k, const QString &projectPath, ProjectExplorer::BuildConfiguration::BuildType type) const;
};

}
