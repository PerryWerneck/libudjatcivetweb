project('civetweb', 'c',
  version: '1.16',
  default_options: ['c_std=c99']
)

civetweb_src = files('src/civetweb.c')
civetweb_inc = include_directories('include')

civetweb_deps = [dependency('threads')]
civetweb_args = [
  '-D_XOPEN_SOURCE=600',
  '-DUSE_STACK_SIZE=102400'
]

openssl_dep = dependency('openssl', required: false, disabler: true)

if openssl_dep.found()
  civetweb_deps += openssl_dep
  civetweb_args += ['-DNO_SSL_DL']
  
  # --- BEGIN OPENSSL VERSION AUTO-DETECTION ---
  # Extract version string (e.g., "3.0.2" or "1.1.1f")
  ssl_version = openssl_dep.version()
  
  if ssl_version.version_compare('>=3.0.0')
    civetweb_args += ['-DOPENSSL_API_3_0']
  elif ssl_version.version_compare('>=1.1.0')
    civetweb_args += ['-DOPENSSL_API_1_1']
  else
    civetweb_args += ['-DOPENSSL_API_1_0']
  endif
  # --- END OPENSSL VERSION AUTO-DETECTION ---
  
else
  civetweb_args += ['-DNO_SSL']
endif

civetweb_lib = static_library('civetweb',
  civetweb_src,
  include_directories: civetweb_inc,
  c_args: civetweb_args,
  dependencies: civetweb_deps
)

civetweb_dep = declare_dependency(
  link_with: civetweb_lib,
  include_directories: civetweb_inc
)
