#include "constants.h"
#include "fixdirectoryparser.h"
#include "ninjamakestep.h"
#include "ninjamakestepconfigwidget.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/buildstep.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/processparameters.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>
#include <projectexplorer/toolchain.h>

namespace MesonProjectManager {

NinjaMakeStep::NinjaMakeStep(ProjectExplorer::BuildStepList *bsl, const QString target):
    AbstractProcessStep(bsl, NINJA_MS_ID)
{
    setBuildTarget(target, true);
    setDefaultDisplayName("Ninja");
    m_ninjaProgress = QRegExp(R"(^\[\s*([0-9]*)/\s*([0-9]*)\])");
    if (m_buildTargets.isEmpty()) {
        if (bsl->id() == ProjectExplorer::Constants::BUILDSTEPS_CLEAN)
            m_buildTargets.append("clean");
        else if (bsl->id() == ProjectExplorer::Constants::BUILDSTEPS_DEPLOY)
            m_buildTargets.append("install");
        else
            m_buildTargets.append("all");
    }
}

bool NinjaMakeStep::setupPP(ProjectExplorer::ProcessParameters &pp)
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

    pp.setMacroExpander(bc->macroExpander());
    pp.setWorkingDirectory(bc->buildDirectory().toString());
    Utils::Environment env = bc->environment();
    Utils::Environment::setupEnglishOutput(&env);
    pp.setEnvironment(env);
    pp.setCommand(ninjaCommand(bc->environment()));
    pp.setArguments(allArguments());
    pp.resolveAll();
    return true;
}

bool NinjaMakeStep::init(QList<const ProjectExplorer::BuildStep *> &earlierSteps)
{
    ProjectExplorer::ProcessParameters *pp = processParameters();
    if(!setupPP(*pp))
        return false;

    setIgnoreReturnValue(m_buildTargets == QStringList{ "clean" });

    setOutputParser(new FixDirectoryParser());
    ProjectExplorer::IOutputParser *parser = target()->kit()->createOutputParser();
    if (parser)
        appendOutputParser(parser);
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

QString NinjaMakeStep::getSummary()
{
    ProjectExplorer::ProcessParameters pp;
    setupPP(pp);
    return pp.summary("ninja");
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
                futureInterface()->setProgressRange(0, all);
                futureInterface()->setProgressValue(done);
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

NinjaMakeAllStepFactory::NinjaMakeAllStepFactory(): BuildStepFactory()
{
    struct Step : NinjaMakeStep
    {
        Step(ProjectExplorer::BuildStepList *bsl) : NinjaMakeStep(bsl, "all") { }
    };

    registerStep<Step>(NINJA_MS_ID);
    setDisplayName("Ninja");
    setSupportedProjectType(MESONPROJECT_ID);
    setSupportedStepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
}

NinjaMakeCleanStepFactory::NinjaMakeCleanStepFactory()
{
    struct Step : NinjaMakeStep
    {
        Step(ProjectExplorer::BuildStepList *bsl) : NinjaMakeStep(bsl, "clean") { }
    };

    registerStep<Step>(NINJA_MS_ID);
    setDisplayName("Ninja");
    setSupportedProjectType(MESONPROJECT_ID);
    setSupportedStepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);
}

}
