// historybox.cpp
//
// An editable QComboBox with history
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: historybox.cpp,v 1.2 2008/09/08 14:17:59 cvs Exp $
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

#include <historybox.h>


HistoryBox::HistoryBox(QWidget *parent,const char *name)
  : QComboBox(parent,name)
{
  box_max_items=HISTORYBOX_DEFAULT_MAX_ITEMS;
  setEditable(true);
}


int HistoryBox::maxItems() const
{
  return box_max_items;
}


void HistoryBox::setMaxItems(int size)
{
  box_max_items=size;
  for(int i=count()-1;i>box_max_items;i--) {
    removeItem(i);
  }
}


void HistoryBox::addToList(const QString &str)
{
  if(str.isEmpty()) {
    insertItem(currentText(),0);
  }
  else {
    insertItem(str,0);
  }
  for(int i=count()-1;i>0;i--) {
    if(text(i)==currentText()) {
      removeItem(i);
    }
  }
  for(int i=count()-1;i>box_max_items;i--) {
    removeItem(i);
  }
}
