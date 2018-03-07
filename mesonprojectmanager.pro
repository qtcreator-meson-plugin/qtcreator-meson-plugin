DEFINES += MESONPROJECTMANAGER_LIBRARY

QT *= network

# MesonProjectManager files

SOURCES += \
    mesonproject.cpp \
    mesonbuildconfigurationfactory.cpp \
    mesonprojectimporter.cpp \
    mesonprojectwizard.cpp \
    src/ninjamakestep.cpp \
    src/ninjamakestepconfigwidget.cpp \
    src/mesonprojectmanagerplugin.cpp \
    src/mesonprojectparser.cpp

HEADERS += \
    mesonproject.h \
    mesonbuildconfigurationfactory.h \
    mesonprojectimporter.h \
    mesonprojectwizard.h \
    src/ninjamakestep.h \
    src/ninjamakestepconfigwidget.h \
    src/constants.h \
    src/mesonprojectmanagerplugin.h \
    src/mesonprojectparser.h

# Qt Creator linking

## Either set the IDE_SOURCE_TREE when running qmake,
## or set the QTC_SOURCE environment variable, to override the default setting
isEmpty(IDE_SOURCE_TREE): IDE_SOURCE_TREE = $$(QTC_SOURCE)
isEmpty(IDE_SOURCE_TREE): IDE_SOURCE_TREE = "/home/trilader/code/qt-creator-opensource-src-4.5.1"

## Either set the IDE_BUILD_TREE when running qmake,
## or set the QTC_BUILD environment variable, to override the default setting
isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE = "/home/trilader/code/build-qtcreator-Desktop-Debug"

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on OS X
USE_USER_DESTDIR = yes

###### If the plugin can be depended upon by other plugins, this code needs to be outsourced to
###### <dirname>_dependencies.pri, where <dirname> is the name of the directory containing the
###### plugin's sources.

QTC_PLUGIN_NAME = MesonProjectManager
QTC_LIB_DEPENDS += \
    # nothing here at this time

QTC_PLUGIN_DEPENDS += \
    coreplugin projectexplorer cpptools

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######

include($$IDE_SOURCE_TREE/src/qtcreatorplugin.pri)
