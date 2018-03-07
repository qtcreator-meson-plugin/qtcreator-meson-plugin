#pragma once

#include "ninjamakestep.h"

namespace MesonProjectManager {

class NinjaMakeStepConfigWidget : public ProjectExplorer::BuildStepConfigWidget
{
    Q_OBJECT

public:
    explicit NinjaMakeStepConfigWidget(NinjaMakeStep *makeStep);

    QString displayName() const override;
    QString summaryText() const override;
};

}
