#pragma once

#include <projectexplorer/project.h>
//#include <projectexplorer/runconfiguration.h>

#include <QStringList>
#include <QFutureInterface>
#include <QTimer>
#include <QFuture>

namespace xxxMeson {

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

protected:
    RestoreResult fromMap(const QVariantMap &map, QString *errorMessage) override;
};

}
