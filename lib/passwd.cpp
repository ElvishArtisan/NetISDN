// passwd.cpp
//
// Set Password widget for NetISDN.
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: passwd.cpp,v 1.1 2008/08/09 01:58:06 cvs Exp $
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

#include <qdialog.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qbuttongroup.h>
#include <math.h>

#include <passwd.h>


Passwd::Passwd(QString *password,QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  //
  // Generate Fonts
  //
  QFont label_font=QFont("Helvetica",12,QFont::Bold);
  label_font.setPixelSize(12);
  QFont button_font=QFont("Helvetica",14,QFont::Bold);
  button_font.setPixelSize(14);

  passwd_password=password;
  setCaption(tr("Change Password"));

  //
  // OK Button
  //
  QPushButton *ok_button=new QPushButton(this,"ok_button");
  ok_button->setGeometry(10,60,100,55);
  ok_button->setFont(button_font);
  ok_button->setText(tr("&OK"));
  ok_button->setDefault(true);
  connect(ok_button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  // Cancel Button
  //
  QPushButton *cancel_button=new QPushButton(this,"cancel_button");
  cancel_button->setGeometry(120,60,100,55);
  cancel_button->setFont(button_font);
  cancel_button->setText(tr("&Cancel"));
  connect(cancel_button,SIGNAL(clicked()),this,SLOT(cancelData()));

  //
  // Password
  //
  passwd_password_1_edit=new QLineEdit(this,"passwd_password_1_edit");
  passwd_password_1_edit->setGeometry(90,11,100,19);
  passwd_password_1_edit->setMaxLength(16);
  passwd_password_1_edit->setEchoMode(QLineEdit::Password);
  passwd_password_1_edit->setFocus();
  QLabel *passwd_password_1_label=new QLabel(passwd_password_1_edit,
					     tr("&Password:"),this,
					     "passwd_password_1_label");
  passwd_password_1_label->setFont(label_font);
  passwd_password_1_label->setGeometry(10,13,75,19);
  passwd_password_1_label->setAlignment(AlignRight|ShowPrefix);

  //
  // Confirm Password
  //
  passwd_password_2_edit=new QLineEdit(this,"passwd_password_2_edit");
  passwd_password_2_edit->setGeometry(90,32,100,19);
  passwd_password_2_edit->setMaxLength(16);
  passwd_password_2_edit->setEchoMode(QLineEdit::Password);
  QLabel *passwd_password_2_label=
    new QLabel(passwd_password_2_edit,tr("C&onfirm:"),this,
	       "passwd_password_2_label");
  passwd_password_2_label->setFont(label_font);
  passwd_password_2_label->setGeometry(10,34,75,19);
  passwd_password_2_label->setAlignment(AlignRight|ShowPrefix);

}


Passwd::~Passwd()
{
  delete passwd_password_1_edit;
  delete passwd_password_2_edit;
}


QSize Passwd::sizeHint() const
{
  return QSize(230,125);
} 


QSizePolicy Passwd::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void Passwd::okData()
{
  if(passwd_password_1_edit->text()==passwd_password_2_edit->text()) {
    *passwd_password=passwd_password_1_edit->text();
    done(0);
  }
  else {
    QMessageBox::warning(this,tr("Password Mismatch"),
			 tr("The passwords don't match,\nplease try again!"),
			 tr("OK"),0);
  }
}


void Passwd::cancelData()
{
  done(1);
}
