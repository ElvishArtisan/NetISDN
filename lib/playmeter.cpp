// playmeter.cpp
//
// This implements a widget that represents a stereo audio level meter,
// complete with labels and scale.
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: playmeter.cpp,v 1.1 2008/05/21 13:35:39 cvs Exp $
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

#include <qwidget.h>
#include <qstring.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qcolor.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qsize.h>
#include <stdio.h>
#include <qslider.h>
#include <qbuttongroup.h>
#include <qsizepolicy.h>
#include <qmessagebox.h>

#include <playmeter.h>

PlayMeter::PlayMeter(unsigned id,SegMeter::Orientation orient,
		     QWidget *parent,const char *name)
  : QWidget(parent,name)
{
  meter_id=id;
  meter_label=QString("");
  setBackgroundColor(black);
  orientation=orient;
  makeFont();
  meter=new SegMeter(orientation,this,"meter");
  meter->setSegmentSize(5);
  meter->setSegmentGap(1);
}


QSize PlayMeter::sizeHint() const
{
  if(meter_label==QString("")) {
    return QSize(335,60);
  }
  else {
    return QSize(335,80);
  }
}


QSizePolicy PlayMeter::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}


void PlayMeter::setRange(int min,int max)
{
  meter->setRange(min,max);
}


void PlayMeter::setDarkLowColor(QColor color)
{
  meter->setDarkLowColor(color);
}


void PlayMeter::setDarkHighColor(QColor color)
{
  meter->setDarkHighColor(color);
}


void PlayMeter::setDarkClipColor(QColor color)
{
  meter->setDarkClipColor(color);
}


void PlayMeter::setLowColor(QColor color)
{
  meter->setLowColor(color);
}


void PlayMeter::setHighColor(QColor color)
{
  meter->setHighColor(color);
}


void PlayMeter::setClipColor(QColor color)
{
  meter->setClipColor(color);
}


void PlayMeter::setHighThreshold(int level)
{
  meter->setHighThreshold(level);
}


void PlayMeter::setClipThreshold(int level)
{
  meter->setClipThreshold(level);
}


void PlayMeter::setLabel(QString label)
{
  meter_label=label;
  makeFont();
  setGeometry(geometry().left(),geometry().top(),
	      geometry().width(),geometry().height());
}


void PlayMeter::setGeometry(int x,int y,int w,int h)
{
  QWidget::setGeometry(x,y,w,h);
  if(meter_label.isEmpty()) {
    meter->setGeometry(2,2,w-4,h-4);
  }
  else {
    switch(orientation) {
	case SegMeter::Left:
	  meter->setGeometry(2,2,w-4-h,h-4);
	  label_font=QFont("helvetica",height()-2,QFont::Bold);
	  label_font.setPixelSize(height()-2);
	  break;
	case SegMeter::Right:
	  meter->setGeometry(2+h,2,w-4-h,h-4);
	  label_font=QFont("helvetica",height()-2,QFont::Bold);
	  label_font.setPixelSize(height()-2);
	  break;
	case SegMeter::Up:
	  meter->setGeometry(2,2,w-4,h-4-w);
	  label_font=QFont("helvetica",width()-2,QFont::Bold);
	  label_font.setPixelSize(width()-2);
	  break;
	case SegMeter::Down:
	  meter->setGeometry(2,2+width(),w-4,h-4-w);
	  label_font=QFont("helvetica",width()-2,QFont::Bold);
	  label_font.setPixelSize(width()-2);
	  break;
    }
    makeFont();
  }
}


void PlayMeter::setGeometry(QRect &rect)
{
  setGeometry(rect.left(),rect.top(),rect.width(),rect.height());
}


void PlayMeter::setSolidBar(unsigned id,int level)
{
  if(id==meter_id) {
    meter->setSolidBar(level);
  }
}


void PlayMeter::setPeakBar(unsigned id,int level)
{
  if(id==meter_id) {
    meter->setPeakBar(level);
  }
}


void PlayMeter::setFloatingBar(unsigned id,int level)
{
  if(id==meter_id) {
    meter->setFloatingBar(level);
  }
}


void PlayMeter::setSegmentSize(int size)
{
  meter->setSegmentSize(size);
}


void PlayMeter::setSegmentGap(int gap)
{
  meter->setSegmentGap(gap);
}


SegMeter::Mode PlayMeter::mode() const
{
  return meter->mode();
}


void PlayMeter::setMode(SegMeter::Mode mode)
{
  meter->setMode(mode);
}


void PlayMeter::paintEvent(QPaintEvent *paintEvent)
{
  //
  // Setup
  //
  QPainter *p=new QPainter(this);
  p->setFont(label_font);
  p->setPen(white);
  if(!meter_label.isEmpty()) {
    switch(orientation) {
	case SegMeter::Left:
	  p->drawText(width()-height()+meter_label_x,height()-2,meter_label);
	  break;
	case SegMeter::Right:
	  p->drawText(meter_label_x,height()-2,meter_label);
	  break;
	case SegMeter::Up:
	  p->drawText(meter_label_x,height()-3,meter_label);
	  break;
	case SegMeter::Down:
	  p->drawText(meter_label_x,width()-1,meter_label);
	  break;
    }
  }
  p->end();
}



void PlayMeter::makeFont()
{
  switch(orientation) {
      case SegMeter::Left:
	label_font=QFont("helvetica",height()-2,QFont::Bold);
	label_font.setPixelSize(height()-2);
	meter_label_x=(height()-QFontMetrics(label_font).
		       width(meter_label))/2;
	break;
      case SegMeter::Right:
	label_font=QFont("helvetica",height()-2,QFont::Bold);
	label_font.setPixelSize(height()-2);
	meter_label_x=(height()-QFontMetrics(label_font).
		       width(meter_label))/2;
	break;
      case SegMeter::Up:
	label_font=QFont("helvetica",width()-2,QFont::Bold);
	label_font.setPixelSize(width()-2);
	meter_label_x=(width()-QFontMetrics(label_font).
		       width(meter_label))/2;
	break;
      case SegMeter::Down:
	label_font=QFont("helvetica",width()-2,QFont::Bold);
	label_font.setPixelSize(width()-2);
	meter_label_x=(width()-QFontMetrics(label_font).
		       width(meter_label))/2;
	break;
  }
}
