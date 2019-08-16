// version.cpp
//
// Get / Set Version Numbers of NetISDN Components
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: version.cpp,v 1.1 2008/08/09 01:58:06 cvs Exp $
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

#include <version.h>


//
// Global Classes
//
Version::Version()
{
}


int Version::database()
{
  int ver;

  QSqlQuery *q=new QSqlQuery("select DB from VERSION");
  if(!q->first()) {
    delete q;
    return 0;
  }
  ver=q->value(0).toInt();
  delete q;
  return ver;
}


void Version::setDatabase(int ver)
{
  QSqlQuery *q;
  QString sql;

  sql=QString().sprintf("update VERSION set DB=%d",ver);
  q=new QSqlQuery(sql);
  delete q;
}
