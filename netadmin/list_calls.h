// list_calls.h
//
// List NetISDN Calls
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: list_calls.h,v 1.2 2009/01/29 15:31:08 pcvs Exp $
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

#ifndef LIST_CALLS_H
#define LIST_CALLS_H

#include <qdialog.h>
#include <qlistview.h>
#include <qsqldatabase.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <netlistviewitem.h>

class ListCalls : public QDialog
{
  Q_OBJECT
 public:
  ListCalls(QWidget *parent=0,const char *name=0);
  ~ListCalls();
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void callUpdateData();
  void activeToggledData(bool state);
  void closeData();

 protected:
  void resizeEvent(QResizeEvent *e);

 private:
  void RefreshItem(int cid);
  void RefreshList();
  void SetItem(QSqlQuery *q,NetListViewItem *item);
  QSqlDatabase *list_db;
  QListView *list_view;
  QPushButton *list_close_button;
  QPixmap *list_call_map;
  QPixmap *list_active_call_map;
  QLabel *list_active_label;
  QCheckBox *list_active_check;
};


#endif  // LIST_CALLS_H
