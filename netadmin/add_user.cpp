// add_user.cpp
//
// Add a NetISDN User
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: add_user.cpp,v 1.1 2008/08/13 18:55:57 cvs Exp $
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

#include <math.h>

#include <qdialog.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>

#include <edit_user.h>
#include <add_user.h>
#include <passwd.h>


AddUser::AddUser(QString *username,QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  user_name=username;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  setCaption(tr("Add User"));

  //
  // Create Fonts
  //
  QFont font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);

  //
  // User Name
  //
  user_name_edit=new QLineEdit(this,"user_name_edit");
  user_name_edit->setGeometry(135,11,sizeHint().width()-145,19);
  user_name_edit->setMaxLength(64);
  QLabel *user_name_label=new QLabel(user_name_edit,tr("&New User Name:"),this,
				       "user_name_label");
  user_name_label->setGeometry(10,13,120,19);
  user_name_label->setFont(font);
  user_name_label->setAlignment(AlignRight|ShowPrefix);

  //
  //  Ok Button
  //
  QPushButton *ok_button=new QPushButton(this,"ok_button");
  ok_button->setGeometry(30,45,80,50);
  ok_button->setDefault(true);
  ok_button->setFont(font);
  ok_button->setText(tr("&OK"));
  connect(ok_button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  //  Cancel Button
  //
  QPushButton *cancel_button=new QPushButton(this,"cancel_button");
  cancel_button->setGeometry(120,45,80,50);
  cancel_button->setFont(font);
  cancel_button->setText(tr("&Cancel"));
  connect(cancel_button,SIGNAL(clicked()),this,SLOT(cancelData()));
}


AddUser::~AddUser()
{
  delete user_name_edit;
}


QSize AddUser::sizeHint() const
{
  return QSize(230,110);
} 


QSizePolicy AddUser::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void AddUser::okData()
{
  QSqlQuery *q;
  QString sql;

  if(user_name_edit->text().isEmpty()) {
    QMessageBox::warning(this,tr("Invalid Name"),tr("You must give the user a name!"));
    return;
  }

  sql=QString().sprintf("insert into USERS set NAME=\"%s\"",
			(const char *)user_name_edit->text());
  q=new QSqlQuery(sql);
  if(!q->isActive()) {
    QMessageBox::warning(this,tr("User Exists"),tr("User Already Exists!"),
			 1,0,0);
    delete q;
    return;
  }
  delete q;
  EditUser *user=new EditUser(user_name_edit->text(),this,"user");
  if(user->exec()<0) {
    sql=QString().sprintf("delete from USERS where LOGIN_NAME=\"%s\"",
			  (const char *)user_name_edit->text());
    q=new QSqlQuery(sql);
    delete q;
    delete user;
    done(-1);
    return;
  }
  delete user;
  *user_name=user_name_edit->text();
  done(0);
}


void AddUser::cancelData()
{
  done(-1);
}
