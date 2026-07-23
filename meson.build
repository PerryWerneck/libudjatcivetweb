# Maintainer: Perry Werneck <perry.werneck@gmail.com>

# SPDX-License-Identifier: LGPL-3.0-or-later 

# Copyright (C) 2024 Perry Werneck <perry.werneck@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

project(
	'udjathttpd', 
	['cpp'],
	version: '2.0.0',
	default_options : ['c_std=c11', 'cpp_std=c++17', 'buildtype=release'],
	license: 'GPL-3.0-or-later',
)

project_description = 'CivetWeb based HTTP server library for udjat'

#
# Versioning
#
pkg_version = meson.project_version()
version_array = pkg_version.split('.')
pkg_major_version = version_array[0].to_int()
pkg_minor_version = version_array[1].to_int()
pkg_micro_version = version_array[2].to_int()

libversion = '@0@.@1@'.format(pkg_major_version,pkg_minor_version)

#
# i18n
#
subdir('po')

libudjat = dependency(
  'libudjat',
  required: true, 
  version: '>=3.0.0',
  fallback: ['libudjat'],
)

json_c = dependency('json', required: false)
civetweb = dependency('civetweb', fallback: ['civetweb', 'civetweb_dep'])

lib_deps = [
  libudjat,
  json_c,
  civetweb,
  dependency(
    'pugixml',
    method : 'pkg-config',
    required: true,
  ),
]

#
# Compiler flags
#
cxx = meson.get_compiler('cpp')
enable_debug = get_option('debug') or get_option('buildtype').contains('debug')

time_epoch = run_command('sh', '-c', 'echo "$SOURCE_DATE_EPOCH"', check : true).stdout().strip()
if time_epoch == ''
  time_epoch = run_command('date','+%Y%m%d', check: true).stdout().strip()
endif

compiler_flags_common = [
  '-ggdb3',
  '-fvisibility=hidden',
  '-fPIC',
  '-DBUILD_DATE='+time_epoch,
  '-Wno-overloaded-virtual',
]

if enable_debug
  compiler_flags_common += [
	  '-DDEBUG=1'
  ]
else
  compiler_flags_common += [
	'-DNDEBUG=1'
  ]
endif

compiler_flags_common += [
  '-DHAVE_CONFIG_H=1'
]

global_cxxflags = cxx.get_supported_arguments(compiler_flags_common)

add_project_arguments(global_cxxflags, language: 'cpp')

#
# Configuration
#
app_conf = configuration_data()
app_conf.set_quoted('PACKAGE_NAME', meson.project_name())
app_conf.set_quoted('PACKAGE_VERSION', meson.project_version())
app_conf.set_quoted('PACKAGE_DESCRIPTION', project_description)
app_conf.set('PACKAGE_VERSION_MAJOR', pkg_major_version)
app_conf.set('PACKAGE_VERSION_MINOR', pkg_minor_version)
app_conf.set('PACKAGE_VERSION_MICRO', pkg_micro_version)

app_conf.set_quoted('LOG_DOMAIN', 'civetweb')
app_conf.set('PRODUCT_NAME', libudjat.get_variable('product_name'))
app_conf.set('PRODUCT_VERSION', libudjat.version())

if json_c.found()
  app_conf.set('HAVE_JSON_C', 1)
endif

#
# Linux standards
#
if cxx.compiles('#include <unistd.h>', name : 'unistd.h')
  app_conf.set('HAVE_UNISTD_H', 1)
endif

includes_dir = include_directories('src/include')

#
# Sources
#
lib_src = [
  'src/library/icon.cc',
  'src/library/connection.cc',
  'src/library/request.cc',
  'src/library/response.cc',
  'src/library/server/interface.cc',
  'src/library/server/call.cc',
  'src/library/server/construct.cc',
  'src/library/server/handler.cc',
  'src/library/handlers/generic.cc',
  'src/library/handlers/favicon.cc',
  'src/library/handlers/icon.cc',
  'src/library/handlers/image.cc',
  'src/library/client/client.cc',
  'src/library/testsuite.cc',
  'src/library/authentication.cc',
  'src/library/oauth2/context.cc',
  'src/library/oauth2/handle.cc',
  'src/library/oauth2/responses.cc',
  'src/library/oauth2/callback.cc',
  ]

module_src = [
  'src/module/handlers/favicon.cc',
  'src/module/handlers/icons.cc',
  'src/module/handlers/images.cc',
  'src/module/handlers/product.cc',
  'src/module/handlers/default.cc',
  'src/module/handlers/user.cc',
  'src/module/oauth2/handler.cc',
  'src/module/oauth2/context.cc',
  'src/module/connection.cc',
  'src/module/request.cc',
  'src/module/connection.cc',
  'src/module/init.cc',

  'src/module/factory.cc',
  'src/module/service/send.cc',
  'src/module/service/construct.cc',
]

