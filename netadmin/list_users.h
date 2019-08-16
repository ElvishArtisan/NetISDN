// list_users.h
//
// List NetISDN Users
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: list_users.h,v 1.4 2009/01/26 16:26:30 pcvs Exp $
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

#ifndef LIST_USERS_H
#define LIST_USERS_H

#include <qdialog.h>
#include <qlistview.h>
#include <qsqldatabase.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <netlistviewitem.h>

class ListUsers : public QDialog
{
  Q_OBJECT
 public:
  ListUsers(QWidget *parent=0,const char *name=0);
  ~ListUsers();
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void addData();
  void editData();
  void deleteData();
  void doubleClickedData(QListViewItem *item,const QPoint &pt,int col);
  void onlineToggledData(bool state);
  void userUpdateData();
  void closeData();

 protected:
  void resizeEvent(QResizeEvent *e);

 private:
  void RefreshItem(NetListViewItem *item);
  void RefreshItem(int uid);
  void RefreshList();
  QSqlDatabase *list_db;
  QLabel *list_online_label;
  QCheckBox *list_online_check;
  QListView *list_view;
  QString list_current_user;
  QPushButton *list_add_button;
  QPushButton *list_edit_button;
  QPushButton *list_delete_button;
  QPushButton *list_close_button;
  QPixmap *list_admin_map;
  QPixmap *list_user_map;
  QPixmap *list_greenball_map;
  QPixmap *list_redball_map;
  QPixmap *list_whiteball_map;
};


#endif  // LIST_USERS_H
