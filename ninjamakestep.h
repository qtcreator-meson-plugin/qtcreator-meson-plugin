#pragma once

#include <projectexplorer/abstractprocessstep.h>

namespace xxxMeson {

class NinjaMakeStep : public ProjectExplorer::AbstractProcessStep
{
    Q_OBJECT

    friend class NinjaMakeStepConfigWidget;
    friend class NinjaMakeStepFactory;

public:
    explicit NinjaMakeStep(ProjectExplorer::BuildStepList *parent);

    bool init(QList<const BuildStep *> &earlierSteps) override;
    void run(QFutureInterface<bool> &fi) override;

    ProjectExplorer::BuildStepConfigWidget *createConfigWidget() override;
    bool immutable() const override;
    bool buildsTarget(const QString &target) const;
    void setBuildTarget(const QString &target, bool on);
    QString allArguments() const;
    QString ninjaCommand(const Utils::Environment &environment) const;

    void setClean(bool clean);
    bool isClean() const;

    QVariantMap toMap() const override;

protected:
    NinjaMakeStep(ProjectExplorer::BuildStepList *parent, NinjaMakeStep *bs);
    NinjaMakeStep(ProjectExplorer::BuildStepList *parent, Core::Id id);
    bool fromMap(const QVariantMap &map) override;

    void stdOutput(const QString &line);

private:
    void ctor(ProjectExplorer::BuildStepList *bsl);

    QStringList m_buildTargets;
    QString m_ninjaArguments;
    QString m_ninjaCommand;

    QRegExp m_ninjaProgress;
};

class NinjaMakeStepConfigWidget : public ProjectExplorer::BuildStepConfigWidget
{
    Q_OBJECT

public:
    explicit NinjaMakeStepConfigWidget(NinjaMakeStep *makeStep);

    QString displayName() const override;
    QString summaryText() const override;
};

class NinjaMakeStepFactory : public ProjectExplorer::IBuildStepFactory
{
    Q_OBJECT

public:
    explicit NinjaMakeStepFactory(QObject *parent = nullptr);

    QList<ProjectExplorer::BuildStepInfo>
        availableSteps(ProjectExplorer::BuildStepList *parent) const override;

    ProjectExplorer::BuildStep *create(ProjectExplorer::BuildStepList *parent, Core::Id id) override;
    ProjectExplorer::BuildStep *clone(ProjectExplorer::BuildStepList *parent,
                                      ProjectExplorer::BuildStep *source) override;
};


}
