#pragma once

#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>
//#include <projectexplorer/runconfiguration.h>
#include "mesonprojectparser.h"

#include <QStringList>
#include <QFutureInterface>
#include <QTimer>
#include <QFuture>

#include <cpptools/cppprojectupdater.h>

namespace xxxMeson {

class MesonProject;

class MesonProjectNode: public ProjectExplorer::ProjectNode
{
public:
    MesonProjectNode(MesonProject *project);

    // Node interface
public:
    bool supportsAction(ProjectExplorer::ProjectAction action, const ProjectExplorer::Node *node) const override;
    MesonProject *project;
    ProjectExplorer::ProjectDocument *meson_build;
};

class MesonProject : public ProjectExplorer::Project
{
    Q_OBJECT

public:
    explicit MesonProject(const Utils::FileName &proFile);
    virtual ~MesonProject();

    // Project interface
public:
    bool supportsKit(ProjectExplorer::Kit *k, QString *errorMessage) const override;
    bool setupTarget(ProjectExplorer::Target *t);
    QStringList filesGeneratedFrom(const QString &sourceFile) const override;
    bool needsConfiguration() const override;
    void configureAsExampleProject(const QSet<Core::Id> &platforms) override;
    bool requiresTargetPanel() const override;
    ProjectExplorer::ProjectImporter *projectImporter() const override;

    void regenerateProjectFile();
    std::unique_ptr<MesonBuildParser> parser;

    const Utils::FileName filename;

    void refreshCppCodeModel();
    CppTools::CppProjectUpdater *m_cppCodeModelUpdater = nullptr;
protected:
    RestoreResult fromMap(const QVariantMap &map, QString *errorMessage) override;

public slots:
    void refresh();
};

}
