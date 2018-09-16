#include "pathresolver.h"

#include <utils/fileutils.h>

namespace MesonProjectManager {

PathResolver::PathResolver()
{

}

PathResolver::DirectoryInfo PathResolver::getForPath(QString intendedPath)
{
    DirectoryInfo info;
    if(!intendedPath.endsWith("/"))
        intendedPath.append("/");
    info.intendedPath = intendedPath;
#if defined(Q_OS_UNIX)
    bool ok = false;
    info.id = getCachedInfo(intendedPath, ok);
    // Ignore ok==false because parsing meson.build will fail later so it does not matter here.
#endif
    return info;
}

QString PathResolver::getIntendedFileName(const DirectoryInfo &base, QString filePath) const
{
#if defined(Q_OS_UNIX)
    QString parent = filePath;
    while(parent.contains("/")) {
        parent = parent.section('/', 0, -2);
        if(parent.isEmpty())
            break;
        bool ok = false;
        DirectoryCacheInfo parentId = getCachedInfo(parent, ok);
        if(ok) {
            if(parentId==base.id) {
                return base.intendedPath + filePath.mid(parent.length()+1);
            }
        }
    }

#endif
    return filePath;
}

#if defined(Q_OS_UNIX)
PathResolver::DirectoryCacheInfo PathResolver::getCachedInfo(const QString &path, bool &ok) const
{
    if(cache.contains(path)) {
        ok = true;
        return cache.value(path);
    }

    DirectoryCacheInfo info;

    struct stat sb;
    if(stat(path.toUtf8().data(), &sb)==0) {
        info.device = sb.st_dev;
        info.inode = sb.st_ino;
        cache.insert(path, info);
        ok = true;
    } else {
        //int error = errno;
        info.device = -1;
        info.inode = -1;
        ok = false;
    }
    return info;
}
#endif

}
