#pragma once

#include "mesonbuildconfigurationfactory.h"

#include "src/mesonbuildinfo.h"

#include <QStringList>
#include <QTextStream>

#include <projectexplorer/kitmanager.h>

#include <projectexplorer/projectimporter.h>
#include <memory>
#include <projectexplorer/buildinfo.h>

namespace MesonProjectManager {

struct MesonProjectImporterData {
    Utils::FileName directory;
    QString mesonPath;
};

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
        QList<void *> result;
        auto ninjaFile = Utils::FileName(importPath).appendPath("build.ninja");
        if (!ninjaFile.exists()) {
            return result;
        }

        if (!Utils::FileName(importPath).appendPath("meson-private").exists()) {
            return result;
        }

        auto data = std::make_unique<MesonProjectImporterData>();

        QFile file(ninjaFile.toString());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return result;
        }

        data->mesonPath = "meson.py";
        data->directory = importPath;

        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line == "rule REGENERATE_BUILD") {
                if (!in.atEnd()) {
                    line = in.readLine().simplified();
                    data->mesonPath = line.section(" ", 3, 3);
                }
            }
        }

        result.append(data.release());
        return result;
    }

    bool matchKit(void *data, const ProjectExplorer::Kit *k) const override
    {
        return ProjectExplorer::KitManager::instance()->defaultKit() == k;
    }
    ProjectExplorer::Kit *createKit(void *directoryData) const override
    {
        // TODO
        Q_UNREACHABLE();
    }
    QList<ProjectExplorer::BuildInfo *> buildInfoListForKit(const ProjectExplorer::Kit *k, void *directoryData) const override
    {
        auto data = static_cast<MesonProjectImporterData *>(directoryData);
        QList<ProjectExplorer::BuildInfo *> result;
        // TODO
        auto factory = new MesonBuildConfigurationFactory();
        // create info:


        std::unique_ptr<MesonBuildInfo> info(new MesonBuildInfo(factory));
        info->buildType = ProjectExplorer::BuildConfiguration::Unknown;
        info->displayName = data->directory.fileName(1);
        info->kitId = k->id();
        info->buildDirectory = data->directory;
        info->mesonPath = data->mesonPath;
        //info->additionalArguments = {};//data->additionalArguments;
        //info->config = data->config;
        //info->makefile = data->makefile;

        result << info.release();

        return result;
    }
    void deleteDirectoryData(void *data) const override
    {
        delete static_cast<MesonProjectImporterData *>(data);
    }
};

}
