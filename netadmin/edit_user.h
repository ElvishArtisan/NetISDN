// edit_user.h
//
// Edit a NetISDN User
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: edit_user.h,v 1.2 2008/08/15 10:44:25 cvs Exp $
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

#ifndef EDIT_USER_H
#define EDIT_USER_H

#include <qdialog.h>
#include <qlistbox.h>
#include <qtextedit.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qsqldatabase.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qdatetimeedit.h>

class EditUser : public QDialog
{
  Q_OBJECT
 public:
  EditUser(const QString &user,QWidget *parent=0,const char *name=0);
  ~EditUser();
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void dateData();
  void passwordData();
  void okData();
  void cancelData();

 private:
  QLineEdit *user_name_edit;
  QLineEdit *user_full_name_edit;
  QLineEdit *user_phone_edit;
  QLineEdit *user_email_edit;
  QLineEdit *user_location_edit;
  QCheckBox *user_admin_box;
  QDateEdit *user_expiration_edit;
  QString user_password;
};


#endif  // EDIT_USER_H
