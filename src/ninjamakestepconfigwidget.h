#pragma once

#include "ninjamakestep.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>

namespace MesonProjectManager {

class NinjaMakeStepConfigWidget : public ProjectExplorer::BuildStepConfigWidget
{
    Q_OBJECT

public:
    explicit NinjaMakeStepConfigWidget(NinjaMakeStep *makeStep);
    QString displayName() const;
    QString summaryText() const;
private:
    NinjaMakeStep *step;
};

}
