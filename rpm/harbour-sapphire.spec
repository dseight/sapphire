Name: harbour-sapphire
Summary: Sapphire - Unofficial Sketch Mirror
Version: 0.1.0
Release: 1
Group: Qt/Qt
License: GPL-2.0-or-later
Source0: %{name}-%{version}.tar.bz2
Requires: sailfishsilica-qt5 >= 1.1.83
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5WebSockets)
BuildRequires: pkgconfig(sailfishapp)
BuildRequires: pkgconfig(mlite5)
BuildRequires: pkgconfig(qmdnsengine)

%define __provides_exclude_from ^%{_datadir}/.*$
%define __requires_exclude ^libqmdnsengine.*$

%description
%{summary}.

%prep
%setup -q

%build
%qmake5 -r VERSION=%{version}
%make_build

%install
%qmake5_install

# Bundle qmdnsengine library into package
mkdir -p %{buildroot}%{_datadir}/%{name}/lib
cp %{_libdir}/libqmdnsengine.so.* %{buildroot}%{_datadir}/%{name}/lib

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
