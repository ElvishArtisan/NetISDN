## Makefile.am
##
## icons/Makefile.am for NetISDN
##
## (C) Copyright 2008-2019 Fred Gleason <fredg@salemradiolabs.com>
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
##
## Use automake to process this into a Makefile.in

install-exec-am:	
	mkdir -p $(DESTDIR)@prefix@/share/icons/hicolor/22x22/apps
	cp netisdn-22x22.xpm $(DESTDIR)@prefix@/share/icons/hicolor/22x22/apps/netisdn.xpm
	mkdir -p $(DESTDIR)@prefix@/share/icons/hicolor/48x48/apps
	cp netisdn-48x48.xpm $(DESTDIR)@prefix@/share/icons/hicolor/48x48/apps/netisdn.xpm

uninstall-local:	
	rm -f $(DESTDIR)@prefix@/share/icons/hicolor/22x22/apps/netisdn.xpm
	rm -f $(DESTDIR)@prefix@/share/icons/hicolor/48x48/apps/netisdn.xpm

EXTRA_DIST = netisdn.res\
             netisdn.ico\
             autorun.inf.in\
             netisdn-48x48.xpm\
             netisdn-22x22.xpm\
             admin.xpm\
             user.xpm\
             active_call.xpm\
             call.xpm\
             greenball.xpm\
             redball.xpm\
             whiteball.xpm

CLEANFILES = *~
DISTCLEANFILES = *.inf
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4
