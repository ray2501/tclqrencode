%{!?directory:%define directory /usr}

%define buildroot %{_tmppath}/%{name}

Name:          tclqrencode
Summary:       Tcl interface for libqrencode 
Version:       1.2
Release:       1
License:       BSD-2-Clause
Group:         Development/Libraries/Tcl
Source:        %{name}-%{version}.tar.gz
URL:           https://github.com/ray2501/tclqrencode
BuildRequires: autoconf
BuildRequires: gcc
BuildRequires: make
BuildRequires: tcl-devel >= 8.4
BuildRequires: libpng-devel
Requires:      tcl >= 8.4
Requires:      libpng
BuildRoot:     %{buildroot}

%description
It is Tcl interface for libqrencode.

Libqrencode is a C library for encoding data in a QR Code symbol, a kind of 2D 
symbology that can be scanned by handy terminals such as a mobile phone with 
CCD.

Tclqrencode is using libqrencode to encode string to be a EPS/PNG/SVG/XPM file.

%prep
%setup -q -n %{name}-%{version}

%build
CFLAGS="%optflags" ./configure \
	--prefix=%{directory} \
	--exec-prefix=%{directory} \
	--libdir=%{directory}/%{_lib}
make 

%install
make DESTDIR=%{buildroot} pkglibdir=%{tcl_archdir}/%{name}%{version} install

%clean
rm -rf %buildroot

%files
%defattr(-,root,root)
%doc README.md libqrencode.COPYING LICENSE
%{tcl_archdir}
