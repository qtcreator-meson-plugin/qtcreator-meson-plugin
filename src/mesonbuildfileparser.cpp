#include "mesonbuildfileparser.h"

#include <QFile>
#include <QByteArray>
#include <QFileInfo>

namespace MesonProjectManager {

MesonBuildFileParser::MesonBuildFileParser(const QString &filename): filename(filename)
{
    project_base=QFileInfo(filename).absolutePath();
}

void MesonBuildFileParser::parse()
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
        return;

    QString line;
    QString trimmed_line;
    auto read_line = [&]() {
        line=QString::fromUtf8(file.readLine());
        trimmed_line=line.trimmed();
    };

    while (!file.atEnd()) {
        read_line();
        if (trimmed_line.startsWith("#ide:editable-filelist")) {
            chunks.append({ChunkType::other, line, "", {}});
            if (file.atEnd())
                break;
            read_line();

            QStringList files;
            QString section_name = trimmed_line.section('=', 0, 0).simplified();

            while (true) {
                read_line();
                if (file.atEnd())
                    break;
                if (trimmed_line.startsWith("]"))
                    break;

                files.append(trimmed_line.section("'", 1, 1));
            }

            chunks.append({ChunkType::file_list, "", section_name, files});
        } else {
            chunks.append({ChunkType::other, line, "", {}});
        }
    }

    for (auto &chunk: chunks) {
        if (chunk.type!=ChunkType::file_list)
            continue;
        file_lists.insert(chunk.file_list_name, &chunk);
    }
}

QByteArray MesonBuildFileParser::regenerate()
{
    QByteArray output;
    for (auto& chunk: chunks)
    {
        if (chunk.type==ChunkType::other) {
            output.append(chunk.line.toUtf8());
        } else if (chunk.type==ChunkType::file_list) {
            output.append(QString(chunk.file_list_name+" = [\n").toUtf8());
            chunk.file_list.sort();
            for (const auto& file: chunk.file_list)
                output.append(QString("    '"+file+"',\n").toUtf8());
            output.append("]\n");
        }
    }
    return output;
}

MesonBuildFileParser::ChunkInfo &MesonBuildFileParser::fileList(const QString &name)
{
    return *file_lists.value(name);
}

QStringList MesonBuildFileParser::fileListAbsolute(const QString &name)
{
    QStringList result;
    for (const auto &filename: fileList(name).file_list)
        result.append(project_base+"/"+filename);
    return result;
}

const MesonBuildFileParser::ChunkInfo &MesonBuildFileParser::fileList(const QString &name) const
{
    return *file_lists.value(name);
}

bool MesonBuildFileParser::hasFileList(const QString &name) const
{
    return file_lists.contains(name);
}

QStringList MesonBuildFileParser::fileListNames() const
{
    return file_lists.keys();
}

QString MesonBuildFileParser::getProject_base() const
{
    return project_base;
}

}
