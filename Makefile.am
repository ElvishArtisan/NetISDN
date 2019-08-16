## automake.am
##
## Top level automake.am for NetISDN
##
## Use automake to process this into a Makefile.in
##
##   (C) Copyright 2002-2006 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.12 2008/11/13 20:50:09 cvs Exp $
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
SUBDIRS = helpers icons xdg lib netisdn netsipd netadmin netfilter client docs
else
SUBDIRS = helpers icons xdg lib netisdn netsipd netadmin client docs
endif

install-exec-local:
	mkdir -p /etc/init.d
	cp rc.netisdn /etc/init.d/netisdn

uninstall-exec:
	rm /etc/init.d/netisdn

rpm:	dist
	cp netisdn-$(VERSION).tar.gz $(RPM_ROOT)/SOURCES/
	$(RPMBUILD) -ba netisdn.spec
	mv $(RPM_ROOT)/RPMS/$(ARCH)/netisdn-$(VERSION)-@RPM_RELEASE@.$(ARCH).rpm $(top_srcdir)/
	mv $(RPM_ROOT)/RPMS/$(ARCH)/netisdn-server-$(VERSION)-@RPM_RELEASE@.$(ARCH).rpm $(top_srcdir)/
	mv $(RPM_ROOT)/SRPMS/netisdn-$(VERSION)-@RPM_RELEASE@.src.rpm $(top_srcdir)/
	rm $(RPM_ROOT)/SOURCES/netisdn-$(VERSION).tar.gz
	rm -rf $(RPM_ROOT)/BUILD/netisdn-$(VERSION)

zip:	dist
	tar -zxf netisdn-@VERSION@.tar.gz
	zip -r netisdn-@VERSION@.zip netisdn-@VERSION@
	rm -rf netisdn-@VERSION@

slack:	all
	./make_slack

EXTRA_DIST = autogen.sh get_target.sh get_distro.sh PACKAGE_VERSION\
             netisdn.pro build_win32.bat netisdn.ism netisdn.spec.in\
             netsipd.conf-sample netisdn.conf-sample rc.netisdn.in\
             make_slack.in slack-desc.in slack_doinst.sh

CLEANFILES = *~ *.tar.gz *.tgz *.mo *.rpm *.zip
DISTCLEANFILES = make_slack slack-desc ltmain.sh config.guess config.sub\
                 *.spec rc.netisdn
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz\
                       config.guess config.status config.sub install-sh\
                       libtool ltmain.sh missing mkinstalldirs depcomp
