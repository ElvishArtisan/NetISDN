// passwd.h
//
// Set Password Widget for NetISDN.
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: passwd.h,v 1.1 2008/08/09 01:58:06 cvs Exp $
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

#ifndef PASSWD_H
#define PASSWD_H

#include <qdialog.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <qpixmap.h>
#include <qradiobutton.h>


class Passwd : public QDialog
{
  Q_OBJECT
  public:
   Passwd(QString *password,QWidget *parent=0,const char *name=0);
   ~Passwd();
   QSize sizeHint() const;
   QSizePolicy sizePolicy() const;

  protected:

  private slots:
   void okData();
   void cancelData();

  private:
   QLineEdit *passwd_password_1_edit;
   QLineEdit *passwd_password_2_edit;
   QString *passwd_password;
};


#endif  // PASSWD_H
