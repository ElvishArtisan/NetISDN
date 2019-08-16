// edit_user.cpp
//
// Edit a NetISDN User
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: edit_user.cpp,v 1.3 2008/10/03 15:21:08 cvs Exp $
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
#include <qlistbox.h>
#include <qtextedit.h>
#include <qpainter.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qsqldatabase.h>

#include <passwd.h>
#include <netconf.h>
#include <edit_user.h>
#include <escape_string.h>
#include <datedialog.h>

EditUser::EditUser(const QString &user,QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  QString sql;
  QSqlQuery *q;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  setCaption(tr("User: ")+user);

  //
  // Create Fonts
  //
  QFont font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);
  QFont select_font=QFont("Helvetica",12,QFont::Normal);
  select_font.setPixelSize(12);

  //
  // User Name
  //
  user_name_edit=new QLineEdit(this,"user_name_edit");
  user_name_edit->setGeometry(100,11,sizeHint().width()-110,19);
  user_name_edit->setMaxLength(64);
  user_name_edit->setReadOnly(true);
  QLabel *user_name_label=new QLabel(user_name_edit,tr("&User Name:"),this,
				       "user_name_label");
  user_name_label->setGeometry(5,11,90,19);
  user_name_label->setFont(font);
  user_name_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Full Name
  //
  user_full_name_edit=new QLineEdit(this,"user_full_name_edit");
  user_full_name_edit->setGeometry(100,32,sizeHint().width()-110,19);
  user_full_name_edit->setMaxLength(64);
  QLabel *user_full_name_label=
    new QLabel(user_full_name_edit,tr("&Full Name:"),this,
	       "user_full_name_label");
  user_full_name_label->setGeometry(10,32,85,19);
  user_full_name_label->setFont(font);
  user_full_name_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // User Phone
  //
  user_phone_edit=new QLineEdit(this,"user_phone_edit");
  user_phone_edit->setGeometry(100,53,sizeHint().width()-110,19);
  user_phone_edit->setMaxLength(255);
  QLabel *user_phone_label=
    new QLabel(user_phone_edit,tr("&Phone:"),this,"user_phone_label");
  user_phone_label->setGeometry(10,53,85,19);
  user_phone_label->setFont(font);
  user_phone_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // User E-Mail Address
  //
  user_email_edit=new QLineEdit(this,"user_email_edit");
  user_email_edit->setGeometry(100,75,sizeHint().width()-110,19);
  user_email_edit->setMaxLength(255);
  QLabel *user_email_label=
    new QLabel(user_email_edit,tr("&Email:"),this,"user_email_label");
  user_email_label->setGeometry(10,75,85,19);
  user_email_label->setFont(font);
  user_email_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // User Location
  //
  user_location_edit=new QLineEdit(this,"user_location_edit");
  user_location_edit->setGeometry(100,97,sizeHint().width()-110,19);
  user_location_edit->setMaxLength(255);
  user_location_edit->setReadOnly(true);
  QLabel *user_location_label=
    new QLabel(user_location_edit,tr("&Location:"),this,"user_location_label");
  user_location_label->setGeometry(10,97,85,19);
  user_location_label->setFont(font);
  user_location_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Administrator Rights
  //
  user_admin_box=new QCheckBox(this,"user_web_box");
  user_admin_box->setGeometry(100,119,15,15);
  QLabel *label=
    new QLabel(user_admin_box,tr("User has admin rights"),this,"user_admin_label");
  label->setGeometry(120,119,180,19);
  label->setFont(font);
  label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

  //
  // Expiration Date
  //
  user_expiration_edit=new QDateEdit(this,"user_expiration_edit");
  user_expiration_edit->setGeometry(100,144,100,19);
  QLabel *user_expiration_label=
    new QLabel(user_expiration_edit,tr("&Expires:"),this,"user_expiration_label");
  user_expiration_label->setGeometry(10,144,85,19);
  user_expiration_label->setFont(font);
  user_expiration_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  //  Select Date Button
  //
  QPushButton *button=new QPushButton(this,"select_button");
  button->setGeometry(210,139,60,28);
  button->setFont(select_font);
  button->setText(tr("Select"));
  connect(button,SIGNAL(clicked()),this,SLOT(dateData()));

  //
  // Change Password Button
  //
  QPushButton *password_button=new QPushButton(this,"password_button");
  password_button->setGeometry(sizeHint().width()-90,119,80,50);
  password_button->setFont(font);
  password_button->setText(tr("Change\n&Password"));
  connect(password_button,SIGNAL(clicked()),this,SLOT(passwordData()));

  //
  //  Ok Button
  //
  button=new QPushButton(this,"ok_button");
  button->setGeometry(sizeHint().width()-180,sizeHint().height()-60,80,50);
  button->setDefault(true);
  button->setFont(font);
  button->setText(tr("&OK"));
  connect(button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  //  Cancel Button
  //
  button=new QPushButton(this,"cancel_button");
  button->setGeometry(sizeHint().width()-90,sizeHint().height()-60,
			     80,50);
  button->setFont(font);
  button->setText(tr("&Cancel"));
  connect(button,SIGNAL(clicked()),this,SLOT(cancelData()));

  //
  // Populate Fields
  //
  sql=QString().sprintf("select FULL_NAME,PHONE_NUMBER,EMAIL,LOCATION,\
                         ADMIN_PRIV,PASSWORD,EXPIRATION_DATE \
                         from USERS where NAME=\"%s\"",
			(const char *)EscapeString(user));
  q=new QSqlQuery(sql);
  if(q->first()) {
    user_name_edit->setText(user);
    user_full_name_edit->setText(q->value(0).toString());
    user_phone_edit->setText(q->value(1).toString());
    user_email_edit->setText(q->value(2).toString());
    user_location_edit->setText(q->value(3).toString());
    user_admin_box->setChecked(NetBool(q->value(4).toString()));
    user_password=q->value(5).toString();
    user_expiration_edit->setDate(q->value(6).toDate());
  }
  delete q;
}


