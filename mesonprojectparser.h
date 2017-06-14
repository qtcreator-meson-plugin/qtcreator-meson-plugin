#pragma once

#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>

namespace xxxMeson {

class MesonBuildParser {
public:
    enum class ChunkType {
        file_list,
        other,
    };

    struct ChunkInfo
    {
        ChunkType type;
        QString line;
        QString file_list_name;
        QStringList file_list;
    };

    const QString filename;
    MesonBuildParser(const QString& filename);
    void parse();
    QByteArray regenerate();

    ChunkInfo& fileList(const QString &name);
    const ChunkInfo& fileList(const QString &name) const;
    bool hasFileList(const QString &name) const;
    QStringList fileListNames() const;

private:
    QList<ChunkInfo> chunks;
    QMap<QString, ChunkInfo*> file_lists;
};

}
