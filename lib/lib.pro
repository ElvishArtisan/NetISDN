# lib.pro
#
# The lib/ QMake project file for NetISDN.
#
# (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
#
#      $Id: lib.pro,v 1.16 2009/01/26 13:05:20 pcvs Exp $
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2 as
#   published by the Free Software Foundation.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public
#   License along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

TEMPLATE = lib

win32 {
  DEFINES += WIN32
  DEFINES += VERSION=\"$$[VERSION]\"
  DEFINES += PACKAGE=\"netisdn\"
  DEFINES += PACKAGE_VERSION=\"$$[VERSION]\" 
  DEFINES += PACKAGE_NAME=\"netisdn\"
  DEFINES += PACKAGE_BUGREPORT=\"fredg@paravelsystems.com\"
  DEFINES += HAVE_MPEG_L3
  DEFINES += HAVE_SPEEX
  DEFINES += HAVE_VORBIS
}

SOURCES += audiosettings.cpp
SOURCES += cmd_switch.cpp
SOURCES += codec.cpp
SOURCES += codec_stats.cpp
SOURCES += controlconnect.cpp
SOURCES += datedialog.cpp
SOURCES += datepicker.cpp
SOURCES += escape_string.cpp
SOURCES += historybox.cpp
SOURCES += meteraverage.cpp
SOURCES += netconf.cpp
SOURCES += netlistviewitem.cpp
SOURCES += oneshot.cpp
SOURCES += passwd.cpp
SOURCES += playmeter.cpp
SOURCES += profile.cpp
SOURCES += profilesection.cpp
SOURCES += profileline.cpp
SOURCES += segmeter.cpp
SOURCES += ringbuffer.cpp
SOURCES += rtpbye.cpp
SOURCES += rtpcontrolheader.cpp
SOURCES += rtpheader.cpp
SOURCES += rtpreceptionblock.cpp
SOURCES += rtpreceptionreport.cpp
SOURCES += rtpsenderreport.cpp
SOURCES += rtpsourcedescription.cpp
SOURCES += sqldb.cpp

HEADERS += audiosettings.h
HEADERS += cmd_switch.h
HEADERS += codec.h
HEADERS += codec_stats.h
HEADERS += controlconnect.h
HEADERS += datedialog.h
HEADERS += datepicker.h
HEADERS += escape_string.h
HEADERS += historybox.h
HEADERS += meteraverage.h
HEADERS += netconf.h
HEADERS += netlistviewitem.h
HEADERS += oneshot.h
HEADERS += passwd.h
HEADERS += playmeter.h
HEADERS += profile.h
HEADERS += profilesection.h
HEADERS += profileline.h
HEADERS += segmeter.h
HEADERS += ringbuffer.h
HEADERS += rtpbye.cpp
HEADERS += rtpcontrolheader.cpp
HEADERS += rtpheader.cpp
HEADERS += rtpreceptionblock.cpp
HEADERS += rtpreceptionreport.cpp
HEADERS += rtpsenderreport.cpp
HEADERS += rtpsourcedescription.cpp
HEADERS += sqldb.h

INCLUDEPATH += ..\..\portaudio\include
INCLUDEPATH += ..\..\libmad-0.15.1b
INCLUDEPATH += ..\..\lame-3.97\include
INCLUDEPATH += ..\..\speex\include
INCLUDEPATH += ..\..\libsamplerate-0.1.2\src
INCLUDEPATH += ..\..\libogg-1.1.3\include
INCLUDEPATH += ..\..\libvorbis-1.2.0\include

CONFIG += qt staticlib thread
