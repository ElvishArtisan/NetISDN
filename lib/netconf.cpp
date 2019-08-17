// netconf.c
// A small library for handling common configuration file tasks
// 
// Adopted from conflib
//
//   (C) Copyright 1996-2003 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: netconf.cpp,v 1.6 2008/11/03 18:54:33 cvs Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <qhostaddress.h>
#include <qvariant.h>
#include <qmessagebox.h>
#include <qdir.h>
#ifdef WIN32
#include <win32_types.h>
#else
#include <unistd.h>
#include <netdb.h>
#include <sys/timex.h>
#include <time.h>
#endif
#include <netconf.h>
#include <escape_string.h>

#define BUFFER_SIZE 1024


bool NetBool(QString string)
{
  if(string.contains("Y",false)) {
    return true;
  }
  return false;
}


QString NetYesNo(bool state)
{
  if(state) {
    return QString("Y");
  }
  return QString("N");
}


QString NetGetTimeLength(int mseconds,bool leadzero,bool tenths)
{
  int hour,min,seconds,tenthsecs;
  char negative[2];

  if(mseconds<0) {
    mseconds=-mseconds;
    strcpy(negative,"-");
  }
  else {
    negative[0]=0;
  }
  QTime time_length(QTime(0,0,0).addMSecs(mseconds));
  hour = time_length.hour();
  min = time_length.minute();
  seconds = time_length.second();
  mseconds = time_length.msec();
  tenthsecs=mseconds/100;
  if(leadzero) {
    if(tenths) {
     return QString().sprintf("%s%d:%02d:%02d.%d",negative,hour,min,seconds,
			      tenthsecs);
    }
    return QString().sprintf("%s%d:%02d:%02d",negative,hour,min,seconds);
  }
  if((hour==0)&&(min==0)) {
    if(tenths) {
      return QString().sprintf("%s:%02d.%d",negative,seconds,tenthsecs);
    }
    return QString().sprintf("%s:%02d",negative,seconds);
  }
  if(hour==0) {
    if(tenths) {
      return QString().sprintf("%s%2d:%02d.%d",negative,min,seconds,
			       tenthsecs);
    }
    return QString().sprintf("%s%2d:%02d",negative,min,seconds);
  }
  if(tenths) {
    return QString().sprintf("%s%2d:%02d:%02d.%d",negative,hour,min,seconds,
			     tenthsecs);
  }
  return QString().sprintf("%s%2d:%02d:%02d",negative,hour,min,seconds);
}


int NetSetTimeLength(const QString &str)
{
  int istate=2;
  QString field;
  int res=0;

  if(str.isEmpty()) {
    return -1;
  }
  for(unsigned i=0;i<str.length();i++) {
    if(str.at(i)==':') {
      istate--;
    }
  }
  if(istate<0) {
    return -1;
  }
  for(unsigned i=0;i<str.length();i++) {
    if(str.at(i).isNumber()) {
      field+=str.at(i);
    }
    else {
      if((str.at(i)==':')||(str.at(i)=='.')) {
	if(field.length()>2) {
	  return -1;
	}
	switch(istate) {
	    case 0:
	      res+=3600000*field.toInt();
	      break;

	    case 1:
	      res+=60000*field.toInt();
	      break;

	    case 2:
	      res+=1000*field.toInt();
	      break;
	}
	istate++;
	field="";
      }
      else {
	if(!str.at(i).isSpace()) {
	  return -2;
	}
      }
    }
  }
  switch(istate) {
      case 2:
	res+=1000*field.toInt();
	break;

      case 3:
	switch(field.length()) {
	    case 1:
	      res+=100*field.toInt();
	      break;

	    case 2:
	      res+=10*field.toInt();
	      break;

	    case 3:
	      res+=field.toInt();
	      break;
	}
  }

  return res;
}


#ifndef WIN32
bool NetWritePid(const QString &dirname,const QString &filename,int owner,
		int group)
{
  FILE *file;
  mode_t prev_mask;
  QString pathname=QString().sprintf("%s/%s",
				     (const char *)dirname,
				     (const char *)filename);

  prev_mask = umask(0113);      // Set umask so pid files are user and group writable.
  file=fopen((const char *)pathname,"w");
  umask(prev_mask);
  if(file==NULL) {
    return false;
  }
  fprintf(file,"%d",getpid());
  fclose(file);
  chown((const char *)pathname,owner,group);

  return true;
}


void NetDeletePid(const QString &dirname,const QString &filename)
{
  QString pid=QString().sprintf("%s/%s",
				(const char *)dirname,
				(const char *)filename);
  unlink((const char *)pid);
}


bool NetCheckPid(const QString &dirname,const QString &filename)
{
  QDir dir;
  QString path;
  path=QString("/proc/")+
    QString().sprintf("%d",NetGetPid(dirname+QString("/")+filename));
  dir.setPath(path);
  return dir.exists();
}


pid_t NetGetPid(const QString &pidfile)
{
  FILE *handle;
  pid_t ret;

  if((handle=fopen((const char *)pidfile,"r"))==NULL) {
    return -1;
  }
  if(fscanf(handle,"%d",&ret)!=1) {
    ret=-1;
  }
  fclose(handle);
  return ret;
}
#endif  // WIN32


QString NetGetHomeDir(bool *found)
{
  if(getenv("HOME")==NULL) {
    if(found!=NULL) {
      *found=false;
    }
    return QString("/");
  }
  if(found!=NULL) {
    *found=true;
  }
  return QString(getenv("HOME"));
}


