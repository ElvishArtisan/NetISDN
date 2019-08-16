## Makefile.am
##
## Makefile.am for NetISDN/lib
##
## (C) Copyright 2008-2019 Fred Gleason <fredg@paravelsystems.com>
##
## Use automake to process this into a Makefile.in
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

AM_CPPFLAGS = -Wall @QT_CXXFLAGS@
MOC = @QT_MOC@
CWRAP = ../helpers/cwrap

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

lib_LTLIBRARIES = libnetisdn.la
dist_libnetisdn_la_SOURCES = audiosettings.cpp audiosettings.h\
                             check_version.cpp check_version.h\
                             codec.cpp codec.h\
                             codec_stats.cpp codec_stats.h\
                             cmd_switch.cpp cmd_switch.h\
                             controlconnect.cpp controlconnect.h\
                             datedialog.cpp datedialog.h\
                             datepicker.cpp datepicker.h\
                             dbversion.h\
                             escape_string.cpp escape_string.h\
                             segmeter.cpp segmeter.h\
                             playmeter.cpp playmeter.h\
                             historybox.cpp historybox.h\
                             login.cpp login.h\
                             netconfig.cpp netconfig.h\
                             netconf.cpp netconf.h\
                             netlistviewitem.cpp netlistviewitem.h\
                             oneshot.cpp oneshot.h\
                             passwd.cpp passwd.h\
                             profile.cpp profile.h\
                             profilesection.cpp profilesection.h\
                             profileline.cpp profileline.h\
                             meteraverage.cpp meteraverage.h\
                             ringbuffer.cpp ringbuffer.h\
                             rtpheader.cpp rtpheader.h\
                             rtpcontrolheader.cpp rtpcontrolheader.h\
                             rtpsenderreport.cpp rtpsenderreport.h\
                             rtpreceptionreport.cpp rtpreceptionreport.h\
                             rtpreceptionblock.cpp rtpreceptionblock.h\
                             rtpsourcedescription.cpp rtpsourcedescription.h\
                             rtpbye.cpp rtpbye.h\
                             sqldb.cpp sqldb.h\
                             version.cpp version.h

nodist_libnetisdn_la_SOURCES = moc_codec.cpp\
                               moc_controlconnect.cpp\
                               moc_datedialog.cpp\
                               moc_datepicker.cpp\
                               moc_oneshot.cpp\
                               moc_passwd.cpp\
                               moc_playmeter.cpp\
                               moc_segmeter.cpp\
                               moc_sqldb.cpp\
                               moc_historybox.cpp moc_login.cpp


libnetisdn_la_LDFLAGS = -release $(VERSION)

EXTRA_DIST = lib.pro win32_types.h

CLEANFILES = *~ moc_* *.qm *.obj *.lib *.dll
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
