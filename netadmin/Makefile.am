## automake.am
##
## Automake.am for netisdn/netadmin
##
## (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
##
##      $Id: Makefile.am,v 1.7 2009/01/26 13:05:20 pcvs Exp $
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

bin_PROGRAMS = netadmin

dist_netadmin_SOURCES = add_user.cpp add_user.h\
                        createdb.cpp createdb.h\
                        edit_user.cpp edit_user.h\
                        list_calls.cpp list_calls.h\
                        list_users.cpp list_users.h\
                        mysql_login.cpp mysql_login.h\
                        netadmin.cpp netadmin.h\
                        opendb.cpp opendb.h\
                        globals.h

nodist_netadmin_SOURCES = moc_add_user.cpp\
                          moc_edit_user.cpp\
                          moc_list_calls.cpp\
                          moc_list_users.cpp\
                          moc_mysql_login.cpp\
                          moc_netadmin.cpp

netadmin_LDADD = -lqui @LIBSPEEX@ @MPEG_L3_LIBS@ @LIBVORBIS@ -lportaudio -lsamplerate -lssl -lnetisdn

EXTRA_DIST = netadmin.pro

CLEANFILES = *~ moc_* *.qm *.obj *.exe
MAINTAINERCLEANFILES = *~ Makefile.in configure aclocal.m4 *.tar.gz moc_*
