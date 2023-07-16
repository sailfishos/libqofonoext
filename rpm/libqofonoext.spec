Name:       libqofonoext

Summary:    A library of Qt bindings for ofono extensions
Version:    1.0.31
Release:    1
License:    LGPLv2
URL:        https://github.com/sailfishos/libqofonoext
Source0:    %{name}-%{version}.tar.bz2

%define libqofono_version 0.101

BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(qofono-qt5) >= %{libqofono_version}

# license macro requires rpm >= 4.11
BuildRequires:  pkgconfig(rpm)
%define license_support %(pkg-config --exists 'rpm >= 4.11'; echo $?)

Requires:   libqofono-qt5 >= %{libqofono_version}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}

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
%qtc_qmake5
%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/%{name}.so.*
%if %{license_support} == 0
%license LICENSE.LGPL
%endif

%files declarative
%defattr(-,root,root,-)
%{_libdir}/qt5/qml/org/nemomobile/ofono

%files devel
%defattr(-,root,root,-)
%{_libdir}/%{name}.so
%{_libdir}/pkgconfig/qofonoext.pc
%{_includedir}/qofonoext/*.h
