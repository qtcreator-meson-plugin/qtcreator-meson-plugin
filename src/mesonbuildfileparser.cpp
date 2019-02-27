#include "mesonbuildfileparser.h"

#include <QFile>
#include <QByteArray>
#include <QFileInfo>
#include <QRegularExpression>

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

    const QRegularExpression interpreted_line(R"(^\s*'([^']+)',?\s*$)");

    while (!file.atEnd()) {
        read_line();
        if (trimmed_line.startsWith("#ide:editable-filelist")) {
            chunks.append({ChunkType::other, line, QString(), {}, {}});
            if (file.atEnd())
                break;
            read_line();

            QStringList files;
            QString section_name = trimmed_line.section('=', 0, 0).simplified();
            QString uninterpreted;
            QMap<QString, QString> uninterpreted_lines;

            while (true) {
                read_line();
                if (file.atEnd())
                    break;
                if (trimmed_line.startsWith("]"))
                    break;

                const auto match = interpreted_line.match(trimmed_line);
                if (match.hasMatch()) {
                    const QString fn = match.captured(1);
                    files.append(fn);
                    if (!uninterpreted.isEmpty()) {
                        uninterpreted_lines.insert(fn, uninterpreted);
                        uninterpreted.clear();
                    }
                }
                else
                {
                    uninterpreted.append(line);
                }
            }

            if (!uninterpreted.isEmpty())
                uninterpreted_lines.insert("", uninterpreted);
            chunks.append({ChunkType::file_list, QString(), section_name, files, uninterpreted_lines});
        } else {
            chunks.append({ChunkType::other, line, QString(), {}, {}});
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
            auto uninterpreted_it = chunk.uninterpreted_lines.cbegin();
            const auto uninterpreted_it_end = chunk.uninterpreted_lines.cend();
            QString trailing_uninterpreted;
            if (uninterpreted_it!=uninterpreted_it_end && uninterpreted_it.key()=="") {
                trailing_uninterpreted = uninterpreted_it.value();
                uninterpreted_it++;
            }
            for (const auto& file: chunk.file_list) {
                while (uninterpreted_it!=uninterpreted_it_end && uninterpreted_it.key()<=file) {
                    output.append(uninterpreted_it.value().toUtf8());
                    uninterpreted_it++;
                }
                output.append(QString("  '"+file+"',\n").toUtf8());
            }
            while (uninterpreted_it!=uninterpreted_it_end) {
                output.append(uninterpreted_it.value().toUtf8());
                uninterpreted_it++;
            }
            output.append(trailing_uninterpreted.toUtf8());
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
