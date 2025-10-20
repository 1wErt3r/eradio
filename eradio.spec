Name:           eradio
Version:        0.0.14
Release:        1%{?dist}
Summary:        A simple EFL internet radio player
License:        MIT
URL:            https://github.com/1wErt3r/eradio
Source0:        https://github.com/1wErt3r/eradio/releases/download/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  automake
BuildRequires:  autoconf
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  desktop-file-utils

Requires:       elementary
Requires:       libxml2

%description
eradio is a simple internet radio player built with the Enlightenment Foundation Libraries (EFL).

%prep
%autosetup

%build
%configure
%make_build

%install
%make_install

desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop

%files
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/128x128/apps/%{name}.png
%license LICENSE
%doc README.md

%changelog
* Mon Oct 20 2025 1wErt3r <root@politebot.com> - 0.0.14
- Show bitrate and other station metadata in search results 
* Fri Oct 17 2025 1wErt3r <root@politebot.com> - 0.0.13
- Add manual URL entry, add volume slider 
* Wed Oct 15 2025 1wErt3r <root@politebot.com> - 0.0.12
- Use genlist for search results, clear status when clicking stop
* Sun Oct 12 2025 1wErt3r <root@politebot.com> - 0.0.8
- Initial package release
