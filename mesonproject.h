#pragma once

#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>
//#include <projectexplorer/runconfiguration.h>
#include "mesonprojectparser.h"

#include <QStringList>
#include <QFutureInterface>
#include <QTimer>
#include <QFuture>

namespace xxxMeson {

class MesonProject;

class MesonProjectNode: public ProjectExplorer::ProjectNode
{
    Q_OBJECT
public:
    MesonProjectNode(MesonProject *project);

    // Node interface
public:
    bool supportsAction(ProjectExplorer::ProjectAction action, ProjectExplorer::Node *node) const override;
    MesonProject *project;
    ProjectExplorer::ProjectDocument *meson_build;
};

class MesonProject : public ProjectExplorer::Project
{
public:
    explicit MesonProject(const Utils::FileName &proFile);

    // Project interface
public:
    bool supportsKit(ProjectExplorer::Kit *k, QString *errorMessage) const override;
    QStringList filesGeneratedFrom(const QString &sourceFile) const override;
    bool needsConfiguration() const override;
    void configureAsExampleProject(const QSet<Core::Id> &platforms) override;
    bool requiresTargetPanel() const override;
    ProjectExplorer::ProjectImporter *projectImporter() const override;

    void refresh();
    void regenerateProjectFile();
    MesonBuildParser parser;

    const Utils::FileName filename;
protected:
    RestoreResult fromMap(const QVariantMap &map, QString *errorMessage) override;
};

}
