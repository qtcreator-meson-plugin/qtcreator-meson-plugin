# About
This is some basic Meson build system integration for Qt Creator.

Currently implemented features:
- Create new projects
- Show files in project:
  - Explicitly marked file lists
  - Other files
  - Subdirectories
  - Subprojects
  - Automatically shows corresponding header files (.h, .hpp, .hh, _p variants)
- Add/Remove/Rename files in explicitly marked file lists
- Basic build integration (ninja backend only)
- Extract C++ code completion information from build directory
- Group files by target
- Edit build directory configuration

Not yet implemented:
- Automatically detect (or create) build directory for existing projects
- Kit support
- meson.build syntax highlighting/code completion
- Asynchronous meson introspection / project parsing

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

Please note that the build was only tested with Qt Creator 4.9.0 and other versions most likely won't work as the Qt Creator APIs tend to change even between minor version.

To build the plugin you need the qtcreator sources and libraries. So the safest way is to also build qtcreator from source.

Rough steps:

```
# Optional:
#export LLVM_INSTALL_DIR=/path/to/llvm
# useful on debian's stock qt
export QT_SELECT=5

wget https://download.qt.io/official_releases/qtcreator/4.9/4.9.0/qt-creator-opensource-src-4.9.0.tar.xz
tar -xf qt-creator-opensource-src-4.9.0.tar.xz
mkdir qt-creator-opensource-src-4.9.0-build
cd qt-creator-opensource-src-4.9.0-build

qmake -r ../qt-creator-opensource-src-4.9.0
make -j 6
IDE_SOURCE_TREE=$(realpath ../qt-creator-opensource-src-4.9.0)
IDE_BUILD_TREE=$(realpath .)

cd /path/to/qtcreator-meson-plugin
qmake mesonprojectmanager.pro IDE_SOURCE_TREE="$IDE_SOURCE_TREE" IDE_BUILD_TREE="$IDE_BUILD_TREE"
make
```

# Snapshots
You can find automatically built snapshots on [Azure Pipelines](https://dev.azure.com/qtcreator-meson-plugin/qtcreator-meson-plugin/_build).
