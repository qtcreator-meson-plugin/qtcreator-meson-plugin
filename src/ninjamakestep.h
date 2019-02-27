#pragma once

#include <projectexplorer/abstractprocessstep.h>

namespace MesonProjectManager {

class NinjaMakeStep : public ProjectExplorer::AbstractProcessStep
{
    friend class NinjaMakeStepConfigWidget;
    friend class NinjaMakeStepFactory;

public:
    NinjaMakeStep(ProjectExplorer::BuildStepList *bsl, const QString target);

    bool init() override;

    ProjectExplorer::BuildStepConfigWidget *createConfigWidget() override;

    bool buildsTarget(const QString &target) const;
    void setBuildTarget(const QString &target, bool on);
    QString allArguments() const;
    QString ninjaCommand() const;

    void setClean(bool clean);
    bool isClean() const;

    QVariantMap toMap() const override;

    QString getSummary();

protected:
    bool fromMap(const QVariantMap &map) override;

    void stdOutput(const QString &line) override;

private:
    bool setupPP(ProjectExplorer::ProcessParameters &pp);

    QStringList m_buildTargets;
    QString m_ninjaArguments;
    QString m_ninjaCommand;

    QRegExp m_ninjaProgress;

    friend class MesonBuildConfiguration;
};

class NinjaMakeAllStepFactory : public ProjectExplorer::BuildStepFactory
{
public:
    NinjaMakeAllStepFactory();
};

class NinjaMakeCleanStepFactory: public ProjectExplorer::BuildStepFactory
{
public:
    NinjaMakeCleanStepFactory();
};


}
