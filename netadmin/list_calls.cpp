// list_calls.cpp
//
// List NetISDN Calls
//
//   (C) Copyright 2008-2019 Fred Gleason <fredg@paravelsystems.com>
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

#include <unistd.h>

#include <globals.h>

#include <qdialog.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qdatetime.h>
#include <qmessagebox.h>
#include <qtimer.h>

#include <audiosettings.h>

#include <list_calls.h>
#include <edit_user.h>
#include <add_user.h>

//
// Icons
//
#include "../icons/call.xpm"
#include "../icons/active_call.xpm"

ListCalls::ListCalls(QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());

  setCaption(tr("Rivendell User List"));

  //
  // Create Fonts
  //
  QFont font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);
  QFont small_font=QFont("Helvetica",10,QFont::Bold);
  small_font.setPixelSize(10);

  //
  // Create Icons
  //
  list_call_map=new QPixmap(call_xpm);
  list_active_call_map=new QPixmap(active_call_xpm);

  //
  //  Close Button
  //
  list_close_button=new QPushButton(this,"list_close_button");
  list_close_button->setDefault(true);
  list_close_button->setFont(font);
  list_close_button->setText(tr("&Close"));
  connect(list_close_button,SIGNAL(clicked()),this,SLOT(closeData()));

  //
  // Active Checkbox
  //
  list_active_check=new QCheckBox(this,"list_active_check");
  list_active_label=new QLabel(list_active_check,tr("Show active calls only"),
			   this,"list_active_label");
  list_active_label->setFont(font);
  list_active_label->setAlignment(AlignVCenter|AlignLeft);
  connect(list_active_check,SIGNAL(toggled(bool)),
	  this,SLOT(activeToggledData(bool)));

  //
  // User List
  //
  list_view=new QListView(this,"list_view");
  list_view->setAllColumnsShowFocus(true);
  list_view->setSelectionMode(QListView::Single);
  list_view->setItemMargin(5);
  list_view->addColumn(tr(" "));
  list_view->setColumnAlignment(0,Qt::AlignLeft);
  list_view->addColumn(tr("Originator"));
  list_view->setColumnAlignment(1,Qt::AlignLeft);
  list_view->addColumn(tr("Recipient"));
  list_view->setColumnAlignment(2,Qt::AlignLeft);
  list_view->addColumn(tr("Start Time"));
  list_view->setColumnAlignment(3,Qt::AlignLeft);
  list_view->addColumn(tr("End Time"));
  list_view->setColumnAlignment(4,Qt::AlignLeft);
  list_view->addColumn(tr("Sample Rate"));
  list_view->setColumnAlignment(5,Qt::AlignCenter);
  list_view->addColumn(tr("Origin Format"));
  list_view->setColumnAlignment(6,Qt::AlignCenter);
  list_view->addColumn(tr("Origin Channels"));
  list_view->setColumnAlignment(7,Qt::AlignCenter);
  list_view->addColumn(tr("Origin Bitrate"));
  list_view->setColumnAlignment(8,Qt::AlignCenter);
  list_view->addColumn(tr("Recep Format"));
  list_view->setColumnAlignment(9,Qt::AlignCenter);
  list_view->addColumn(tr("Recep Channels"));
  list_view->setColumnAlignment(10,Qt::AlignCenter);
  list_view->addColumn(tr("Recep Bitrate"));
  list_view->setColumnAlignment(11,Qt::AlignCenter);

  list_view->setSorting(3,false);

  QTimer *timer=new QTimer(this);
  connect(timer,SIGNAL(timeout()),this,SLOT(callUpdateData()));
  timer->start(500);

  RefreshList();
}


ListCalls::~ListCalls()
{
  delete list_view;
}


QSize ListCalls::sizeHint() const
{
  return QSize(400,300);
} 


QSizePolicy ListCalls::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void ListCalls::callUpdateData()
{

  char buf[32];
  int n;
  int cid=0;
  bool ok=false;

  for(unsigned i=0;i<32;i++) {
    buf[i]=0;
  }
  if((n=read(global_status_socket,buf,31))>0) {
    if(buf[0]!='c') {
      return;
    }
    cid=QString(buf+1).toInt(&ok);
    if(!ok) {
      fprintf(stderr,"malformed call id received: %s\n",buf);
      return;
    }
    RefreshItem(cid);
  }
}


void ListCalls::activeToggledData(bool state)
{
  RefreshList();
}


