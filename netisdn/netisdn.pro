# netisdn.pro
#
# The netisdn/ QMake project file for NetISDN
#
# (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
#
#      $Id: netisdn.pro,v 1.11 2008/10/14 14:06:17 cvs Exp $
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

TEMPLATE = app

TARGET = netisdn

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

SOURCES += advanced_dialog.cpp
SOURCES += stats_dialog.cpp
SOURCES += audiosettings_dialog.cpp
SOURCES += netisdn.cpp

HEADERS += advanced_dialog.h
HEADERS += audiosettings_dialog.h
HEADERS += netisdn.h
HEADERS += stats_dialog.h

RES_FILE += ..\icons\netisdn.res

INCLUDEPATH += ..\lib
INCLUDEPATH += ..\..\portaudio\include
INCLUDEPATH += ..\..\libmad-0.15.1b
INCLUDEPATH += ..\..\lame-3.97\include
INCLUDEPATH += ..\..\libsamplerate-0.1.2\src
INCLUDEPATH += ..\..\libogg-1.1.3\include
INCLUDEPATH += ..\..\libvorbis-1.2.0\include

LIBS = -lqui -L..\lib -llib
LIBS += -L..\..\lame-3.97\libmp3lame\Debug -llame_enc -llibmp3lame
LIBS += -L..\..\lame-3.97 -llame_enc
LIBS += -L..\..\lame-3.97\mpglib\Debug -lmpglib
LIBS += -L..\..\libmad-0.15.1b\msvc++\Debug -llibmad
LIBS += -L..\..\portaudio\build\msvc\Debug_x86 -lportaudio_x86
LIBS += -L..\..\speex\lib -llibspeex
LIBS += -L..\..\speex\lib -llibspeexdsp
LIBS += -L..\..\libsamplerate-0.1.2 -llibsamplerate
LIBS += -L..\..\OpenSSL\lib\VC -llibeay32MT
LIBS += -L..\..\libogg-1.1.3\win32\Static_Release -logg_static
LIBS += -L..\..\libvorbis-1.2.0\win32\Vorbis_Static_Release -lvorbis_static
LIBS += -L..\..\libvorbis-1.2.0\win32\VorbisEnc_Static_Release -lvorbisenc_static
LIBS += -L..\..\libvorbis-1.2.0\win32\VorbisFile_Static_Release -lvorbisfile_static
QMAKE_LFLAGS_DEBUG += /VERBOSE:LIB
QMAKE_LFLAGS_RELEASE += /VERBOSE:LIB

CONFIG += qt thread

