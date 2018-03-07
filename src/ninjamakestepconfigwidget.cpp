#include "ninjamakestepconfigwidget.h"

namespace MesonProjectManager
{

NinjaMakeStepConfigWidget::NinjaMakeStepConfigWidget(NinjaMakeStep *makeStep)
{

}

QString NinjaMakeStepConfigWidget::displayName() const
{
    return "ninja";
}

QString NinjaMakeStepConfigWidget::summaryText() const
{
    return "TODO";
}

}
