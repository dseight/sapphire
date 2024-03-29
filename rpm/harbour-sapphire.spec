Name: harbour-sapphire
Summary: Sapphire - Unofficial Sketch Mirror
Version: 0.1.0
Release: 1
License: GPLv2
Source0: %{name}-%{version}.tar.bz2
Requires: sailfishsilica-qt5 >= 1.1.83
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5WebSockets)
BuildRequires: pkgconfig(sailfishapp)
BuildRequires: pkgconfig(mlite5)
BuildRequires: cmake

%description
%{summary}.

%prep
%autosetup

%build
%cmake -DVERSION=%{version}
%make_build

%install
%make_install

%files
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
