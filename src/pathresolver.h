#pragma once

#include <QMap>
#if defined(Q_OS_UNIX)
#include <sys/stat.h>
#endif

namespace MesonProjectManager
{

class PathResolver
{
public:
    PathResolver();
#if defined(Q_OS_UNIX)
    struct DirectoryCacheInfo
    {
        dev_t device;
        ino_t inode;
        bool operator==(const DirectoryCacheInfo &o)
        {
            return device==o.device && inode==o.inode;
        }
    };
#endif

    struct DirectoryInfo
    {
        QString intendedPath;
#if defined(Q_OS_UNIX)
        DirectoryCacheInfo id;
#endif
    };

    DirectoryInfo getForPath(QString intendedPath);
    QString getIntendedFileName(const DirectoryInfo &base, QString filePath) const;

private:
#if defined(Q_OS_UNIX)
    mutable QMap<QString, DirectoryCacheInfo> cache;

    DirectoryCacheInfo getCachedInfo(const QString &path, bool &ok) const;
#endif
};

}
