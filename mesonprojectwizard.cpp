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

#include "mesonprojectwizard.h"

#include <coreplugin/icore.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/customwizard/customwizard.h>

#include <utils/algorithm.h>
#include <utils/fileutils.h>
#include <utils/filewizardpage.h>
#include <utils/mimetypes/mimedatabase.h>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QVBoxLayout>

#include "constants.h"

namespace MesonProjectManager {

FilesSelectionWizardPage::FilesSelectionWizardPage(MesonProjectWizardDialog *mesonProjectWizard,
                                                   QWidget *parent) :
    QWizardPage(parent),
    m_mesonProjectWizardDialog(mesonProjectWizard),
    m_filesWidget(new ProjectExplorer::SelectableFilesWidget(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(m_filesWidget);
    m_filesWidget->setBaseDirEditable(false);
    connect(m_filesWidget, &ProjectExplorer::SelectableFilesWidget::selectedFilesChanged,
            this, &FilesSelectionWizardPage::completeChanged);

    setProperty(Utils::SHORT_TITLE_PROPERTY, tr("Files"));
}

void FilesSelectionWizardPage::initializePage()
{
    m_filesWidget->resetModel(Utils::FileName::fromString(m_mesonProjectWizardDialog->path()),
                              Utils::FileNameList());
}

void FilesSelectionWizardPage::cleanupPage()
{
    m_filesWidget->cancelParsing();
}

bool FilesSelectionWizardPage::isComplete() const
{
    return m_filesWidget->hasFilesSelected();
}

Utils::FileNameList FilesSelectionWizardPage::selectedPaths() const
{
    return m_filesWidget->selectedPaths();
}

Utils::FileNameList FilesSelectionWizardPage::selectedFiles() const
{
    return m_filesWidget->selectedFiles();
}


//////////////////////////////////////////////////////////////////////////////
//
// MesonProjectWizardDialog
//
//////////////////////////////////////////////////////////////////////////////

MesonProjectWizardDialog::MesonProjectWizardDialog(const Core::BaseFileWizardFactory *factory,
                                                       QWidget *parent) :
    Core::BaseFileWizard(factory, QVariantMap(), parent)
{
    setWindowTitle(tr("Import Existing Project"));

    // first page
    m_firstPage = new Utils::FileWizardPage;
    m_firstPage->setTitle(tr("Project Name and Location"));
    m_firstPage->setFileNameLabel(tr("Project name:"));
    m_firstPage->setPathLabel(tr("Location:"));
    addPage(m_firstPage);

    // second page
    //m_secondPage = new FilesSelectionWizardPage(this);
    //m_secondPage->setTitle(tr("File Selection"));
    //addPage(m_secondPage);
}

QString MesonProjectWizardDialog::path() const
{
    return m_firstPage->path();
}

/*Utils::FileNameList MesonProjectWizardDialog::selectedPaths() const
{
    return m_secondPage->selectedPaths();
}

Utils::FileNameList MesonProjectWizardDialog::selectedFiles() const
{
    return m_secondPage->selectedFiles();
}*/

void MesonProjectWizardDialog::setPath(const QString &path)
{
    m_firstPage->setPath(path);
}

QString MesonProjectWizardDialog::projectName() const
{
    return m_firstPage->fileName();
}

//////////////////////////////////////////////////////////////////////////////
//
// MesonProjectWizard
//
//////////////////////////////////////////////////////////////////////////////

MesonProjectWizard::MesonProjectWizard()
{
    setSupportedProjectTypes({MesonProjectManager::PROJECT_ID});
    // TODO do something about the ugliness of standard icons in sizes different than 16, 32, 64, 128
    {
        QPixmap icon(22, 22);
        icon.fill(Qt::transparent);
        QPainter p(&icon);
        p.drawPixmap(3, 3, 16, 16, qApp->style()->standardIcon(QStyle::SP_DirIcon).pixmap(16));
        setIcon(icon);
    }
    setDisplayName(tr("Meosn Foo"));
    setId("Z.Meson");
    setDescription(tr("Imports existing projects that do not use qmake, CMake or Autotools. "
                      "This allows you to use Qt Creator as a code editor."));
    setCategory(QLatin1String(ProjectExplorer::Constants::QT_PROJECT_WIZARD_CATEGORY));
    setDisplayCategory(QLatin1String(ProjectExplorer::Constants::QT_PROJECT_WIZARD_CATEGORY_DISPLAY));
    setFlags(Core::IWizardFactory::PlatformIndependent);
}

Core::BaseFileWizard *MesonProjectWizard::create(QWidget *parent,
                                                   const Core::WizardDialogParameters &parameters) const
{
    MesonProjectWizardDialog *wizard = new MesonProjectWizardDialog(this, parent);

    wizard->setPath(parameters.defaultPath());

    foreach (QWizardPage *p, wizard->extensionPages())
        wizard->addPage(p);

    return wizard;
}

Core::GeneratedFiles MesonProjectWizard::generateFiles(const QWizard *w,
                                                         QString *errorMessage) const
{
    Q_UNUSED(errorMessage)

    const MesonProjectWizardDialog *wizard = qobject_cast<const MesonProjectWizardDialog *>(w);
    const QString projectPath = wizard->path();
    const QDir dir(projectPath);
    const QString projectName = wizard->projectName();
    const QString creatorFileName = QFileInfo(dir, QLatin1String("meson.build")).absoluteFilePath();
    const QString filesFileName = QFileInfo(dir, projectName + QLatin1String(".files")).absoluteFilePath();
    const QString includesFileName = QFileInfo(dir, projectName + QLatin1String(".includes")).absoluteFilePath();
    const QString configFileName = QFileInfo(dir, projectName + QLatin1String(".config")).absoluteFilePath();
    //const QStringList paths = Utils::transform(wizard->selectedPaths(), &Utils::FileName::toString);

    Utils::MimeType headerTy = Utils::mimeTypeForName(QLatin1String("text/x-chdr"));

    QStringList nameFilters = headerTy.globPatterns();

    QStringList includePaths;
    /*foreach (const QString &path, paths) {
        QFileInfo fileInfo(path);
        QDir thisDir(fileInfo.absoluteFilePath());

        if (! thisDir.entryList(nameFilters, QDir::Files).isEmpty()) {
            QString relative = dir.relativeFilePath(path);
            if (relative.isEmpty())
                relative = QLatin1Char('.');
            includePaths.append(relative);
        }
    }*/
    includePaths.append(QString()); // ensure newline at EOF

    Core::GeneratedFile generatedCreatorFile(creatorFileName);
    generatedCreatorFile.setContents(QLatin1String("[General]\n"));
    generatedCreatorFile.setAttributes(Core::GeneratedFile::OpenProjectAttribute);

    /*QStringList sources = Utils::transform(wizard->selectedFiles(), &Utils::FileName::toString);
    for (int i = 0; i < sources.length(); ++i)
        sources[i] = dir.relativeFilePath(sources[i]);
    Utils::sort(sources);
    sources.append(QString()); // ensure newline at EOF

    Core::GeneratedFile generatedFilesFile(filesFileName);
    generatedFilesFile.setContents(sources.join(QLatin1Char('\n')));*/

    /*Core::GeneratedFile generatedIncludesFile(includesFileName);
    generatedIncludesFile.setContents(includePaths.join(QLatin1Char('\n')));

    Core::GeneratedFile generatedConfigFile(configFileName);
    generatedConfigFile.setContents(QLatin1String(ConfigFileTemplate));
*/
    Core::GeneratedFiles files;
/*    files.append(generatedFilesFile);
    files.append(generatedIncludesFile);
    files.append(generatedConfigFile);*/
    files.append(generatedCreatorFile);

    return files;
}

bool MesonProjectWizard::postGenerateFiles(const QWizard *w, const Core::GeneratedFiles &l,
                                             QString *errorMessage) const
{
    Q_UNUSED(w);
    return ProjectExplorer::CustomProjectWizard::postGenerateOpen(l, errorMessage);
}

} // namespace xxxMeson
