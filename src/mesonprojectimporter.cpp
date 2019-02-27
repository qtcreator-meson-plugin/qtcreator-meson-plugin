#include "mesonprojectimporter.h"

#include <QStringList>
#include <QTextStream>

#include <memory>
#include "constants.h"

class MesonProjectImporterData
{
public:
    Utils::FileName directory;
    QString mesonPath;
};

MesonProjectManager::MesonProjectImporter::MesonProjectImporter(const Utils::FileName &path) :
    ProjectExplorer::ProjectImporter(path)
{
}

QStringList MesonProjectManager::MesonProjectImporter::importCandidates()
{
    return {};
}

QList<void *> MesonProjectManager::MesonProjectImporter::examineDirectory(const Utils::FileName &importPath) const
{
    QList<void *> result;
    auto ninjaFile = Utils::FileName(importPath).appendPath("build.ninja");
    if (!ninjaFile.exists())
        return result;

    if (!Utils::FileName(importPath).appendPath("meson-private").exists())
        return result;

    auto data = std::make_unique<MesonProjectImporterData>();

    QFile file(ninjaFile.toString());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return result;

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

bool MesonProjectManager::MesonProjectImporter::matchKit(void *data, const ProjectExplorer::Kit *k) const
{
    return ProjectExplorer::KitManager::instance()->defaultKit() == k;
}

ProjectExplorer::Kit *MesonProjectManager::MesonProjectImporter::createKit(void *directoryData) const
{
    // TODO
    Q_UNREACHABLE();
}

const QList<ProjectExplorer::BuildInfo> MesonProjectManager::MesonProjectImporter::buildInfoListForKit(const ProjectExplorer::Kit *k, void *directoryData) const
{
    auto data = static_cast<MesonProjectImporterData *>(directoryData);
    QList<ProjectExplorer::BuildInfo> result;
    // TODO
    auto factory = new MesonBuildConfigurationFactory();
    // create info:


    ProjectExplorer::BuildInfo info(factory);
    info.buildType = ProjectExplorer::BuildConfiguration::Unknown;
    info.displayName = data->directory.fileName(1);
    info.kitId = k->id();
    info.buildDirectory = data->directory;
    QVariantMap extraParams;
    extraParams.insert(MESON_BI_MESON_PATH, data->mesonPath);
    info.extraInfo = extraParams;

    result << info;

    return result;
}

void MesonProjectManager::MesonProjectImporter::deleteDirectoryData(void *data) const
{
    delete static_cast<MesonProjectImporterData *>(data);
}
