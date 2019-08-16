// createdb.cpp
//
// Create, Initialize and/or Update a NetISDN Database
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: createdb.cpp,v 1.8 2008/10/13 13:53:42 cvs Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
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

#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>

#include <qsqldatabase.h>

#include <dbversion.h>
#include <netconfig.h>
#include <createdb.h>

bool RunQuery(QString sql)
{
  QSqlQuery *q=new QSqlQuery(sql);
  if(!q->isActive()) {
    fprintf(stderr,"SQL: %s\n",(const char *)sql);
    fprintf(stderr,"SQL Error: %s\n",(const char *)q->lastError().databaseText());
    delete q;
    return false;
  }
  delete q;
  return true;
}


bool CreateDb(QString name,QString pwd)
{
  QString sql;

//
// Create USERS table
//
  sql=QString("create table if not exists USERS (\
    ID int not null primary key auto_increment,\
    NAME char(64) not null unique,\
    FULL_NAME char(64),\
    PHONE_NUMBER char(64),\
    EMAIL char(255),\
    LOCATION char(255),\
    PASSWORD char(64),\
    ADMIN_PRIV enum('N','Y') default 'N',\
    ONLINE enum('N','Y') default 'N',\
    EXPIRATION_DATE date,\
    LAST_ACCESS_DATETIME datetime,\
    LAST_ACCESS_ADDR char(16),\
    CLIENT_VERSION char(32),\
    index NAME_IDX(NAME))");
  if(!RunQuery(sql)) {
    return false;
  }

//
// Create VERSION table
//
  sql="create table if not exists VERSION (\
       DB int not null primary key)";
  if(!RunQuery(sql)) {
    return false;
  }

//
// Create CALLS table
//
  sql="create table if not exists CALLS (\
       ID int not null auto_increment primary key,\
       ORIG_UID int not null,\
       ORIG_ADDR char(16) not null,\
       ORIG_PORT int unsigned not null,\
       DEST_UID int not null,\
       DEST_ADDR char(16) not null,\
       DEST_PORT int unsigned not null,\
       START_DATETIME datetime,\
       END_DATETIME datetime,\
       FORMAT int unsigned,\
       CHANNELS int unsigned,\
       SAMPLE_RATE int unsigned,\
       BIT_RATE int unsigned,\
       DEST_FORMAT int unsigned,\
       DEST_CHANNELS int unsigned,\
       DEST_SAMPLE_RATE int unsigned,\
       DEST_BIT_RATE int unsigned,\
       index ORIG_UID_IDX(ORIG_UID,START_DATETIME))";
  if(!RunQuery(sql)) {
    return false;
  }

  return true;
}


bool InitDb(QString name,QString pwd)
{
  QString sql;

  //
  // Create Default Admin Account
  //
  sql=QString().sprintf("insert into USERS (NAME,PASSWORD,FULL_NAME,\
                         ADMIN_PRIV,EXPIRATION_DATE)\
                         values (\"%s\",\"%s\",\"%s\",\"Y\",now())",
			DEFAULT_LOGIN_NAME,
			DEFAULT_PASSWORD,
			DEFAULT_FULLNAME);
  if(!RunQuery(sql)) {
    return false;
  }

  //
  // Generate Version Number
  //
  sql=QString().sprintf("insert into VERSION (DB) values (%d)",
			NET_VERSION_DATABASE);
  if(!RunQuery(sql)) {
    return false;
  }

  return true;
}


bool UpdateDb(int ver)
{
  QString sql;
  QSqlQuery *q;

  // **** Start of version updates ****

  if(ver<2) {
    sql="alter table USERS add column ONLINE enum('N','Y') default 'N' \
         after ADMIN_PRIV";
    RunQuery(sql);
  }

  if(ver<3) {
    sql="create table if not exists CALLS (\
         ID int not null auto_increment primary key,\
         ORIG_UID int not null,\
         ORIG_ADDR char(16) not null,\
         DEST_UID int not null,\
         DEST_ADDR char(16) not null,\
         START_DATETIME datetime,\
         END_DATETIME datetime,\
         FORMAT int unsigned,\
         CHANNELS int unsigned,\
         SAMPLE_RATE int unsigned,\
         BIT_RATE int unsigned,\
         index ORIG_UID_IDX(ORIG_UID,START_DATETIME))";
    if(!RunQuery(sql)) {
      return false;
    }
  }

  if(ver<4) {
    sql="alter table USERS add column CLIENT_VERSION char(32) \
         after LAST_ACCESS_ADDR";
    if(!RunQuery(sql)) {
      return false;
    }
  }

  if(ver<5) {
    sql="alter table CALLS add column ORIG_PORT int unsigned not null \
         after ORIG_ADDR";
    if(!RunQuery(sql)) {
      return false;
    }

    sql="alter table CALLS add column DEST_PORT int unsigned not null \
         after DEST_ADDR";
    if(!RunQuery(sql)) {
      return false;
    }
  }

  if(ver<6) {
    sql="alter table CALLS add column DEST_FORMAT int unsigned \
         after BIT_RATE";
    if(!RunQuery(sql)) {
      return false;
    }
    sql="alter table CALLS add column DEST_CHANNELS int unsigned \
         after DEST_FORMAT";
    if(!RunQuery(sql)) {
      return false;
    }
    sql="alter table CALLS add column DEST_SAMPLE_RATE int unsigned \
         after DEST_CHANNELS";
    if(!RunQuery(sql)) {
      return false;
    }
    sql="alter table CALLS add column DEST_BIT_RATE int unsigned \
         after DEST_SAMPLE_RATE";
    if(!RunQuery(sql)) {
      return false;
    }
  }


  // **** End of version updates ****
  
  //
  // Update Version Field
  //
  q=new QSqlQuery(QString().sprintf("update VERSION set DB=%d",
				    NET_VERSION_DATABASE));
  delete q;
  return true;
}
