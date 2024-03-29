// datedialog.cpp
//
// A Dialog Box for using an DatePicker widget.
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: datedialog.cpp,v 1.1 2008/08/15 10:44:25 cvs Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Library General Public License 
//   version 2 as published by the Free Software Foundation.
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


#include <qapplication.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qrect.h>
#include <qpoint.h>
#include <qpainter.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qsignalmapper.h>

#include <netconf.h>
#include <datedialog.h>

//
// Global Classes
//
DateDialog::DateDialog(int low_year,int high_year,
			 QWidget *parent,const char *name)
  :QDialog(parent,name,true)
{
  QFont font;

  font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);

  setCaption(tr("Select Date"));

  //
  // Datepicker
  //
  date_picker=new DatePicker(low_year,high_year,this,"date_picker");
  date_picker->setGeometry(10,10,
			   date_picker->sizeHint().width(),
			   date_picker->sizeHint().height());

  //
  // OK Button
  //
  QPushButton *button=new QPushButton(this,"ok_button");
  button->setGeometry(sizeHint().width()-130,sizeHint().height()-40,50,30);
  button->setFont(font);
  button->setText(tr("&OK"));
  connect(button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  // Cancel Button
  //
  button=new QPushButton(this,"cancel_button");
  button->setGeometry(sizeHint().width()-65,sizeHint().height()-40,55,30);
  button->setFont(font);
  button->setText(tr("&Cancel"));
  connect(button,SIGNAL(clicked()),this,SLOT(cancelData()));
}


DateDialog::~DateDialog()
{
}


QSize DateDialog::sizeHint() const
{
  return QSize(date_picker->sizeHint().width()+20,
	       date_picker->sizeHint().height()+60);
}


QSizePolicy DateDialog::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


int DateDialog::exec(QDate *date)
{
  date_date=date;
  date_picker->setDate(*date_date);
  return QDialog::exec();
}


void DateDialog::okData()
{
  *date_date=date_picker->date();
  done(0);
}


void DateDialog::cancelData()
{
  done(-1);
}
