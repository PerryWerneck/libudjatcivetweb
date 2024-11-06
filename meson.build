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
	version: '1.3.1',
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
subdir('po')

#
# LibUdjat
#
libudjat = dependency('libudjat')

lib_deps = [
  libudjat,
  dependency('civetweb'),
  dependency('libssl'),
  dependency('libcrypto')
]

#
# Compiler flags
#
cxx = meson.get_compiler('cpp')
enable_debug = get_option('debug') or get_option('buildtype').contains('debug')

compiler_flags_common = [
  '-ggdb3',
  '-ffat-lto-objects',
  '-fvisibility=hidden',
  '-fPIC',
  '-DBUILD_DATE='+run_command('date','+%Y%m%d').stdout().strip(),
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
app_conf.set('PRODUCT_NAME',libudjat.get_variable('product_name'))

app_conf.set('HAVE_LIBSSL',1)

if cxx.compiles('#include <unistd.h>', name : 'unistd.h')
  app_conf.set('HAVE_UNISTD_H', 1)
endif

#
# SDK
#
sdk_conf = configuration_data()
sdk_conf.set('PACKAGE_NAME', 'udjatcivetweb')
sdk_conf.set('PACKAGE_VERSION', meson.project_version())
sdk_conf.set('PACKAGE_DESCRIPTION', project_description)
sdk_conf.set('prefix',get_option('prefix'))
sdk_conf.set('exec_prefix',get_option('prefix'))
sdk_conf.set('includedir',get_option('includedir'))
sdk_conf.set('libdir',get_option('libdir'))

includes_dir = include_directories('../src/include')

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
  'src/library/report.cc',
  'src/library/server.cc',
  'src/library/template.cc',
  'src/library/value.cc',
  'src/library/connection.cc',
  'src/library/request.cc',
  'src/library/response.cc',
]

module_src = [
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
  'src/module/send.cc',
  'src/module/connection.cc',
  'src/module/custom.cc',
  'src/module/init.cc',
  'src/module/protocol.cc',
  'src/module/service.cc',
]

test_src = [
  'src/testprogram/testprogram.cc'
]

#
# OS dependencies
#
if host_machine.system() == 'windows'
  sdk_conf.set('REQUIRES','')
  sdk_conf.set('DYNLIBS','-ludjathttpd.dll')
  sdk_conf.set('STATICLIBS','-ludjathttpd')
  app_conf.set('HAVE_WINHTTP', 1)
  lib_src += [
    'src/library/os/windows/image.cc',
    'src/library/os/windows/oauth2user.cc',
  ]
else
  sdk_conf.set('REQUIRES','libcurl')
  sdk_conf.set('DYNLIBS','-ludjathttpd')
  sdk_conf.set('STATICLIBS','-l:libudjathttpd.a')
  app_conf.set('HAVE_CURL', 1)
  lib_src += [
    'src/library/os/linux/image.cc',
    'src/library/os/linux/oauth2user.cc',
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

executable(
  'httpdtest',
  config_src + test_src,
  install: false,
  link_with: [ dynamic, module ],
  link_args: [ '-l:libudjat-testing.a' ],
  dependencies: [ libudjat ],
  include_directories: includes_dir
)

configure_file(
  input : 'sdk/dynamic.pc.in',
  output : meson.project_name() +'.pc',
  install: true,
  install_dir: get_option('libdir') + '/pkgconfig',
  configuration : sdk_conf
)

configure_file(
  input : 'sdk/static.pc.in',
  output : meson.project_name() + '-static.pc',
  install: true,
  install_dir: get_option('libdir') + '/pkgconfig',
  configuration : sdk_conf
)

configure_file(
  input : 'sdk/module.pc.in',
  output : 'udjatcivetweb.pc',
  install: true,
  install_dir: get_option('libdir') + '/pkgconfig',
  configuration : sdk_conf
)

#
# Common targets
#
install_subdir('src/include/udjat', install_dir : get_option('includedir'))
