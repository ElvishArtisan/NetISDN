// netadmin.h
//
// The Administration Utility for NetISDN.
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: netadmin.h,v 1.3 2009/01/26 13:05:20 pcvs Exp $
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


#ifndef NETADMIN_H
#define NETADMIN_H

#include <qwidget.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qsqldatabase.h>
#include <qpixmap.h>
#include <qsocketdevice.h>

#include <netconfig.h>

#define NETADMIN_USAGE "\n"

class MainWidget : public QWidget
{
  Q_OBJECT
 public:
  MainWidget(QWidget *parent=0,const char *name=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;
  
 private slots:
  void manageUsersData();
  void listCallsData();
  void backupData();
  void restoreData();
  void quitMainWidget();
  
 private:
  void ClearTables();
  QString admin_username;
  QString admin_password;
  QPixmap *admin_netisdn_map;
  QString admin_filter;
  QString admin_group;
};


#endif 
