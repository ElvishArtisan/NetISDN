// historybox.h
//
// An editable QComboBox with history
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: historybox.h,v 1.2 2008/09/08 14:17:59 cvs Exp $
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

#ifndef HISTORYBOX_H
#define HISTORYBOX_H

#include <qcombobox.h>

#define HISTORYBOX_DEFAULT_MAX_ITEMS 10

class HistoryBox : public QComboBox
{
  Q_OBJECT;
 public:
  HistoryBox(QWidget *parent=0,const char *name=0);
  int maxItems() const;
  void setMaxItems(int size);
  void addToList(const QString &str="");

 private:
  int box_max_items;
};


#endif  // HISTORYBOX_H