void ListCalls::closeData()
{
  done(0);
}


void ListCalls::resizeEvent(QResizeEvent *e)
{
  list_close_button->setGeometry(size().width()-90,size().height()-60,80,50);
  list_active_check->setGeometry(15,10,15,15);
  list_active_label->setGeometry(35,8,size().width()-45,20);
  list_view->setGeometry(10,30,size().width()-20,size().height()-100);
}


void ListCalls::RefreshItem(int cid)
{
  QString sql;
  QSqlQuery *q;
  NetListViewItem *l=NULL;
  NetListViewItem *item=NULL;

  l=(NetListViewItem *)list_view->firstChild();
  while(l!=NULL) {
    if(l->id()==cid) {
      item=l;
    }
    l=(NetListViewItem *)l->nextSibling();
  }
  if(item==NULL) {
    item=new NetListViewItem(list_view);
    item->setId(cid);
  }
  sql=QString().sprintf("select CALLS.ID,USERS.NAME,CALLS.DEST_UID,\
                         CALLS.START_DATETIME,CALLS.END_DATETIME,CALLS.\
                         SAMPLE_RATE,CALLS.FORMAT,CALLS.CHANNELS,\
                         CALLS.BIT_RATE,CALLS.DEST_SAMPLE_RATE,\
                         CALLS.DEST_FORMAT,CALLS.DEST_CHANNELS,\
                         CALLS.DEST_BIT_RATE from CALLS left join USERS \
                         on CALLS.ORIG_UID=USERS.ID where (CALLS.ID=%d)",cid);
  if(list_active_check->isChecked()) {
    sql+=" &&(END_DATETIME is null)";
  }
  sql+=" order by CALLS.START_DATETIME";
  q=new QSqlQuery(sql);
  if(q->first()) {
    SetItem(q,item);
  }
  else {
    delete item;
  }
  delete q;
}


void ListCalls::RefreshList()
{
  QString sql;
  QSqlQuery *q;
  NetListViewItem *item;
  QDate today=QDate::currentDate();

  list_view->clear();
  sql="select CALLS.ID,USERS.NAME,CALLS.DEST_UID,CALLS.START_DATETIME,\
       CALLS.END_DATETIME,CALLS.SAMPLE_RATE,CALLS.FORMAT,CALLS.CHANNELS,\
       CALLS.BIT_RATE,CALLS.DEST_SAMPLE_RATE,CALLS.DEST_FORMAT,\
       CALLS.DEST_CHANNELS,CALLS.DEST_BIT_RATE from CALLS left join USERS \
       on CALLS.ORIG_UID=USERS.ID";
  if(list_active_check->isChecked()) {
    sql+=" where END_DATETIME is null";
  }
  sql+=" order by CALLS.START_DATETIME";
  q=new QSqlQuery(sql);
  while (q->next()) {
    item=new NetListViewItem(list_view);
    SetItem(q,item);
 }
  delete q;
}


void ListCalls::SetItem(QSqlQuery *q,NetListViewItem *item)
{
  QString sql;
  QSqlQuery *q1;

  item->setId(q->value(0).toInt());
  if(q->value(4).toDateTime().isNull()) {
    item->setPixmap(0,*list_active_call_map);
    item->setText(4,tr("[none]"));
  }
  else {
    item->setPixmap(0,*list_call_map);
    item->
      setText(4,q->value(4).toDateTime().toString("MM/dd/yyyy - hh:mm:ss"));
  }
  item->setText(1,q->value(1).toString());
  item->setText(3,q->value(3).toDateTime().toString("MM/dd/yyyy - hh:mm:ss"));
  item->setText(5,q->value(5).toString()+" samples/sec");
  item->setText(6,
     AudioSettings::formatString((AudioSettings::Format)q->value(6).toUInt()));
  item->setText(7,q->value(7).toString());
  item->setText(8,q->value(8).toString()+" bits/sec");
  
  item->setText(9,
     AudioSettings::formatString((AudioSettings::Format)q->value(10).toUInt()));
  item->setText(10,q->value(11).toString());
  item->setText(11,q->value(12).toString()+" bits/sec");
  
  sql=QString().sprintf("select NAME from USERS where ID=%d",
			q->value(2).toInt());
  q1=new QSqlQuery(sql);
  if(q1->first()) {
    item->setText(2,q1->value(0).toString());
  }
  delete q1;
}
