#pragma once
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/namedwidget.h>

namespace MesonProjectManager {

class MesonBuildConfiguration : public ProjectExplorer::BuildConfiguration
{
    Q_OBJECT

public:
    explicit MesonBuildConfiguration(ProjectExplorer::Target *parent, const Core::Id &id);

public:
    void initialize() override;

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

class MesonBuildConfigurationFactory : public ProjectExplorer::BuildConfigurationFactory
{
public:
    MesonBuildConfigurationFactory();

    QList<ProjectExplorer::BuildInfo> availableBuilds( const ProjectExplorer::Kit *k, const Utils::FilePath &projectPath, bool forSetup) const override;
    ProjectExplorer::BuildInfo createBuildInfo(const ProjectExplorer::Kit *k, const QString &projectPath) const;

private:
    bool correctProject(const ProjectExplorer::Target *parent) const;
};

}
