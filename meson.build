project(
  'washroomRAT',
  'cpp',
  default_options: [
    'buildtype=debug',
    'default_library=static',
    'cpp_std=c++20',
  ],
  license: 'Apache-2.0',
  meson_version: '>= 1.0.0',
  version: '0.0.1-alpha',
)

# preprocessor
add_project_arguments('-DVERSION="@0@"'.format(meson.project_version()), language: 'cpp')

# subprojects
gtest_dep = subproject('gtest', required: false).get_variable('gtest_main_dep')

# dependencies
readline_dep = dependency('readline')

# include directories
inc_dir = include_directories(['src/client', 'src/common', 'src/server'])

# sub-directories
# subdir('src/client')
subdir('src/common')
subdir('src/server')

# server
executable(
  'washroom',
  'src/server/main.cc',
  include_directories: inc_dir,
  dependencies: [readline_dep],
  link_with: [listener_static],
)

# client
executable(
  'client',
  'src/client/main.cpp',
  include_directories: inc_dir,
)

# test
crypto = executable(
  'crypto_test',
  'tests/test_crypto.cc',
  build_by_default: false,
  dependencies: [gtest_dep],
  include_directories: inc_dir,
  link_with: [crypto_static],
)

test('crypto_test', crypto)
