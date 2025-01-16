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

Summary:		HTTP server library for %{udjat_product_name}  
Name:			libudjat%{module_name}
Version: 1.4.0
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
HTTP server library for %{udjat_product_name}

C++ HTTP server classes for use with lib%{udjat_product_name}

%package -n libudjathttpd%{udjat_major}_%{udjat_minor}
Summary:       httpd library fo %{udjat_product_name}
Recommends:    %{udjat_product_name}-branding

%description -n libudjathttpd%{udjat_major}_%{udjat_minor}
HTTP Server abstraction library for %{udjat_product_name}

%package devel
Summary: Development files for %{name}
Requires:	libudjathttpd%{udjat_major}_%{udjat_minor} = %{version}
Provides:	%{name}%{udjat_major}-devel = %{version}
Provides:	%{udjat_library}-devel = %{version}

%description devel
HTTP server library for %{udjat_product_name} based on libcivetweb

C++ HTTP server classes for use with lib%{udjat_product_name}

%lang_package -n libudjathttpd%{udjat_major}_%{udjat_minor}

%package -n %{udjat_product_name}%{udjat_module_version}-module-%{module_name}
Summary: HTTP server module for %{name}
%udjat_module_requires

Supplements: udjat-devel
Recommends: %{udjat_product_name}-branding-http

%description -n %{udjat_product_name}%{udjat_module_version}-module-%{module_name}
Dynamic module for http server support on %{udjat_product_name}

%prep
%autosetup
%meson

%build
%meson_build

%install
%meson_install
%find_lang libudjathttpd-%{udjat_major}.%{udjat_minor} langfiles

%files -n libudjathttpd%{udjat_major}_%{udjat_minor}-lang -f langfiles

%files -n libudjathttpd%{udjat_major}_%{udjat_minor}
%defattr(-,root,root)
%{_libdir}/*.so.%{udjat_major}.%{udjat_minor}

%files -n %{udjat_product_name}%{udjat_module_version}-module-%{module_name}
%{udjat_module_path}/*.so

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

%post -n libudjathttpd%{udjat_major}_%{udjat_minor} -p /sbin/ldconfig

%postun -n libudjathttpd%{udjat_major}_%{udjat_minor} -p /sbin/ldconfig

%changelog

