#include "ninjamakestepconfigwidget.h"

namespace MesonProjectManager
{

NinjaMakeStepConfigWidget::NinjaMakeStepConfigWidget(NinjaMakeStep *makeStep):
    ProjectExplorer::BuildStepConfigWidget{makeStep},step(makeStep)
{
    QFormLayout *layout = new QFormLayout();
    setLayout(layout);

    QLineEdit *lineedit_cmd = new QLineEdit();
    lineedit_cmd->setText(step->m_ninjaCommand);
    layout->addRow(new QLabel(tr("Override Command")), lineedit_cmd);
    connect(lineedit_cmd, &QLineEdit::textChanged, [this, lineedit_cmd] {
        step->m_ninjaCommand = lineedit_cmd->text();
        emit updateSummary();
    });

    QLineEdit *lineedit_args = new QLineEdit();
    lineedit_args->setText(step->m_ninjaArguments);
    layout->addRow(new QLabel(tr("Additional Arguments")), lineedit_args);
    connect(lineedit_args, &QLineEdit::textChanged, [this, lineedit_args] {
        step->m_ninjaArguments = lineedit_args->text();
        emit updateSummary();
    });

    QLineEdit *lineedit_targets = new QLineEdit();
    lineedit_targets->setText(step->m_buildTargets.join(" "));
    layout->addRow(new QLabel(tr("Targets")), lineedit_targets);
    connect(lineedit_targets, &QLineEdit::textChanged, [this, lineedit_targets] {
        step->m_buildTargets = lineedit_targets->text().split(" ", QString::SkipEmptyParts);
        emit updateSummary();
    });
}

QString NinjaMakeStepConfigWidget::displayName() const
{
    return "ninja";
}

QString NinjaMakeStepConfigWidget::summaryText() const
{
    return step->getSummary();
}

}
