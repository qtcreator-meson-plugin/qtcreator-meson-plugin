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

#include "constants.h"
#include "mesonbuildconfiguration.h"
#include "mesonbuildinfo.h"
#include "mesonproject.h"

#include <coreplugin/icore.h>

#include <projectexplorer/customwizard/customwizard.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>
#include <projectexplorer/task.h>
#include <projectexplorer/taskhub.h>

#include <utils/algorithm.h>
#include <utils/environment.h>
#include <utils/fileutils.h>
#include <utils/filewizardpage.h>
#include <utils/mimetypes/mimedatabase.h>
#include <utils/synchronousprocess.h>

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QStyle>
#include <QVBoxLayout>

namespace MesonProjectManager {

MesonProjectWizardDialog::MesonProjectWizardDialog(const Core::BaseFileWizardFactory *factory, QWidget *parent) :
    Core::BaseFileWizard(factory, QVariantMap(), parent)
{
    setWindowTitle(tr("Create New Meson C++ Application"));

    // first page
    m_firstPage = new Utils::FileWizardPage;
    m_firstPage->setTitle(tr("Project Name and Location"));
    m_firstPage->setFileNameLabel(tr("Project name:"));
    m_firstPage->setPathLabel(tr("Location:"));
    addPage(m_firstPage);
}

QString MesonProjectWizardDialog::path() const
{
    return m_firstPage->path();
}

void MesonProjectWizardDialog::setPath(const QString &path)
{
    m_firstPage->setPath(path);
}

QString MesonProjectWizardDialog::projectName() const
{
    return m_firstPage->fileName();
}

bool MesonProjectWizardDialog::validateCurrentPage()
{
    if (!Core::BaseFileWizard::validateCurrentPage())
        return false;
    if (m_firstPage->fileName().startsWith("meson-")) {
        QMessageBox::warning(this, tr("Invalid project name"),
                             tr("Names of Meson targets may not start with \"meson-\" as they are reserved for internal use."));
        return false;
    }
    return true;
}

MesonProjectWizard::MesonProjectWizard()
{
    setCategory(QLatin1String(ProjectExplorer::Constants::QT_PROJECT_WIZARD_CATEGORY));
    setDescription(tr("Creates a new project for use with the meson build system."));
    setDisplayCategory(QLatin1String(ProjectExplorer::Constants::QT_PROJECT_WIZARD_CATEGORY_DISPLAY));
    setDisplayName(tr("Meson C++ Application"));
    setFlags(Core::IWizardFactory::PlatformIndependent);
    setIcon(QPixmap(":/mesonprojectmanager/images/meson_logo.png"));
    setId("Z.Meson");
    setSupportedProjectTypes({MesonProjectManager::PROJECT_ID});
}

Core::BaseFileWizard *MesonProjectWizard::create(QWidget *parent, const Core::WizardDialogParameters &parameters) const
{
    MesonProjectWizardDialog *wizard = new MesonProjectWizardDialog(this, parent);

    wizard->setPath(parameters.defaultPath());

    foreach (QWizardPage *p, wizard->extensionPages())
        wizard->addPage(p);

    return wizard;
}

Core::GeneratedFiles MesonProjectWizard::generateFiles(const QWizard *w, QString *errorMessage) const
{
    Q_UNUSED(errorMessage)

    const MesonProjectWizardDialog *wizard = qobject_cast<const MesonProjectWizardDialog *>(w);
    const QString projectPath = wizard->path();
    const QString projectName = wizard->projectName();
    QString safeProjectName = projectName;
    safeProjectName.replace(' ', '_');
    const QDir dir(projectPath+"/"+safeProjectName);
    const QString mesonBuildFileName = QFileInfo(dir, QStringLiteral("meson.build")).absoluteFilePath();
    const QString cppFileName = QFileInfo(dir, QStringLiteral("%1.cpp").arg(safeProjectName)).absoluteFilePath();

    Core::GeneratedFile mesonBuildFile(mesonBuildFileName);
    const QString projectFileTemplate = QStringLiteral(R"(project('%1', 'cpp',
  version : '1.0',
  default_options : ['warning_level=3',
                     'cpp_std=c++14'])

#ide:editable-filelist
sources = [
  '%2.cpp',
]

exe = executable('%2', sources, install : true)

test('basic', exe))");
    mesonBuildFile.setContents(projectFileTemplate.arg(projectName, safeProjectName));
    mesonBuildFile.setAttributes(Core::GeneratedFile::OpenProjectAttribute);

    Core::GeneratedFile cppFile(cppFileName);
    cppFile.setContents(R"(#include <iostream>

int main(int argc, char *argv[]) {
    std::cout<<"Hello World!"<<std::endl;
    return 0;
})");

    Core::GeneratedFiles files;
    files.append(mesonBuildFile);
    files.append(cppFile);

    return files;
}

bool MesonProjectWizard::postGenerateFiles(const QWizard *w, const Core::GeneratedFiles &l, QString *errorMessage) const
{
    Q_UNUSED(w);

    const Utils::FileName mesonBin = MesonProject::findDefaultMesonExecutable();
    if(mesonBin.isEmpty())
        return false;

    const MesonProjectWizardDialog *wizard = qobject_cast<const MesonProjectWizardDialog *>(w);
    const QString projectPath = wizard->path();
    const QString projectName = wizard->projectName();
    QString safeProjectName = projectName;
    safeProjectName.replace(' ', '_');
    const QDir dir(projectPath+"/"+safeProjectName);

    Utils::SynchronousProcess proc;
    proc.setWorkingDirectory(dir.absolutePath());
    auto response = proc.run(mesonBin.toString(), {"_build"});
    if (response.exitCode!=0) {
        ProjectExplorer::TaskHub::addTask(ProjectExplorer::Task::Error,
                                          QStringLiteral("Can't create build directory. rc=%1").arg(QString::number(response.exitCode)),
                                          ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM);
        return false;
    }

    {
        MesonProject proj(Utils::FileName::fromString(l.first().path()));
        ProjectExplorer::Kit *kit = ProjectExplorer::KitManager::instance()->defaultKit();
        ProjectExplorer::Target *target = proj.createTarget(kit);
        MesonBuildConfigurationFactory factory;

        MesonBuildInfo buildInfo(&factory);
        buildInfo.buildDirectory = Utils::FileName::fromString(dir.absoluteFilePath("_build"));
        buildInfo.displayName = "Default";
        buildInfo.kitId = kit->id();
        buildInfo.typeName = buildInfo.displayName;
        buildInfo.mesonPath = mesonBin.toString();

        ProjectExplorer::BuildConfiguration *cfg = factory.create(target, &buildInfo);
        target->addBuildConfiguration(cfg);

        proj.addTarget(target);
        proj.saveSettings();
    }

    return ProjectExplorer::CustomProjectWizard::postGenerateOpen(l, errorMessage);
}

}
