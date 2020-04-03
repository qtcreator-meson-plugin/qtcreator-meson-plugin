#include "fixdirectoryparser.h"

#include <projectexplorer/task.h>

#include <utils/fileutils.h>

namespace MesonProjectManager {

FixDirectoryParser::FixDirectoryParser()
{
    setObjectName(QLatin1String("FixDirectoryParser"));
}

void FixDirectoryParser::setWorkingDirectory(const QString &workingDirectory)
{
    m_workingDirectory = QDir(workingDirectory);
    IOutputParser::setWorkingDirectory(workingDirectory);
}

void FixDirectoryParser::taskAdded(const ProjectExplorer::Task &task, int linkedLines, int skipLines)
{
    ProjectExplorer::Task editable(task);

    QString filePath = task.file.toString();

    if (!filePath.isEmpty())
        editable.file = Utils::FilePath::fromUserInput(m_workingDirectory.absoluteFilePath(filePath));

    IOutputParser::taskAdded(editable, linkedLines, skipLines);
}

} // namespace MesonProjectManager
