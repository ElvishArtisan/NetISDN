// sqldb.h
//
// Database driver with automatic reconnect
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//  Adapted from the database driver for Rivendell
//
//   (C) Copyright 2007 Dan Mills <dmills@exponent.myzen.co.uk>
//
//      $Id: sqldb.h,v 1.1 2008/11/03 17:17:00 cvs Exp $
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

#ifndef SQLDB_H
#define SQLDB_H

#include <qobject.h>
#include <qsqldatabase.h>
#include <qstring.h>

#include <netconfig.h>


class NetSqlDatabaseStatus : public QObject
{
  Q_OBJECT
 signals:
  void reconnected();
  void connectionFailed ();
 private:
  NetSqlDatabaseStatus ();
  bool discon;
  friend NetSqlDatabaseStatus * NetDbStatus();
 public:
  void sendRecon();
  void sendDiscon(QString query);
};


class NetSqlQuery : public QSqlQuery
{
 public:
 NetSqlQuery ( const QString & query = QString::null, QSqlDatabase * db = 0 );
};

// Setup the default database, returns true on success.
// if error is non NULL, an error string will be appended to it
// if there is a problem.
QSqlDatabase * NetInitDb (QString *error=NULL);
void NetFreeDb();

// Return a handle to the database status object.
NetSqlDatabaseStatus * NetDbStatus();

#endif  // SQLDB_H
