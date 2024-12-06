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
	version: '1.4.0',
	default_options : ['c_std=c11', 'cpp_std=c++17'],
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

#
# Required libraries
#
sdk_deps = [
  'libudjat',
  'civetweb'
]

libudjat = dependency('libudjat')

lib_deps = [
  libudjat,
  dependency('civetweb'),
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
  '-ffat-lto-objects',
  '-fvisibility=hidden',
  '-fPIC',
  '-DBUILD_DATE='+time_epoch,
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

#
# Crypto APIs
#
# No libkcapi, use libssl
ssl = dependency('libssl', required: false)
if ssl.found()
  sdk_deps += [
    'libssl',
    'libcrypto'
  ]
  lib_deps += [ 
      ssl,
      dependency('libcrypto')
  ]
  app_conf.set('HAVE_LIBSSL',1)
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
  'src/library/exec.cc',
  'src/library/handler.cc',
  'src/library/icon.cc',
  'src/library/index.cc',
  'src/library/keypair.cc',
  'src/library/layout/csv.cc',
  'src/library/oauth2/access_token.cc',
  'src/library/oauth2/authorize.cc',
  'src/library/oauth2/signin.cc',
  'src/library/oauth2/client.cc',
  'src/library/oauth2/user.cc',
  'src/library/template.cc',
  'src/library/value.cc',
  'src/library/connection.cc',
  'src/library/request.cc',
  'src/library/response.cc',
  'src/library/server/interface.cc',
  'src/library/server/call.cc',
  'src/library/server/construct.cc',
  'src/library/server/handler.cc',
]

module_src = [
  'src/module/handlers/api.cc',
  'src/module/handlers/favicon.cc',
  'src/module/handlers/icons.cc',
  'src/module/handlers/images.cc',
  'src/module/handlers/pubkey.cc',
  'src/module/handlers/report.cc',
  'src/module/handlers/root.cc',
  'src/module/handlers/swagger.cc',
  'src/module/handlers/product.cc',
  'src/module/oauth2/handler.cc',
  'src/module/protocol.cc',
  'src/module/worker/get.cc',
  'src/module/worker/test.cc',
  'src/module/worker/worker.cc',
  'src/module/connection.cc',
  'src/module/request.cc',
  'src/module/connection.cc',
  'src/module/custom.cc',
  'src/module/init.cc',
  'src/module/protocol.cc',
  'src/module/factory.cc',
  'src/module/service/send.cc',
  'src/module/service/construct.cc',
]

test_src = [
  'src/testprogram/testprogram.cc'
]

#
# SDK
#
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
    'src/library/os/windows/oauth2user.cc',
  ]

  lib_src += resources
  module_src += resources

else

  pkg.generate(
    name: 'lib' + meson.project_name(),
    description: project_description,
    libraries: [ '-l' + meson.project_name() ]
  )

  lib_src += [
    'src/library/os/linux/image.cc',
    'src/library/os/linux/oauth2user.cc',
  ]

endif

pkg.generate(
  name: 'lib' + meson.project_name() + '-static',
  description: project_description,
  requires: sdk_deps,
  libraries: [ 
    '-l:lib' + meson.project_name() + '.a',
  ]
)

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
  module = shared_library(
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
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps,
    include_directories: includes_dir
  )

  # https://mesonbuild.com/Reference-manual_functions.html#shared_library
  module = shared_library(
    'civetweb', 
    config_src + module_src + [ 'src/module/init.cc' ],
    name_prefix: 'udjat-module-',
    install: true,
    link_with: dynamic,
    install_dir: libudjat.get_variable('module_path'),
    gnu_symbol_visibility: 'hidden',
    dependencies: lib_deps,
    include_directories: includes_dir
  )

endif

static_library(
  meson.project_name(), 
  config_src + lib_src,
  install: true,
  dependencies: lib_deps,
  include_directories: includes_dir
)

static_library(
  'udjatcivetweb', 
  config_src + module_src + lib_src,
  install: true,
  dependencies: lib_deps,
  include_directories: includes_dir
)

executable(
  'udjat',
  config_src + test_src,
  install: false,
  link_with: [ dynamic, module ],
  link_args: [ '-l:libudjat-testing.a' ],
  dependencies: [ libudjat ],
  include_directories: includes_dir
)

install_headers(
  'src/include/udjat/tools/http/connection.h',
  'src/include/udjat/tools/http/handler.h',
  'src/include/udjat/tools/http/icon.h',
  'src/include/udjat/tools/http/image.h',
  'src/include/udjat/tools/http/keypair.h',
  'src/include/udjat/tools/http/layouts.h',
  'src/include/udjat/tools/http/oauth.h',
  'src/include/udjat/tools/http/report.h',
  'src/include/udjat/tools/http/request.h',
  'src/include/udjat/tools/http/response.h',
  'src/include/udjat/tools/http/server.h',
  'src/include/udjat/tools/http/template.h',
  subdir: 'udjat/tools/http'  
)

install_headers(
  'src/include/udjat/tools/civetweb/protocol.h',
  'src/include/udjat/tools/civetweb/service.h',
  subdir: 'udjat/tools/civetweb'  
)

install_headers(
  'src/include/udjat/module/civetweb.h',
  subdir: 'udjat/module'  
)