test_src = [
  'src/testprogram/testprogram.cc'
]

#
# SDK
# https://mesonbuild.com/Pkgconfig-module.html
#
pkg = import('pkgconfig')

#
# OS dependencies
#
if host_machine.system() == 'windows'

  pkg.generate(
  name: 'lib' + meson.project_name(),
  description: project_description,
  requires: [ 'libudjat' ],
  libraries: [ '-l' + meson.project_name() + '.dll' ]
  )

  # https://mesonbuild.com/Windows-module.html
  windows = import('windows')
  resources = windows.compile_resources(
    configure_file(
      input : 'src/library/os/windows/resources.rc.in',
      output : 'resources.rc',
      configuration : app_conf
    )
  )
  
  lib_src += [
    'src/library/os/windows/image.cc',
  ]

  lib_src += resources
  module_src += resources

else

  pkg.generate(
    name: 'lib' + meson.project_name(),
    description: project_description,
    requires: [ 'libudjat' ],
    requires_private: [ json_c, 'civetweb' ],
    libraries: [ '-l' + meson.project_name() ]
  )

  lib_src += [
    'src/library/os/linux/image.cc',
  ]

endif

#
# Targets
#
config_src = [
  configure_file(
      output : 'config.h', 
      configuration : app_conf
  )
]

if host_machine.system() == 'windows'

  dynamic = shared_library(
    meson.project_name(), 
    config_src + lib_src,
    install: true,
    version : libversion,
    soversion : libversion,
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps,
    include_directories: includes_dir,
    link_args: [
      '-lintl'
    ],
  )

  # https://mesonbuild.com/Reference-manual_functions.html#shared_library
  shared_library(
    'civetweb', 
    config_src + module_src + [ 'src/module/init.cc' ],
    name_prefix: 'udjat-module-',
    install: true,
    link_with: dynamic,
    install_dir: libudjat.get_variable('module_path'),
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps,
    include_directories: includes_dir,
    link_args: [
      '-lintl'
    ],
  )

else

  dynamic = shared_library(
    meson.project_name(), 
    config_src + lib_src,
    install: true,
    version : libversion,
    soversion : libversion,
    cpp_args: [ '-ffat-lto-objects' ],
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps,
    include_directories: includes_dir
  )

  # https://mesonbuild.com/Reference-manual_functions.html#shared_library
  shared_library(
    'civetweb', 
    config_src + module_src + [ 'src/module/init.cc' ],
    name_prefix: 'udjat-module-',
    install: true,
    install_tag: 'module',
    link_with: dynamic,
    install_dir: libudjat.get_variable('module_path'),
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps,
    include_directories: includes_dir
  )

endif

static_library = declare_dependency(
  link_with: static_library(
    meson.project_name(), 
    config_src + lib_src,
    cpp_args: [ '-DSTATIC_LIBRARY', '-fno-lto' ],
    install: true,
    dependencies: lib_deps,
    include_directories: includes_dir
  ),
  include_directories: includes_dir,
  dependencies: lib_deps,
)

static_civetweb = declare_dependency(
  link_with: static_library(
    'udjatcivetweb', 
    config_src + module_src + lib_src,
    install: true,
    cpp_args: [ '-DSTATIC_LIBRARY', '-fno-lto' ],
    dependencies: lib_deps,
    include_directories: includes_dir
  ),
  include_directories: includes_dir,
  dependencies: lib_deps,
)

dynamic_library = declare_dependency(
  link_with: dynamic,
  include_directories: includes_dir,
)

executable(
  meson.project_name(),
  config_src + test_src,
  install: false,
  link_with: [ dynamic ],
  dependencies: [ libudjat, civetweb ],
  include_directories: includes_dir
)

install_headers(
  'src/include/udjat/tools/http/connection.h',
  'src/include/udjat/tools/http/icon.h',
  'src/include/udjat/tools/http/image.h',
  'src/include/udjat/tools/http/oauth.h',
  'src/include/udjat/tools/http/request.h',
  'src/include/udjat/tools/http/response.h',
  subdir: 'udjat/tools/http'  
)

install_headers(
  'src/include/udjat/tools/civetweb/service.h',
  subdir: 'udjat/tools/civetweb'  
)

install_headers(
  'src/include/udjat/module/civetweb.h',
  subdir: 'udjat/module'  
)
