#pragma once

#include <projectexplorer/ioutputparser.h>

#include <QDir>

namespace MesonProjectManager {

class FixDirectoryParser : public ProjectExplorer::IOutputParser
{
    Q_OBJECT

public:
    explicit FixDirectoryParser();

private:
    void setWorkingDirectory(const QString &workingDirectory);
    void taskAdded(const ProjectExplorer::Task &task, int linkedLines, int skipLines);

    QDir m_workingDirectory;
};

} // namespace MesonProjectManager
