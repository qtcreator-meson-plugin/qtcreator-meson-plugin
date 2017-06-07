#pragma once

#include "mesonbuildconfigurationfactory.h"

#include <QStringList>

#include <projectexplorer/projectimporter.h>
#include <memory>
#include <projectexplorer/buildinfo.h>

namespace xxxMeson {

class MesonProjectImporter: public ProjectExplorer::ProjectImporter
{
    Q_OBJECT

    // ProjectImporter interface
public:
    MesonProjectImporter(const Utils::FileName &path):ProjectExplorer::ProjectImporter(path)
    {

    }

    QStringList importCandidates() override
    {
        return {};
    }

protected:
    QList<void *> examineDirectory(const Utils::FileName &importPath) const override
    {
        return {(void*)this};
    }
    bool matchKit(void *directoryData, const ProjectExplorer::Kit *k) const override
    {
        return true;
    }
    ProjectExplorer::Kit *createKit(void *directoryData) const override
    {
        // TODO
        Q_UNREACHABLE();
    }
    QList<ProjectExplorer::BuildInfo *> buildInfoListForKit(const ProjectExplorer::Kit *k, void *directoryData) const override
    {
        QList<ProjectExplorer::BuildInfo *> result;
        // TODO
        auto factory = new MesonBuildConfigurationFactory();
        // create info:


        std::unique_ptr<ProjectExplorer::BuildInfo> info(new ProjectExplorer::BuildInfo(factory));
        //if (data->buildConfig & BaseQtVersion::DebugBuild)
        {
            info->buildType = ProjectExplorer::BuildConfiguration::Debug;
            info->displayName = QCoreApplication::translate("QmakeProjectManager::Internal::QmakeProjectImporter", "Debug");
        } /*else {
            info->buildType = BuildConfiguration::Release;
            info->displayName = QCoreApplication::translate("QmakeProjectManager::Internal::QmakeProjectImporter", "Release");
        }*/
        info->kitId = k->id();
        info->buildDirectory = Utils::FileName::fromString("build");//data->buildDirectory;
        //info->additionalArguments = {};//data->additionalArguments;
        //info->config = data->config;
        //info->makefile = data->makefile;

        result << info.release();

        return result;
    }
    void deleteDirectoryData(void *directoryData) const override
    {

    }
};

}
