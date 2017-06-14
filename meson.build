project('importer-test', 'cpp')

files = [ 'tests.cpp', 'mesonprojectparser.cpp', ]

qtcore = dependency('qt5', modules: 'Core')


e = executable('tests',
    files,
    include_directories: [include_directories('.')],
    dependencies: [qtcore])

test('It can\'t be broken', e)
