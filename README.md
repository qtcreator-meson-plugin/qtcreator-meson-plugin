# About
This is some basic Meson buildsystem integration for Qt Creator.

Currently implemented features:
- Create new projects
- Show files in project:
  - Explicitly marked file lists
  - Other files
  - Subdirectories
  - Subprojects
  - Automatially shows coresponding header files (.h, .hpp, .hh, _p variants)
- Add/Remvoe/Rename files in explicitly marked file lists
- Basic build integration (ninja backend only):
- Extract C++ code completion information from compile_commands.json

Not yet implemented:
- Group files by target
- Edit build directory configuration
- Automatically detect (or create) build directory for existing projects
- Kit support
- meson.build syntax highlighting/code completion
- Asychronous meson introspection / project parsing

# How to create editable file groups
To enable editing a file list directly from the project explorer it needs to be marked with a marker comment.
It looks like this: `#ide:editable-filelist`.
Usage example:
```
#ide:editable-filelist
sources = [
  'bar.cpp',
  'foo.cpp',
]
```

# Building from source

To build the plugin you need the qtcreator sources and libraries. So the safest way is to also build qtcreator from source.

Rough steps:

```
# Optional:
#export LLVM_INSTALL_DIR=/path/to/llvm
# useful on debian's stock qt
export QT_SELECT=5

wget http://download.qt.io/official_releases/qtcreator/4.5/4.5.2/qt-creator-opensource-src-4.5.2.tar.xz
tar -xf qt-creator-opensource-src-4.5.2.tar.xz
mkdir qt-creator-opensource-src-4.5.2-build
cd qt-creator-opensource-src-4.5.2-build

qmake -r ../qt-creator-opensource-src-4.5.2
make -j 6
IDE_SOURCE_TREE=$(realpath ../qt-creator-opensource-src-4.5.2)
IDE_BUILD_TREE=$(realpath .)

cd /path/to/qtcreator-meson-plugin
qmake mesonprojectmanager.pro IDE_SOURCE_TREE="$IDE_SOURCE_TREE" IDE_BUILD_TREE="$IDE_BUILD_TREE"
make
```
