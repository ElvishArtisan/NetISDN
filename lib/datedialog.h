// datedialog.h
//
// A Dialog Box for using an DatePicker widget.
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: datedialog.h,v 1.1 2008/08/15 10:44:25 cvs Exp $
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


#ifndef DATEDIALOG_H
#define DATEDIALOG_H

#include <qdialog.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qlabel.h>

#include <datepicker.h>


class DateDialog : public QDialog
{
 Q_OBJECT
 public:
  DateDialog(int low_year,int high_year,QWidget *parent=0,
	       const char *name=0);
  ~DateDialog();
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 public slots:
  int exec(QDate *date);

 private slots:
  void okData();
  void cancelData();

 private:
  DatePicker *date_picker;
  QDate *date_date;
};


#endif  // DATEDIALOG_H