EditUser::~EditUser()
{
  delete user_name_edit;
  delete user_full_name_edit;
  delete user_phone_edit;
  delete user_email_edit;
  delete user_admin_box;
}


QSize EditUser::sizeHint() const
{
  return QSize(375,250);
} 


QSizePolicy EditUser::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void EditUser::dateData()
{
  QDate date=user_expiration_edit->date();
  DateDialog *d=new DateDialog(2008,2099,this);
  if(d->exec(&date)==0) {
    user_expiration_edit->setDate(date);
  }
  delete d;
}


void EditUser::passwordData()
{
  Passwd *passwd=new Passwd(&user_password,this,"passwd");
  passwd->exec();
  delete passwd;
}


void EditUser::okData()
{
  QString sql;
  QSqlQuery *q;

  sql=QString().sprintf("update USERS set \
                         FULL_NAME=\"%s\",\
                         PHONE_NUMBER=\"%s\",\
                         EMAIL=\"%s\",\
                         PASSWORD=\"%s\",\
                         ADMIN_PRIV=\"%s\",\
                         EXPIRATION_DATE=\"%s\" \
                         where NAME=\"%s\"",
			(const char *)EscapeString(user_full_name_edit->text()),
			(const char *)EscapeString(user_phone_edit->text()),
			(const char *)EscapeString(user_email_edit->text()),
			(const char *)EscapeString(user_password),
			(const char *)EscapeString(NetYesNo(user_admin_box->
							    isChecked())),
			(const char *)user_expiration_edit->date().
			toString("yyyy-MM-dd"),
			(const char *)EscapeString(user_name_edit->text()));
  q=new QSqlQuery(sql);
  delete q;

  done(0);
}


void EditUser::cancelData()
{
  done(-1);
}
