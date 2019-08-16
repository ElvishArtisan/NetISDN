//   netlistviewitem.h
//
//   QListViewItem class for NetISDN
//
//   (C) Copyright 2009 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: netlistviewitem.h,v 1.1 2009/01/26 13:05:20 pcvs Exp $
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
//

#ifndef NETLISTVIEWITEM_H
#define NETLISTVIEWITEM_H

#include <vector>

#include <qlistview.h>
#include <qpixmap.h>
#include <qlistview.h>

class NetListViewItem : public QListViewItem
{
 public:
  NetListViewItem(QListView *parent);
  int line() const;
  void setLine(int line);
  int id() const;
  void setId(int id);
  QColor backgroundColor() const;
  void setBackgroundColor(QColor color);
  QColor textColor(int column) const;
  void setTextColor(QColor color);
  void setTextColor(int column,QColor color,int weight);
  void paintCell(QPainter *p,const QColorGroup &cg,int column,
		 int width,int align);

 private:
  int item_line;
  int item_id;
  std::vector<QColor> item_text_color;
  std::vector<int> item_text_weight;
  QColor item_background_color;
  QListView *list_parent;
};


#endif  // NETLISTVIEWITEM_H
