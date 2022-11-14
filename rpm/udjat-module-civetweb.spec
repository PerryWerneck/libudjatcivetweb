#
# spec file for package libudjathttpd
#
# Copyright (c) 2015 SUSE LINUX GmbH, Nuernberg, Germany.
# Copyright (C) <2008> <Banco do Brasil S.A.>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

Summary:		CivetWEB HTTP exporter for UDJat 
Name:			udjat-module-civetweb
Version:		1.0
Release:		0
License:		LGPL-3.0
Source:			%{name}-%{version}.tar.xz

URL:			https://github.com/PerryWerneck/udjat-module-civetweb

Group:			Development/Libraries/C and C++
BuildRoot:		/var/tmp/%{name}-%{version}

%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2 | cut -d+ -f1)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

BuildRequires:	autoconf >= 2.61
BuildRequires:	automake
BuildRequires:	libtool
BuildRequires:	binutils
BuildRequires:	coreutils
BuildRequires:	gcc-c++

BuildRequires:	pkgconfig(libudjat)
BuildRequires:	pkgconfig(pugixml)
BuildRequires:	civetweb-devel >= 1.15

Provides:		udjat-module-httpd
Conflicts:		otherproviders(udjat-module-httpd)

Provides:		udjat-module-http

%description
HTTP exporter module for libudjat based on CivetWEB library.

#---[ Library ]-------------------------------------------------------------------------------------------------------

%package -n libudjathttpd%{_libvrs}
Summary:	UDJat httpd library

%description -n libudjathttpd%{_libvrs}
HTTP Server abstraction library for udjat

#---[ Development ]---------------------------------------------------------------------------------------------------

%package -n udjat-httpd-devel
Summary:	Development files for %{name}
Requires:	pkgconfig(libudjat)
Requires:	libudjathttpd%{_libvrs} = %{version}

%description -n udjat-httpd-devel

Development files for Udjat's HTTP server abstraction library.

%lang_package

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%setup

NOCONFIGURE=1 \
	./autogen.sh

%configure

%build
make all

%install
%makeinstall
%find_lang libudjathttpd-%{MAJOR_VERSION}.%{MINOR_VERSION} langfiles

%files
%defattr(-,root,root)
%{_libdir}/udjat-modules/*/*.so
%config %{_sysconfdir}/udjat.conf.d/*.conf

%files -n libudjathttpd%{_libvrs}
%defattr(-,root,root)
%{_libdir}/libudjathttpd.so.%{MAJOR_VERSION}.%{MINOR_VERSION}

%files lang -f langfiles

%files -n udjat-httpd-devel
%defattr(-,root,root)
%{_includedir}/udjat/tools/http/*.h
%{_libdir}/*.so
%{_libdir}/*.a
%{_libdir}/pkgconfig/*.pc

%pre -n libudjathttpd%{_libvrs} -p /sbin/ldconfig

%post -n libudjathttpd%{_libvrs} -p /sbin/ldconfig

%postun -n libudjathttpd%{_libvrs} -p /sbin/ldconfig

%changelog

