// sqldb.cpp
//
//   Database driver with automatic reconnect
//
//   (C) Copyright 2007 Dan Mills <dmills@exponent.myzen.co.uk>
//
//      $Id: sqldb.cpp,v 1.2 2008/11/03 18:54:33 cvs Exp $
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

#ifndef WIN32
#include <syslog.h>
#endif  // WIN32

#include <qobject.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <qserversocket.h>
#include <qsqldatabase.h>
#include <qsqlerror.h>
#include <assert.h>

#include "sqldb.h"

static QSqlDatabase *db = NULL;
static NetSqlDatabaseStatus * dbStatus = NULL;

QSqlDatabase * NetInitDb (QString *error)
{
  NetConfig *cf = NetConfiguration();
  cf->load();
  assert (cf);
  if (!db){
    db=QSqlDatabase::addDatabase(cf->mysqlDriver());
    if(!db) {
      if (error){
	(*error) += QString(QObject::tr("Couldn't initialize QSql driver!"));
      }
      return NULL;
    }
    db->setDatabaseName(cf->mysqlDbname());
    db->setUserName(cf->mysqlUsername());
    db->setPassword(cf->mysqlPassword());
    db->setHostName(cf->mysqlHostname());
    if(!db->open()) {
      if (error){
	(*error) += QString(QObject::tr("Couldn't open mySQL connection!"));
      }
      db->removeDatabase(cf->mysqlDbname());
      db->close();
      return NULL;
    }
  }
  return db;
}


void NetFreeDb()
{
  db->removeDatabase(NetConfiguration()->mysqlDbname());
  db->close();
}


NetSqlQuery::NetSqlQuery (const QString &query, QSqlDatabase *dbase):
  QSqlQuery (query,dbase)
{
  // With any luck, by the time we get here, we have already done the biz...
  if (!isActive()){ //DB Offline?
    QSqlDatabase *ldb = QSqlDatabase::database();
    // Something went wrong with the DB, trying a reconnect
    ldb->removeDatabase(NetConfiguration()->mysqlDbname());
    ldb->close();
    db = NULL;
    NetInitDb();
    QSqlQuery::prepare(query);
    QSqlQuery::exec();
    if(NetDbStatus()){
      if(isActive()){
	NetDbStatus()->sendRecon();
      } else {
	NetDbStatus()->sendDiscon(query);
      }
    }
  } 
  else {
    NetDbStatus()->sendRecon();
  }
}


void NetSqlDatabaseStatus::sendRecon()
{
  if (discon){
    discon = false;
    emit reconnected();
#ifndef WIN32
    syslog(LOG_ERR,"Database connection restored.\n");
#endif  // WIN32
  }
}

void NetSqlDatabaseStatus::sendDiscon(QString query)
{
  if (!discon){
    emit connectionFailed();
#ifndef WIN32
    syslog(LOG_ERR,"Database connection failed: %s\n",(const char *)query);
#endif  // WIN32
    discon = true;
  }
}

NetSqlDatabaseStatus::NetSqlDatabaseStatus()
{
  discon = false;
}

NetSqlDatabaseStatus * NetDbStatus()
{
  if (!dbStatus){
    dbStatus = new NetSqlDatabaseStatus;
  }
  return dbStatus;
}
