Name:       libqofonoext-qt6

Summary:    A library of Qt bindings for ofono extensions
Version:    1.0.32
Release:    1
License:    LGPLv2
URL:        https://github.com/sailfishos/libqofonoext
Source0:    %{name}-%{version}.tar.bz2

%define libqofono_version 0.101

BuildRequires:  cmake
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(Qt6Core)
BuildRequires:  pkgconfig(Qt6DBus)
BuildRequires:  pkgconfig(Qt6Quick)
BuildRequires:  pkgconfig(qofono-qt6) >= %{libqofono_version}

# license macro requires rpm >= 4.11
BuildRequires:  pkgconfig(rpm)
%define license_support %(pkg-config --exists 'rpm >= 4.11'; echo $?)

Requires:   libqofono-qt6 >= %{libqofono_version}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
This package contains Qt bindings for ofono extensions

%package declarative
Summary:    Declarative plugin for %{name}
Requires:   %{name} = %{version}-%{release}
Requires:   %{name} = %{version}

%description declarative
This package contains declarative plugin for %{name}

%package devel
Summary:    Development files for %{name}
Requires:   %{name} = %{version}-%{release}
Requires:   %{name} = %{version}

%description devel
This package contains the development header files for %{name}

%prep
%setup -q -n %{name}-%{version}

%build
%cmake . -DLIBQOFONOEXT_VERSION=$(sed 's/+.*//' <<<"%{version}") -DQT_MAJOR_VERSION=6
%cmake_build

%install
%cmake_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%{_libdir}/%{name}.so.*
%if %{license_support} == 0
%license LICENSE.LGPL
%endif

%files declarative
%{_libdir}/qt6/qml/org/nemomobile/ofono

%files devel
%{_libdir}/%{name}.so
%{_libdir}/pkgconfig/qofonoext-qt6.pc
%{_includedir}/qofonoext-qt6/*.h
