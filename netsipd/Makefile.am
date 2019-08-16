## Makefile.am
##
## Makefile.am for NetISDN/netsipd
##
## (C) Copyright 2008-2019 Fred Gleason <fredg@paravelsystems.com>
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

AM_CPPFLAGS = -Wall -DPREFIX=\"$(prefix)\" -DQTDIR=\"@QT_DIR@\" @QT_CXXFLAGS@ -I$(top_srcdir)/lib
LIBS = @QT_LIBS@ -L$(top_srcdir)/lib
MOC = @QT_MOC@

# The dependency for qt's Meta Object Compiler (moc)
moc_%.cpp:	%.h
	$(MOC) $< -o $@

sbin_PROGRAMS = netsipd

dist_netsipd_SOURCES = netsipd.cpp netsipd.h\
                       serversocket.cpp serversocket.h\
                       sipconnection.cpp sipconnection.h

nodist_netsipd_SOURCES = moc_netsipd.cpp moc_serversocket.cpp

netsipd_LDADD = -lqui @MPEG_L3_LIBS@ @LIBSPEEX@ @LIBVORBIS@ -lportaudio -lsamplerate -lssl -lGeoIP -lnetisdn

EXTRA_DIST = netsipd.pro

CLEANFILES = *~ moc_* *.qm *.obj *.exe
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
