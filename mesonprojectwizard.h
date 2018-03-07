/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

#include <coreplugin/basefilewizard.h>
#include <coreplugin/basefilewizardfactory.h>
#include <projectexplorer/selectablefilesmodel.h>
#include <utils/fileutils.h>
#include <utils/wizard.h>

namespace Utils { class FileWizardPage; }

namespace MesonProjectManager {

class MesonProjectWizardDialog;

class FilesSelectionWizardPage : public QWizardPage
{
    Q_OBJECT

public:
    FilesSelectionWizardPage(MesonProjectWizardDialog *mesonProjectWizard, QWidget *parent = nullptr);
    bool isComplete() const override;
    void initializePage() override;
    void cleanupPage() override;
    Utils::FileNameList selectedFiles() const;
    Utils::FileNameList selectedPaths() const;

private:
    MesonProjectWizardDialog *m_mesonProjectWizardDialog;
    ProjectExplorer::SelectableFilesWidget *m_filesWidget;
};

class MesonProjectWizardDialog : public Core::BaseFileWizard
{
    Q_OBJECT

public:
    explicit MesonProjectWizardDialog(const Core::BaseFileWizardFactory *factory, QWidget *parent = 0);

    QString path() const;
    void setPath(const QString &path);
    //Utils::FileNameList selectedFiles() const;
    //Utils::FileNameList selectedPaths() const;

    QString projectName() const;

    Utils::FileWizardPage *m_firstPage;
    //FilesSelectionWizardPage *m_secondPage;
};

class MesonProjectWizard : public Core::BaseFileWizardFactory
{
    Q_OBJECT

public:
    MesonProjectWizard();

protected:
    Core::BaseFileWizard *create(QWidget *parent, const Core::WizardDialogParameters &parameters) const override;
    Core::GeneratedFiles generateFiles(const QWizard *w, QString *errorMessage) const override;
    bool postGenerateFiles(const QWizard *w, const Core::GeneratedFiles &l,
                           QString *errorMessage) const override;
};


} // namespace xxxMeson
