#include "mesonbuildconfiguration.h"

#include "constants.h"
#include "ninjamakestep.h"
#include "mesonbuildinfo.h"
#include "mesonproject.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/target.h>
#include <projectexplorer/project.h>
#include <projectexplorer/buildinfo.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <qtsupport/qtkitinformation.h>
#include <utils/qtcassert.h>
#include <utils/pathchooser.h>

#include <QDebug>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QFormLayout>

namespace MesonProjectManager {

MesonBuildConfiguration::MesonBuildConfiguration(ProjectExplorer::Target *parent, const Core::Id &id) :
    BuildConfiguration(parent, id)
{
}

void MesonBuildConfiguration::initialize(const ProjectExplorer::BuildInfo *info)
{
    BuildConfiguration::initialize(info);

    ProjectExplorer::BuildStepList *buildSteps = stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    buildSteps->appendStep(new NinjaMakeStep(buildSteps, "all"));

    ProjectExplorer::BuildStepList *cleanSteps = stepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);
    cleanSteps->appendStep(new NinjaMakeStep(cleanSteps, "clean"));

    updateCacheAndEmitEnvironmentChanged();
}

ProjectExplorer::NamedWidget *MesonBuildConfiguration::createConfigWidget()
{
    auto w = new MesonBuildConfigationWidget(this);
    return w;
}

ProjectExplorer::BuildConfiguration::BuildType MesonBuildConfiguration::buildType() const
{
    return ProjectExplorer::BuildConfiguration::BuildType::Unknown;
}

bool MesonBuildConfiguration::fromMap(const QVariantMap &map)
{
    if (!BuildConfiguration::fromMap(map))
        return false;

    m_mesonPath = map.value(MESON_BC_MESON_PATH).toString();

    return true;
}

QVariantMap MesonBuildConfiguration::toMap() const
{
    QVariantMap map(BuildConfiguration::toMap());
    map.insert(MESON_BC_MESON_PATH, m_mesonPath);
    return map;
}

QString MesonBuildConfiguration::mesonPath() const
{
    return m_mesonPath;
}

void MesonBuildConfiguration::setMesonPath(const QString &mesonPath)
{
    m_mesonPath = mesonPath;
}

MesonBuildConfigurationFactory::MesonBuildConfigurationFactory():
    IBuildConfigurationFactory()
{
    registerBuildConfiguration<MesonBuildConfiguration>(MESON_BC_ID);
    setSupportedProjectType(MESONPROJECT_ID);
    setSupportedProjectMimeTypeName(PROJECT_MIMETYPE);
}

ProjectExplorer::BuildInfo *MesonBuildConfigurationFactory::createBuildInfo(const ProjectExplorer::Kit *k,
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

bool MesonBuildConfigurationFactory::correctProject(const ProjectExplorer::Target *parent) const
{
    return qobject_cast<MesonProject*>(parent->project());
}

QList<ProjectExplorer::BuildInfo *> MesonBuildConfigurationFactory::availableBuilds(const ProjectExplorer::Target *parent) const
{
    QList<ProjectExplorer::BuildInfo *> result;

    // These are the options in the add menu of the build settings page.

    const QString projectFilePath = parent->project()->projectFilePath().toString();

    ProjectExplorer::BuildInfo *info = createBuildInfo(parent->kit(), projectFilePath, ProjectExplorer::BuildConfiguration::Debug);
    info->displayName.clear(); // ask for a name
    info->buildDirectory.clear(); // This depends on the displayName
    result << info;
    return result;
}

QList<ProjectExplorer::BuildInfo *> MesonBuildConfigurationFactory::availableSetups(const ProjectExplorer::Kit *k, const QString &projectPath) const
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

MesonBuildConfigationWidget::MesonBuildConfigationWidget(MesonBuildConfiguration *config, QWidget *parent): ProjectExplorer::NamedWidget(parent), config(config)
{
    setDisplayName(tr("Meson Project"));

    QFormLayout *layout = new QFormLayout(this);
    layout->setContentsMargins(0, -1, 0, -1);
    layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    QLineEdit *lineedit_meson_path = new QLineEdit();
    lineedit_meson_path->setText(config->mesonPath());
    layout->addRow(new QLabel(tr("Meson Path")), lineedit_meson_path);
    connect(lineedit_meson_path, &QLineEdit::textChanged, [config](QString text){
        config->setMesonPath(text);
    });

    Utils::PathChooser *build_dir = new Utils::PathChooser();
    build_dir->setPath(config->buildDirectory().toString());
    layout->addRow(new QLabel(tr("Build Directory")), build_dir);
    connect(build_dir, &Utils::PathChooser::pathChanged, [config](QString path) {
        config->setBuildDirectory(Utils::FileName::fromString(path));
    });

}

}
