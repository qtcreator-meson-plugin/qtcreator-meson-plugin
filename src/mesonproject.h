#pragma once

#include "mesonbuildfileparser.h"
#include "mesonprojectnode.h"

#include <cpptools/cppprojectupdater.h>

#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>

#include <QStringList>
#include <QFutureInterface>
#include <QTimer>
#include <QFuture>

#include <memory>

namespace MesonProjectManager {

class CompileCommandInfo
{
public:
    QMap<QString, QString> defines;
    QStringList includes;
    QString cpp_std;

    bool operator==(const CompileCommandInfo &o) const;
};

class MesonProject : public ProjectExplorer::Project
{
    Q_OBJECT

public:
    explicit MesonProject(const Utils::FileName &proFile);
    virtual ~MesonProject();

    bool supportsKit(ProjectExplorer::Kit *k, QString *errorMessage) const override;
    bool setupTarget(ProjectExplorer::Target *t);
    QStringList filesGeneratedFrom(const QString &sourceFile) const override;
    bool needsConfiguration() const override;
    void configureAsExampleProject(const QSet<Core::Id> &platforms) override;
    bool requiresTargetPanel() const override;
    ProjectExplorer::ProjectImporter *projectImporter() const override;

    void mesonIntrospectBuildsytemFiles(MesonProjectNode *root);
    void mesonIntrospectProjectInfo();
    const QHash<CompileCommandInfo, QStringList> parseCompileCommands() const;

    void refreshCppCodeModel(const QHash<CompileCommandInfo, QStringList> &);

private:
    const Utils::FileName filename;
    CppTools::CppProjectUpdater *cppCodeModelUpdater = nullptr;

protected:
    RestoreResult fromMap(const QVariantMap &map, QString *errorMessage) override;

public slots:
    void refresh();
};

}
