#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include <QTemporaryDir>
#include <QStringList>
#include <QDebug>

#include "src/mesonbuildfileparser.h"

using namespace MesonProjectManager;

const QString testfile_data_no_action=R"(
project('rule_system', 'cpp')

add_global_arguments('-std=c++14', language : 'cpp')

dep_thread = dependency('threads')
dep_dl = meson.get_compiler('cpp').find_library('dl', required: true)
#dep_mosquitto = meson.get_compiler('cpp').find_library('mosquitto', required: true)
dep_mosquittopp = meson.get_compiler('cpp').find_library('mosquittopp', required: true)

app_sources = ['main.cpp', 'chai_wrapper.cpp', 'rule_logic.cpp', 'things.cpp', 'restricted_chaiscript.cpp', 'named_logger.cpp']
app_includes = include_directories('ChaiScript-6.0.0/include')
app_depends = [dep_thread, dep_dl, dep_mosquittopp]
executable('rule_system',
           app_sources,
           include_directories: app_includes,
           dependencies: app_depends)

)";

const QString testfile_data_single_file_list=R"(
project('rule_system', 'cpp')

add_global_arguments('-std=c++14', language : 'cpp')

dep_thread = dependency('threads')
dep_dl = meson.get_compiler('cpp').find_library('dl', required: true)
#dep_mosquitto = meson.get_compiler('cpp').find_library('mosquitto', required: true)
dep_mosquittopp = meson.get_compiler('cpp').find_library('mosquittopp', required: true)

#ide:editable-filelist
app_sources = [
  'chai_wrapper.cpp',
  'main.cpp',
  'named_logger.cpp',
  'restricted_chaiscript.cpp',
  'rule_logic.cpp',
  'things.cpp',
]

app_includes = include_directories('ChaiScript-6.0.0/include')
app_depends = [dep_thread, dep_dl, dep_mosquittopp]
executable('rule_system',
           app_sources,
           include_directories: app_includes,
           dependencies: app_depends)

)";

const QString testfile_data_multiple_file_lists=R"(
project('rule_system', 'cpp')

add_global_arguments('-std=c++14', language : 'cpp')

dep_thread = dependency('threads')
dep_dl = meson.get_compiler('cpp').find_library('dl', required: true)
#dep_mosquitto = meson.get_compiler('cpp').find_library('mosquitto', required: true)
dep_mosquittopp = meson.get_compiler('cpp').find_library('mosquittopp', required: true)

#ide:editable-filelist
app_sources = [
  'chai_wrapper.cpp',
  'main.cpp',
  'named_logger.cpp',
  'restricted_chaiscript.cpp',
  'rule_logic.cpp',
  'things.cpp',
]

#ide:editable-filelist
other_sources = [
  'bar.cpp',
  'baz.cpp',
  'foo.cpp',
]

app_includes = include_directories('ChaiScript-6.0.0/include')
app_depends = [dep_thread, dep_dl, dep_mosquittopp]
executable('rule_system',
           app_sources,
           include_directories: app_includes,
           dependencies: app_depends)

)";

void write_file(const QString &dir, const QString &content)
{
    QFile testdata(dir+"/meson.build");
    testdata.open(QIODevice::ReadWrite);
    REQUIRE(testdata.isOpen());
    testdata.write(content.toUtf8());
    testdata.close();
}

TEST_CASE("Parser should work")
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());

    SECTION("No Action")
    {
        write_file(dir.path(), testfile_data_no_action);
        MesonBuildFileParser p(dir.path()+"/meson.build");
        p.parse();
        REQUIRE(p.regenerate().toStdString()==testfile_data_no_action.toStdString());
        REQUIRE(p.fileListNames()==QStringList{});
    }

    SECTION("Single file list")
    {
        write_file(dir.path(), testfile_data_single_file_list);
        MesonBuildFileParser p(dir.path()+"/meson.build");
        p.parse();
        REQUIRE(p.regenerate().toStdString()==testfile_data_single_file_list.toStdString());
        REQUIRE(p.fileListNames()==QStringList{"app_sources"});
        REQUIRE(p.fileList("app_sources").file_list==QStringList({"chai_wrapper.cpp",
                                                       "main.cpp",
                                                       "named_logger.cpp",
                                                       "restricted_chaiscript.cpp",
                                                       "rule_logic.cpp",
                                                       "things.cpp",}));
    }

    SECTION("Single file list")
    {
        write_file(dir.path(), testfile_data_multiple_file_lists);
        MesonBuildFileParser p(dir.path()+"/meson.build");
        p.parse();
        REQUIRE(p.regenerate().toStdString()==testfile_data_multiple_file_lists.toStdString());
        REQUIRE(p.fileListNames()==QStringList({"app_sources", "other_sources"}));
        REQUIRE(p.fileList("app_sources").file_list==QStringList({"chai_wrapper.cpp",
                                                       "main.cpp",
                                                       "named_logger.cpp",
                                                       "restricted_chaiscript.cpp",
                                                       "rule_logic.cpp",
                                                       "things.cpp",}));
        REQUIRE(p.fileList("other_sources").file_list==QStringList({"bar.cpp",
                                                       "baz.cpp",
                                                       "foo.cpp",}));
    }
}
