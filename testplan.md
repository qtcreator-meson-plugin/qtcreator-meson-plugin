# What to test?

* Import Project
  * with existing build dir
  * without exisiting build dir
* Import existing build for project already loaded into the workspace
* Import existing build for project when importing it into the workspace
* Project still works after restarting Qt Creator
* Create new project using new project wizard
* Check the following for imported project:
  * All specified file lists show up with correct names
  * project name in meson.build is show correctly if existing
  * otherwise project name should be the name of the folder containing the meson.build
  * all subprojects are listed in the "Subprojects" virtual folder node

* File lists (for all test check that item ordering is correct and no comments are lost)
  * Add single file
  * Add multiple files
  * Remove single file
  * lists from subprojects show up correctly
  * files not included in any marked file list show up under "Extra Files" in the correct subfolders if applicable

