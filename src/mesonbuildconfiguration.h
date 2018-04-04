#pragma once
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/namedwidget.h>

namespace MesonProjectManager {

class MesonBuildConfiguration : public ProjectExplorer::BuildConfiguration
{
    Q_OBJECT

public:
    explicit MesonBuildConfiguration(ProjectExplorer::Target *parent);

public:
    ProjectExplorer::NamedWidget *createConfigWidget() override;
    BuildType buildType() const override;

    QString mesonPath() const;
    void setMesonPath(const QString &mesonPath);

    bool fromMap(const QVariantMap &map) override;
    QVariantMap toMap() const override;

private:
    QString m_mesonPath;

    friend class MesonBuildConfigationWidget;
};

class MesonBuildConfigationWidget: public ProjectExplorer::NamedWidget
{
    Q_OBJECT
public:
    explicit MesonBuildConfigationWidget(MesonBuildConfiguration *config, QWidget *parent=nullptr);
private:
    MesonBuildConfiguration *config;
};

class MesonBuildConfigurationFactory : public ProjectExplorer::IBuildConfigurationFactory
{
public:
    MesonBuildConfigurationFactory(QObject *parent = nullptr);

    int priority(const ProjectExplorer::Target *parent) const override;
    int priority(const ProjectExplorer::Kit *k, const QString &projectPath) const override;

    QList<ProjectExplorer::BuildInfo *> availableBuilds(const ProjectExplorer::Target *parent) const override;
    QList<ProjectExplorer::BuildInfo *> availableSetups(const ProjectExplorer::Kit *k, const QString &projectPath) const override;

    ProjectExplorer::BuildConfiguration *create(ProjectExplorer::Target *parent, const ProjectExplorer::BuildInfo *info) const override;
    ProjectExplorer::BuildInfo *createBuildInfo(const ProjectExplorer::Kit *k, const QString &projectPath, ProjectExplorer::BuildConfiguration::BuildType type) const;

    bool canRestore(const ProjectExplorer::Target *parent, const QVariantMap &map) const override;
    ProjectExplorer::BuildConfiguration *restore(ProjectExplorer::Target *parent, const QVariantMap &map) override;

    bool canClone(const ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product) const override;
    ProjectExplorer::BuildConfiguration *clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *product) override;

private:
    bool correctProject(const ProjectExplorer::Target *parent) const;
};

}
