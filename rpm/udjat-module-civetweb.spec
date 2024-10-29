#
# spec file for package udjat-module-civetweb
#
# Copyright (C) <2024> Perry Werneck <perry.werneck@gmail.com>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://github.com/PerryWerneck/udjat-module-civetweb/issues
#

%define product_name %(pkg-config --variable=product_name libudjat)
%define module_path %(pkg-config --variable=module_path libudjat)
%define product_version %(pkg-config --variable=product_version libudjat)

Summary:		CivetWEB HTTP exporter for %{product_name} 
Name:			udjat-module-civetweb
Version:		1.2+git20240919
Release:		0
License:		LGPL-3.0
Source:			%{name}-%{version}.tar.xz

URL:			https://github.com/PerryWerneck/udjat-module-civetweb

Group:			Development/Libraries/C and C++
BuildRoot:		/var/tmp/%{name}-%{version}

%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2 | cut -d+ -f1)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

BuildRequires:	binutils
BuildRequires:	coreutils
BuildRequires:	gcc-c++

BuildRequires:	pkgconfig(libudjat) >= 1.2
BuildRequires:	pkgconfig(pugixml)
BuildRequires:	pkgconfig(civetweb) >= 1.15
BuildRequires:	gettext-devel

# Required to allow ouauth authentication
BuildRequires:  pam-devel

# Should be 1.1 because of civetweb
BuildRequires:	libopenssl-1_1-devel

Provides:		udjat%{product_version}-module-http
Provides:		udjat%{product_version}-module-httpd
Conflicts:		otherproviders(udjat%{product_version}-module-httpd)

Recommends:		udjat-branding-http

%if 0%{?suse_version} == 01500
BuildRequires:  meson = 0.61.4
%else
BuildRequires:  meson
%endif

%description
HTTP exporter module for %{product_name} based on CivetWEB library.

#---[ Library ]-------------------------------------------------------------------------------------------------------

%package -n libudjathttpd%{_libvrs}
Summary:	%{product_name} httpd library

%description -n libudjathttpd%{_libvrs}
HTTP Server abstraction library for %{product_name}

%lang_package -n libudjathttpd%{_libvrs}

#---[ Development ]---------------------------------------------------------------------------------------------------

%package -n libudjathttpd-devel
Summary:	HTTP server libraries for %{product_name}.

Requires:	pkgconfig(libudjat)
Requires:	libudjathttpd%{_libvrs} = %{version}
Provides:	udjat-httpd-devel = %{version}

%description -n libudjathttpd-devel

Development files for %{product_name}'s HTTP server abstraction library.

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%autosetup
%meson

%build
%meson_build

%install
%meson_install
%find_lang libudjathttpd-%{MAJOR_VERSION}.%{MINOR_VERSION} langfiles

%files
%defattr(-,root,root)
%{module_path}/*.so

%files -n libudjathttpd%{_libvrs}
%defattr(-,root,root)
%{_libdir}/libudjathttpd.so.%{MAJOR_VERSION}.%{MINOR_VERSION}

%files -n libudjathttpd%{_libvrs}-lang -f langfiles

%files -n libudjathttpd-devel
%defattr(-,root,root)

%{_includedir}/udjat/tools/http/*.h

%dir %{_includedir}/udjat/tools/civetweb
%{_includedir}/udjat/tools/civetweb/*.h

%{_libdir}/*.so
%{_libdir}/*.a
%{_libdir}/pkgconfig/*.pc

%pre -n libudjathttpd%{_libvrs} -p /sbin/ldconfig

%post -n libudjathttpd%{_libvrs} -p /sbin/ldconfig

%postun -n libudjathttpd%{_libvrs} -p /sbin/ldconfig

%changelog

