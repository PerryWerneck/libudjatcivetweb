#
# spec file for package libudjatcivetweb
#
# Copyright (c) <2024> Perry Werneck <perry.werneck@gmail.com>.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://github.com/PerryWerneck/libudjatcivetweb/issues
#

%define module_name civetweb

%define product_name %(pkg-config --variable=product_name libudjat)
%define product_version %(pkg-config --variable=product_version libudjat)
%define module_path %(pkg-config --variable=module_path libudjat)

Summary:		HTTP server library for %{product_name}  
Name:			libudjat%{module_name}
Version:		1.2.0+git20241024
Release:		0
License:		LGPL-3.0
Source:			%{name}-%{version}.tar.xz

URL:			https://github.com/PerryWerneck/libudjat%{module_name}

Group:			Development/Libraries/C and C++
BuildRoot:		/var/tmp/%{name}-%{version}

BuildRequires:	binutils
BuildRequires:	coreutils
BuildRequires:	gcc-c++ >= 5
BuildRequires:	pkgconfig(civetweb)
BuildRequires:	pkgconfig(libudjat)
BuildRequires:	pkgconfig(libssl)
BuildRequires:	pkgconfig(libcrypto)
BuildRequires:  meson >= 0.61.4

%description
HTTP server library for %{product_name}

C++ HTTP server classes for use with lib%{product_name}

#---[ Library ]-------------------------------------------------------------------------------------------------------

%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2 | cut -d+ -f1)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

%package -n libudjathttpd%{_libvrs}
Summary:       %{product_name} httpd library
Recommends:    %{product_name}-branding

%description -n libudjathttpd%{_libvrs}
HTTP Server abstraction library for %{product_name}

%if "%{_vendor}" != "debbuild"
%lang_package -n libudjathttpd%{_libvrs}
%endif

#---[ Development ]---------------------------------------------------------------------------------------------------

%package devel
Summary: Development files for %{name}

Requires: libudjathttpd%{_libvrs} = %{version}
Provides: libudjathttpd-devel = %{version}

%if "%{_vendor}" == "debbuild"
Requires:	libudjat-dev
Provides:	libudjathttpd-dev
Provides:	pkgconfig(libudjathttpd)
Provides:	pkgconfig(libudjathttpd-static)
%endif

%description devel
HTTP server library for %{product_name}

C++ HTTP server classes for use with lib%{product_name}

#---[ Module ]--------------------------------------------------------------------------------------------------------

%package -n %{product_name}-module-%{module_name}
Summary: HTTP server module for %{name}

Supplements: udjat-devel
Provides: %{product_name}-module-httpd
Recommends: %{product_name}-branding-http

%description -n %{product_name}-module-%{module_name}
%{product_name} module with http server support.

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%autosetup
%meson

%build
%meson_build

%install
%meson_install
%find_lang libudjathttpd-%{MAJOR_VERSION}.%{MINOR_VERSION} langfiles

%files -n libudjathttpd%{_libvrs}
%defattr(-,root,root)
%{_libdir}/*.so.%{MAJOR_VERSION}.%{MINOR_VERSION}

%if "%{_vendor}" != "debbuild"
%files -n libudjathttpd%{_libvrs}-lang -f langfiles
%endif

%files -n %{product_name}-module-%{module_name}
%{module_path}/*.so

%files devel
%defattr(-,root,root)

%{_libdir}/*.so
%{_libdir}/*.a
%{_libdir}/pkgconfig/*.pc

%dir %{_includedir}/udjat/tools/http
%{_includedir}/udjat/tools/http/*.h

%dir %{_includedir}/udjat/tools/civetweb
%{_includedir}/udjat/tools/civetweb/*.h
%{_includedir}/udjat/module/*.h

%post -n libudjathttpd%{_libvrs} -p /sbin/ldconfig

%postun -n libudjathttpd%{_libvrs} -p /sbin/ldconfig

%changelog

