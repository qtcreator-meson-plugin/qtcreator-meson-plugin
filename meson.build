project('importer-test', 'cpp')

files = [ 'tests.cpp', 'src/mesonbuildfileparser.cpp', ]

qtcore = dependency('qt5', modules: 'Core')


e = executable('tests',
    files,
    include_directories: [include_directories('.')],
    dependencies: [qtcore])

test('It can\'t be broken', e)

message('==============================================================================')
message('This is just a simple unit test. To build the plugin have a look at README.md!')
message('==============================================================================')
