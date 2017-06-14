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

