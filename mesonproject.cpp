#include "mesonproject.h"
#include "constants.h"

#include <memory>
#include <projectexplorer/projectimporter.h>
#include <projectexplorer/buildinfo.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectnodes.h>
#include <coreplugin/icontext.h>
#include "mesonbuildconfigurationfactory.h"
#include "mesonprojectimporter.h"

namespace xxxMeson {

MesonProject::MesonProject(const Utils::FileName &filename):
    Project(PROFILE_MIMETYPE, filename)
{
    setId(MESONPROJECT_ID);
    setProjectContext(Core::Context(MESONPROJECT_ID));
    setProjectLanguages(Core::Context(ProjectExplorer::Constants::CXX_LANGUAGE_ID));
    setDisplayName(filename.toFileInfo().completeBaseName());

    // Stuff stolen from genericproject::refresh
    auto newRoot = new MesonProjectNode(this);

    newRoot->addNestedNode(new ProjectExplorer::FileNode(Utils::FileName::fromString("File List"),
                                         ProjectExplorer::FileType::Project,
                                         /* generated = */ false));
    setRootProjectNode(newRoot);
}

bool MesonProject::supportsKit(ProjectExplorer::Kit *k, QString *errorMessage) const
{
    Q_UNUSED(k)
    Q_UNUSED(errorMessage)
    return true;
}

QStringList MesonProject::filesGeneratedFrom(const QString &sourceFile) const
{
    return {};
}

bool MesonProject::needsConfiguration() const
{
    return false;
}

void MesonProject::configureAsExampleProject(const QSet<Core::Id> &platforms)
{

}

bool MesonProject::requiresTargetPanel() const
{
    return false;
}

ProjectExplorer::ProjectImporter *MesonProject::projectImporter() const
{
    return new MesonProjectImporter(projectFilePath());
}

ProjectExplorer::Project::RestoreResult MesonProject::fromMap(const QVariantMap &map, QString *errorMessage)
{
    RestoreResult result = Project::fromMap(map, errorMessage);
    if (result != RestoreResult::Ok)
        return result;
    return RestoreResult::Ok;
}

MesonProjectNode::MesonProjectNode(MesonProject *project): ProjectExplorer::ProjectNode(project->projectDirectory())
{

}

}
