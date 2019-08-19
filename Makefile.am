## automake.am
##
## Top level automake.am for NetISDN
##
## Use automake to process this into a Makefile.in
##
##   (C) Copyright 2002-2019 Fred Gleason <fredg@paravelsystems.com>
##
##   This program is free software; you can redistribute it and/or modify
##   it under the terms of the GNU General Public License version 2 as
##   published by the Free Software Foundation.
##
##   This program is distributed in the hope that it will be useful,
##   but WITHOUT ANY WARRANTY; without even the implied warranty of
##   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##   GNU General Public License for more details.
##
##   You should have received a copy of the GNU General Public
##   License along with this program; if not, write to the Free Software
##   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
##

if BUILD_NETFILTER
SUBDIRS = helpers\
          icons\
          xdg\
          lib\
          netisdn\
          netsipd\
          netadmin\
          netfilter\
          systemd\
          client\
          docs
else
SUBDIRS = helpers\
          icons\
          xdg\
          lib\
          netisdn\
          netsipd\
          netadmin\
          systemd\
          client\
          docs
endif

ACLOCAL_AMFLAGS = -I m4

maintainer-clean-local:	
	rm -rf m4

rpm:	dist
	mkdir -p $(HOME)/rpmbuild/SOURCES
	cp netisdn-$(VERSION).tar.gz $(HOME)/rpmbuild/SOURCES/
	$(RPMBUILD) -ba --target $(ARCH)-$(VENDOR)-linux netisdn.spec
	mv $(HOME)/rpmbuild/RPMS/$(ARCH)/netisdn-*.rpm $(top_srcdir)/
	mv $(HOME)/rpmbuild/SRPMS/netisdn-*.src.rpm $(top_srcdir)/
	rm $(HOME)/rpmbuild/SOURCES/netisdn-$(VERSION).tar.gz

EXTRA_DIST = autogen.sh\
             get_target.sh\
             get_distro.pl\
             PACKAGE_VERSION\
             netisdn.pro\
             build_win32.bat\
             netisdn.ism\
             netisdn.spec.in\
             netsipd.conf-sample\
             netisdn.conf-sample\
             make_slack.in\
             slack-desc.in\
             slack_doinst.sh

CLEANFILES = *~ *.tar.gz *.tgz *.mo *.rpm *.zip
DISTCLEANFILES = make_slack slack-desc ltmain.sh config.guess config.sub\
                 *.spec rc.netisdn
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz\
                       config.guess config.status config.sub install-sh\
                       libtool ltmain.sh missing mkinstalldirs depcomp