QString NetTruncateAfterWord(QString str,int word,bool add_dots)
{
  QString simple=str.simplifyWhiteSpace();
  int quan=0;
  int point;

  for(unsigned i=0;i<simple.length();i++) {
    if(simple.at(i).isSpace()) {
      quan++;
      point=i;
      if(quan==word) {
	if(add_dots) {
	  return simple.left(point)+QString("...");
	}
	else {
	  return simple.left(point);
	}
      }
    }
  }
  return simple;
}


QString NetHomeDir()
{
  if(getenv("HOME")==NULL) {
    return QString("/");
  }
  return QString(getenv("HOME"));
}


QString NetTempDir()
{
#ifdef WIN32
  if(getenv("TEMP")!=NULL) {
    return QString(getenv("TEMP"));
  }
  if(getenv("TMP")!=NULL) {
    return QString(getenv("TMP"));
  }
  return QString("C:\\");
#else
  if(getenv("TMPDIR")!=NULL) {
    return QString(getenv("TMPDIR"));
  }
  return QString("/tmp");
#endif  // WIN32
}


#ifndef WIN32
QString NetTimeZoneName(const QDateTime &datetime)
{
  char name[20];
  time_t time=datetime.toTime_t();
  strftime(name,20,"%Z",localtime(&time));
  return QString(name);
}
#endif  // WIN32

char base64_dict[64]={'A','B','C','D','E','F','G','H','I','J','K','L','M','N',
		      'O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b',
		      'c','d','e','f','g','h','i','j','k','l','m','n','o','p',
		      'q','r','s','t','u','v','w','x','y','z','0','1','2','3',
		      '4','5','6','7','8','9','+','/'};

QString NetStringToBase64(const QString str)
{
  QString ret;
  uint32_t buf;

  //
  // Build Groups
  //
  for(unsigned i=0;i<str.length();i+=3) {
    buf=(0xff&str.at(i))<<16;
    if((i+1)<str.length()) {
      buf|=(0xff&str.at(i+1))<<8;
      if((i+2)<str.length()) {
	buf|=0xff&str.at(i+2);
      }
    }

    //
    // Dictionary Lookup
    //
    for(int i=3;i>=0;i--) {
      ret+=base64_dict[0x3f&(buf>>(6*i))];
    }
  }

  //
  // Apply Padding
  //
  switch(str.length()%3) {
    case 1:
      ret=ret.left(ret.length()-2)+"==";
      break;
      
    case 2:
      ret=ret.left(ret.length()-1)+"=";
      break;
  }


  return ret;
}


QString NetBase64ToString(const QString str,bool *ok)
{
  QString ret;
  uint32_t buf=0;
  char c;
  int pad=0;
  bool found=false;

  //
  // Set Status
  //
  if(ok!=NULL) {
    *ok=true;
  }

  //
  // Dictionary Lookup
  //
  for(unsigned i=0;i<str.length();i+=4) {
    buf=0;
    for(int j=0;j<4;j++) {
      if(((i+j)<str.length())&&(c=str.at(i+j))!='=') {
	found=false;
	for(unsigned k=0;k<64;k++) {
	  if(base64_dict[k]==c) {
	    buf|=(k<<(6*(3-j)));
	    found=true;
	  }
	}
	if(!found) {  // Illegal character!
	  if(ok!=NULL) {
	    *ok=false;
	  }
	  return QString();
	}
      }
      else {
	if(str.at(i+j)=='=') {
	  pad++;
	}
      }
    }

    //
    // Extract Groups
    //
    for(int i=2;i>=0;i--) {
      ret+=(0xff&(buf>>(8*i)));
    }
  }

  //
  // Apply Padding
  //
  if(pad>2) {
    if(ok!=NULL) {
      *ok=false;
    }
    return QString();
  }
  ret=ret.left(ret.length()-pad);

  return ret;
}


QString NetDataToBase64(const unsigned char *data,unsigned len)
{
  QString ret;
  uint32_t buf;

  //
  // Build Groups
  //
  for(unsigned i=0;i<len;i+=3) {
    buf=(0xff&data[i])<<16;
    if((i+1)<len) {
      buf|=(0xff&data[i+1])<<8;
      if((i+2)<len) {
	buf|=0xff&data[i+2];
      }
    }

    //
    // Dictionary Lookup
    //
    for(int i=3;i>=0;i--) {
      ret+=base64_dict[0x3f&(buf>>(6*i))];
    }
  }

  //
  // Apply Padding
  //
  switch(len%3) {
    case 1:
      ret=ret.left(ret.length()-2)+"==";
      break;
      
    case 2:
      ret=ret.left(ret.length()-1)+"=";
      break;
  }


  return ret;
}


unsigned NetBase64ToData(const QString &base64,unsigned char *data,
			 unsigned maxlen)
{
  uint32_t buf=0;
  unsigned ptr=0;
  unsigned char c;
  int pad=0;
  bool found=false;

  //
  // Dictionary Lookup
  //
  for(unsigned i=0;i<base64.length();i+=4) {
    buf=0;
    for(int j=0;j<4;j++) {
      if(((i+j)<base64.length())&&(c=base64.at(i+j))!='=') {
	found=false;
	for(unsigned k=0;k<64;k++) {
	  if(base64_dict[k]==c) {
	    buf|=(k<<(6*(3-j)));
	    found=true;
	  }
	}
	if(!found) {  // Illegal character!
	  return 0;
	}
      }
      else {
	if(base64.at(i+j)=='=') {
	  pad++;
	}
      }
    }

    //
    // Extract Groups
    //
    for(int i=2;i>=0;i--) {
      data[ptr++]=(0xff&(buf>>(8*i)));
    }
  }

  //
  // Apply Padding
  //
  if(pad>2) {
    return 0;
  }
  ptr-=pad;

  return ptr;
}
