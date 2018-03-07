#include "ninjamakestep.h"
#include "ninjamakestepconfigwidget.h"
#include "constants.h"

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/buildstep.h>
#include <projectexplorer/processparameters.h>
#include <projectexplorer/project.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/target.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/projectexplorerconstants.h>

namespace MesonProjectManager {

const char NINJA_MS_ID[] = "MesonProjectManager.NinjaMakeStep";

const char BUILD_TARGETS_KEY[] = "MesonProjectManager.NinjaMakeStep.BuildTargets";
const char NINJA_ARGUMENTS_KEY[] = "MesonProjectManager.NinjaMakeStep.NinjaArguments";
const char NINJA_COMMAND_KEY[] = "MesonProjectManager.NinjaMakeStep.NinjaCommand";

NinjaMakeStep::NinjaMakeStep(ProjectExplorer::BuildStepList *parent)
    : AbstractProcessStep(parent, Core::Id(NINJA_MS_ID))
{
    ctor(parent);
}

NinjaMakeStep::NinjaMakeStep(ProjectExplorer::BuildStepList *parent, const Core::Id id) :
    AbstractProcessStep(parent, id)
{
    ctor(parent);
}

NinjaMakeStep::NinjaMakeStep(ProjectExplorer::BuildStepList *parent, NinjaMakeStep *bs) :
    AbstractProcessStep(parent, bs),
    m_buildTargets(bs->m_buildTargets),
    m_ninjaArguments(bs->m_ninjaArguments),
    m_ninjaCommand(bs->m_ninjaCommand)
{
    ctor(parent);
}


void NinjaMakeStep::ctor(ProjectExplorer::BuildStepList *bsl)
{
    setDefaultDisplayName("Ninja");
    m_ninjaProgress = QRegExp("^\\[\\s*(\\d*)/\\s*(\\d*)");

    if (m_buildTargets.isEmpty()) {
        if (bsl->id() == ProjectExplorer::Constants::BUILDSTEPS_CLEAN)
            m_buildTargets.append("clean");
        else if (bsl->id() == ProjectExplorer::Constants::BUILDSTEPS_DEPLOY)
            m_buildTargets.append("install");
        else
            m_buildTargets.append("all");
    }
}

bool NinjaMakeStep::init(QList<const ProjectExplorer::BuildStep *> &earlierSteps)
{
    ProjectExplorer::BuildConfiguration *bc = buildConfiguration();
    if (!bc)
        bc = target()->activeBuildConfiguration();
    if (!bc)
        emit addTask(ProjectExplorer::Task::buildConfigurationMissingTask());

    ProjectExplorer::ToolChain *tc = ProjectExplorer::ToolChainKitInformation::toolChain(target()->kit(), ProjectExplorer::Constants::CXX_LANGUAGE_ID);
    if (!tc)
        emit addTask(ProjectExplorer::Task::compilerMissingTask());

    if (!bc || !tc) {
        emitFaultyConfigurationMessage();
        return false;
    }

    ProjectExplorer::ProcessParameters *pp = processParameters();
    pp->setMacroExpander(bc->macroExpander());
    pp->setWorkingDirectory(bc->buildDirectory().toString());
    Utils::Environment env = bc->environment();
    Utils::Environment::setupEnglishOutput(&env);
    pp->setEnvironment(env);
    pp->setCommand(ninjaCommand(bc->environment()));
    pp->setArguments(allArguments());
    pp->resolveAll();

    setIgnoreReturnValue(m_buildTargets == QStringList{ "clean" });

    //setOutputParser(new GnuMakeParser());
    ProjectExplorer::IOutputParser *parser = target()->kit()->createOutputParser();
    if (parser)
        setOutputParser(parser);
    outputParser()->setWorkingDirectory(pp->effectiveWorkingDirectory());

    return AbstractProcessStep::init(earlierSteps);
}

QVariantMap NinjaMakeStep::toMap() const
{
    QVariantMap map(AbstractProcessStep::toMap());

    map.insert(BUILD_TARGETS_KEY, m_buildTargets);
    map.insert(NINJA_ARGUMENTS_KEY, m_ninjaArguments);
    map.insert(NINJA_COMMAND_KEY, m_ninjaCommand);
    return map;
}

bool NinjaMakeStep::fromMap(const QVariantMap &map)
{
    m_buildTargets = map.value(BUILD_TARGETS_KEY).toStringList();
    m_ninjaArguments = map.value(NINJA_ARGUMENTS_KEY).toString();
    m_ninjaCommand = map.value(NINJA_COMMAND_KEY).toString();

    return BuildStep::fromMap(map);
}

void NinjaMakeStep::stdOutput(const QString &line)
{
    if (m_ninjaProgress.indexIn(line) != -1) {
        AbstractProcessStep::stdOutput(line);
        bool ok = false;
        int done = m_ninjaProgress.cap(1).toInt(&ok);
        if (ok) {
            int all = m_ninjaProgress.cap(2).toInt(&ok);
            if (ok && all != 0) {
                const int percent = static_cast<int>(100.0 * done/all);
                futureInterface()->setProgressValue(percent);
            }
        }
        return;
    }

    // kit parsers expect errors on std error, but ninja outputs everything on stdout
    AbstractProcessStep::stdError(line);
}

QString NinjaMakeStep::allArguments() const
{
    QString args = m_ninjaArguments;
    Utils::QtcProcess::addArgs(&args, "-v");
    Utils::QtcProcess::addArgs(&args, m_buildTargets);
    return args;
}

QString NinjaMakeStep::ninjaCommand(const Utils::Environment &environment) const
{
    QString command = m_ninjaCommand;
    if (command.isEmpty()) {
        command = "ninja";
    }
    return command;
}

void NinjaMakeStep::run(QFutureInterface<bool> &fi)
{
    AbstractProcessStep::run(fi);
}

ProjectExplorer::BuildStepConfigWidget *NinjaMakeStep::createConfigWidget()
{
    return new NinjaMakeStepConfigWidget(this);
}

bool NinjaMakeStep::immutable() const
{
    return false;
}

bool NinjaMakeStep::buildsTarget(const QString &target) const
{
    return m_buildTargets.contains(target);
}

void NinjaMakeStep::setBuildTarget(const QString &target, bool on)
{
    QStringList old = m_buildTargets;
    if (on && !old.contains(target))
         old << target;
    else if (!on && old.contains(target))
        old.removeOne(target);

    m_buildTargets = old;
}



NinjaMakeStepFactory::NinjaMakeStepFactory(QObject *parent) :
    IBuildStepFactory(parent)
{
}

QList<ProjectExplorer::BuildStepInfo> NinjaMakeStepFactory::availableSteps(ProjectExplorer::BuildStepList *parent) const
{
    if (parent->target()->project()->id() != MESONPROJECT_ID)
        return {};

    return {{NINJA_MS_ID, "Ninja"}};
}

ProjectExplorer::BuildStep *NinjaMakeStepFactory::create(ProjectExplorer::BuildStepList *parent, Core::Id id)
{
    Q_UNUSED(id)
    auto step = new NinjaMakeStep(parent);
    return step;
}

ProjectExplorer::BuildStep *NinjaMakeStepFactory::clone(ProjectExplorer::BuildStepList *parent, ProjectExplorer::BuildStep *source)
{
    auto old = qobject_cast<NinjaMakeStep*>(source);
    Q_ASSERT(old);
    return new NinjaMakeStep(parent, old);
}


}
