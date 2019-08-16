## automake.am
##
## Automake.am for netisdn/netisdn
##
## (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.10 2008/10/07 16:40:32 cvs Exp $
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

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -DQTDIR=\"@QT_DIR@\" @QT_CXXFLAGS@
INCLUDES = -I$(top_srcdir)/lib
LIBS = @QT_LIBS@ -L$(top_srcdir)/lib
MOC = @QT_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

bin_PROGRAMS = netisdn

dist_netisdn_SOURCES = netisdn.cpp netisdn.h\
                       advanced_dialog.cpp advanced_dialog.h\
                       audiosettings_dialog.cpp audiosettings_dialog.h\
                       stats_dialog.cpp stats_dialog.h

nodist_netisdn_SOURCES = moc_netisdn.cpp\
                         moc_advanced_dialog.cpp\
                         moc_audiosettings_dialog.cpp\
                         moc_stats_dialog.cpp

netisdn_LDADD = -lqui @LIBSPEEX@ @MPEG_L3_LIBS@ @LIBVORBIS@ -lportaudio -lsamplerate -lssl -lnetisdn

EXTRA_DIST = netisdn.pro

CLEANFILES = *~ moc_* *.qm *.obj *.exe
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
