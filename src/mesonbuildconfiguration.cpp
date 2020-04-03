#include "mesonbuildconfiguration.h"

#include "constants.h"
#include "ninjamakestep.h"
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
#include <QVariantMap>

namespace MesonProjectManager {

MesonBuildConfiguration::MesonBuildConfiguration(ProjectExplorer::Target *parent, const Core::Id &id) :
    BuildConfiguration(parent, id)
{
}

void MesonBuildConfiguration::initialize()
{
    BuildConfiguration::initialize();

    ProjectExplorer::BuildStepList *buildSteps = stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    buildSteps->appendStep(new NinjaMakeStep(buildSteps, "all"));

    ProjectExplorer::BuildStepList *cleanSteps = stepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);
    cleanSteps->appendStep(new NinjaMakeStep(cleanSteps, "clean"));

    QVariantMap extraParams = extraInfo().toMap();
    setMesonPath(extraParams.value(MESON_BI_MESON_PATH).toString());

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
    BuildConfigurationFactory()
{
    registerBuildConfiguration<MesonBuildConfiguration>(MESON_BC_ID);
    setSupportedProjectType(MESONPROJECT_ID);
    setSupportedProjectMimeTypeName(PROJECT_MIMETYPE);
}

ProjectExplorer::BuildInfo MesonBuildConfigurationFactory::createBuildInfo(const ProjectExplorer::Kit *k,
                                                                            const QString &projectPath) const
{
    ProjectExplorer::BuildInfo info(this);
    info.displayName = tr("Debug");
    QString suffix = tr("Debug", "Shadow build directory suffix");

    info.typeName = info.displayName;
    info.buildDirectory = Utils::FilePath::fromString(QFileInfo(projectPath).absolutePath()).pathAppended("_build");
    info.kitId = k->id();
    QVariantMap extraParams;
    extraParams.insert(MESON_BI_MESON_PATH, MesonProject::findDefaultMesonExecutable().toString());
    info.extraInfo = extraParams;
    return info;
}

bool MesonBuildConfigurationFactory::correctProject(const ProjectExplorer::Target *parent) const
{
    return qobject_cast<MesonProject*>(parent->project());
}

QList<ProjectExplorer::BuildInfo> MesonBuildConfigurationFactory::availableBuilds(const ProjectExplorer::Kit *k, const Utils::FilePath &projectPath, bool forSetup) const
{
    QList<ProjectExplorer::BuildInfo> result;

    // These are the options in the add menu of the build settings page.

    ProjectExplorer::BuildInfo info = createBuildInfo(k, projectPath.toString());
    if (forSetup) {
        info.displayName = tr("Default");
        result.append(info);
    } else {
        info.displayName.clear();     // ask for a name
        info.buildDirectory.clear();  // This depends on the displayName
        result << info;
    }
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
        config->setBuildDirectory(Utils::FilePath::fromString(path));
    });

}

}
