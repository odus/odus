# $Id: php-odus-zb.spec <DATE> xma@zynga.com $
%define php_extdir %(php-config --extension-dir 2>/dev/null || echo %{_libdir}/php4)
%define php_config_opts %(php-config --configuration-options 2>/dev/null)
%define _dbg %{nil}
%define peclname %{lua:

name = "php-odus-zb"

php_config_opts = rpm.expand("%{php_config_opts}")
if (string.find(php_config_opts, "--enable-debug", 1, true)) then
  name = "php-dbg" .. string.sub(name, 4)
end
print(name)
}
%{lua: 
current_name = rpm.expand("%{peclname}")
if (string.find(current_name, "-dbg", 1, true)) then
  rpm.define("_dbg -dbg")
end
}

Summary: php extention package for odus
Name: %{peclname}
Version: <VERSION> 
Release: 1.zb_5.2.10
License: XMA 
Group: Development/Languages
URL: http://php.net/
Source: <SOURCE>
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
Requires: php%{_dbg}
BuildRequires: php%{_dbg}, php%{_dbg}-devel
# Required by phpize
BuildRequires: autoconf, automake, libtool, gcc-c++

%description
odus means On Demand Unserialization & Serialization. It is used to do lazy unserialize and serialize and thus reduce cpu time and memory usage. It is fully transparent to php developers.

%prep
%setup -q -n odus-<VERSION> 

%build
# Workaround for broken old phpize on 64 bits
%{__cat} %{_bindir}/phpize | sed 's|/lib/|/%{_lib}/|g' > phpize && sh phpize
%configure
%{__make} clean
%{__make} %{?_smp_mflags}

%install
%{__rm} -rf %{buildroot}
%{__make} install INSTALL_ROOT=%{buildroot}

# Drop in the bit of configuration
%{__mkdir_p} %{buildroot}%{_sysconfdir}/php.d/
#%{__mkdir_p} %{buildroot}%{_sysconfdir}/php.cgi.d/extensions
#%{__mkdir_p} %{buildroot}%{_sysconfdir}/php.cli.d/extensions

%{__cat} > %{buildroot}%{_sysconfdir}/php.d/odus.ini << 'EOF'
; Enable odus extension module
extension=odus.so
odus.remove_default=0
odus.reduce_fatals=0
odus.throw_exceptions=0
odus.format_version=2
odus.force_release_memory=0
EOF

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root, 0755)
#%doc INSTALL CREDITS 
%config(noreplace) %{_sysconfdir}/php.d/odus.ini
#%config(noreplace) %{_sysconfdir}/php.cli.d/extensions/apc.ini
#%config(noreplace) %{_sysconfdir}/php.cgi.d/extensions/apc.ini
%{php_extdir}/odus.so

