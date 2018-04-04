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
