// netconf.h
//
// The header file for the rconf package
//
//   (C) Copyright 1996-2004 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: netconf.h,v 1.4 2008/11/03 17:17:00 cvs Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Library General Public License 
//   version 2 as published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#ifndef NETCONF_H
#define NETCONF_H

#define MAX_RETRIES 10
#include <stdlib.h>
#include <stdio.h>

#include <qstring.h>
#include <qdatetime.h>
#include <qfont.h>
#include <qhostaddress.h>
#include <qsqldatabase.h>
#include <qvariant.h>
#include <qdatetime.h>


bool NetBool(QString);
QString NetYesNo(bool);
QString NetGetTimeLength(int mseconds,bool leadzero=false,bool tenths=true);
int NetSetTimeLength(const QString &string);
#ifndef WIN32
bool NetWritePid(const QString &dirname,const QString &filename,int owner=-1,
		int group=-1);
void NetDeletePid(const QString &dirname,const QString &filename);
bool NetCheckPid(const QString &dirname,const QString &filename);
pid_t NetGetPid(const QString &pidfile);
#endif  // WIN32
QString NetTruncateAfterWord(QString str,int word,bool add_dots=false);
QString NetGetHomeDir(bool *found=NULL);
QString NetHomeDir();
QString NetTempDir();
QString NetTimeZoneName(const QDateTime &datetime);
QString NetStringToBase64(const QString str);
QString NetBase64ToString(const QString str,bool *ok=NULL);
QString NetDataToBase64(const unsigned char *data,unsigned len);
unsigned NetBase64ToData(const QString &base64,unsigned char *data,
			 unsigned maxlen);

#endif   // NETCONF_H
