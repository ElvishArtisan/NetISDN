// list_users.cpp
//
// List NetISDN Users
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

#include <math.h>
#include <unistd.h>

#include <qdialog.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qdatetime.h>
#include <qmessagebox.h>
#include <qtimer.h>

#include <globals.h>
#include <list_users.h>
#include <edit_user.h>
#include <add_user.h>

//
// Icons
//
#include "../icons/admin.xpm"
#include "../icons/user.xpm"
#include "../icons/greenball.xpm"
#include "../icons/redball.xpm"
#include "../icons/whiteball.xpm"

ListUsers::ListUsers(QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());

  setCaption(tr("NetISDN User List"));

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
  list_admin_map=new QPixmap(admin_xpm);
  list_user_map=new QPixmap(user_xpm);
  list_greenball_map=new QPixmap(greenball_xpm);
  list_redball_map=new QPixmap(redball_xpm);
  list_whiteball_map=new QPixmap(whiteball_xpm);

  //
  //  Add Button
  //
  list_add_button=new QPushButton(this,"list_add_button");
  list_add_button->setFont(font);
  list_add_button->setText(tr("&Add"));
  connect(list_add_button,SIGNAL(clicked()),this,SLOT(addData()));

  //
  //  Edit Button
  //
  list_edit_button=new QPushButton(this,"list_edit_button");
  list_edit_button->setFont(font);
  list_edit_button->setText(tr("&Edit"));
  connect(list_edit_button,SIGNAL(clicked()),this,SLOT(editData()));

  //
  //  Delete Button
  //
  list_delete_button=new QPushButton(this,"list_delete_button");
  list_delete_button->setFont(font);
  list_delete_button->setText(tr("&Delete"));
  connect(list_delete_button,SIGNAL(clicked()),this,SLOT(deleteData()));

  //
  //  Close Button
  //
  list_close_button=new QPushButton(this,"list_close_button");
  list_close_button->setDefault(true);
  list_close_button->setFont(font);
  list_close_button->setText(tr("&Close"));
  connect(list_close_button,SIGNAL(clicked()),this,SLOT(closeData()));

  //
  // Online Checkbox
  //
  list_online_check=new QCheckBox(this,"list_online_check");
  list_online_label=new QLabel(list_online_check,tr("Show online users only"),
			   this,"list_online_label");
  list_online_label->setFont(font);
  list_online_label->setAlignment(AlignVCenter|AlignLeft);
  connect(list_online_check,SIGNAL(toggled(bool)),
	  this,SLOT(onlineToggledData(bool)));

  //
  // User List
  //
  list_view=new QListView(this,"list_view");
  list_view->setAllColumnsShowFocus(true);
  list_view->setSelectionMode(QListView::Single);
  list_view->setItemMargin(5);
  list_view->addColumn(tr(" "));
  list_view->setColumnAlignment(0,Qt::AlignLeft);
  list_view->addColumn(tr("Name"));
  list_view->setColumnAlignment(1,Qt::AlignLeft);
  list_view->addColumn(tr("Full Name"));
  list_view->setColumnAlignment(2,Qt::AlignLeft);
  list_view->addColumn(tr("OnLine"));
  list_view->setColumnAlignment(3,Qt::AlignCenter);
  list_view->addColumn(tr("Active"));
  list_view->setColumnAlignment(4,Qt::AlignCenter);
  list_view->addColumn(tr("Expiration Date"));
  list_view->setColumnAlignment(5,Qt::AlignCenter);
  list_view->addColumn(tr("Last Accessed On"));
  list_view->setColumnAlignment(6,Qt::AlignCenter);
  list_view->addColumn(tr("Last Accessed From"));
  list_view->setColumnAlignment(7,Qt::AlignCenter);

  connect(list_view,SIGNAL(doubleClicked(QListViewItem *,const QPoint &,int)),
	  this,SLOT(doubleClickedData(QListViewItem *,const QPoint &,int)));

  QTimer *timer=new QTimer(this);
  connect(timer,SIGNAL(timeout()),this,SLOT(userUpdateData()));
  timer->start(500);

  RefreshList();
}


ListUsers::~ListUsers()
{
  delete list_view;
}


QSize ListUsers::sizeHint() const
{
  return QSize(400,300);
} 


QSizePolicy ListUsers::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void ListUsers::addData()
{
  QString username;
  NetListViewItem *item=NULL;

  AddUser *add_user=new AddUser(&username,this,"add_user");
  if(add_user->exec()<0) {
    delete add_user;
    return;
  }
  delete add_user;
  item=new NetListViewItem(list_view);
  item->setText(1,username);
  RefreshItem(item);
  list_view->ensureItemVisible(item);
}


void ListUsers::editData()
{
  NetListViewItem *item;
  if((item=(NetListViewItem *)list_view->currentItem())==NULL) {
    return;
  }
  EditUser *edit_user=new EditUser(item->text(1),this,"edit_user");
  if(edit_user->exec()==0) {
    RefreshItem(item);
  }
  delete edit_user;
}


void ListUsers::deleteData()
{
  QString sql;
  QSqlQuery *q;
  QString str;

  NetListViewItem *item;
  if((item=(NetListViewItem *)list_view->currentItem())==NULL) {
    return;
  }
  if(item->text(1)==list_current_user) {
    QMessageBox::warning(this,tr("Can't Delete User"),
			 tr("You cannot delete yourself!"),1,0);
    return;
  }

  str=QString((tr("Are you sure you want to delete user")));
  if(QMessageBox::warning(this,tr("Delete User"),
			  QString().sprintf(
			    "%s %s?",(const char *)str,
			    (const char *)item->text(1)),
			  QMessageBox::Yes,QMessageBox::No)==
     QMessageBox::Yes) {

    sql=QString().sprintf("delete from USERS where NAME=\"%s\"",
			  (const char *)item->text(1));
    q=new QSqlQuery(sql);
    delete q;
    delete item;
  }
}


void ListUsers::onlineToggledData(bool state)
{
  RefreshList();
}


void ListUsers::userUpdateData()
{

  char buf[32];
  int n;
  int uid=0;
  bool ok=false;

  for(unsigned i=0;i<32;i++) {
    buf[i]=0;
  }
  if((n=read(global_status_socket,buf,31))>0) {
    if(buf[0]!='u') {
      return;
    }
    uid=QString(buf+1).toInt(&ok);
    if(!ok) {
      fprintf(stderr,"malformed user id received: %s\n",buf);
      return;
    }
    RefreshItem(uid);
  }
}


void ListUsers::closeData()
{
  done(0);
}


void ListUsers::doubleClickedData(QListViewItem *item,const QPoint &pt,int col)
{
  editData();
}


void ListUsers::resizeEvent(QResizeEvent *e)
{
  list_add_button->setGeometry(10,size().height()-60,80,50);
  list_edit_button->setGeometry(100,size().height()-60,80,50);
  list_delete_button->setGeometry(190,size().height()-60,80,50);
  list_close_button->setGeometry(size().width()-90,size().height()-60,80,50);
  list_online_check->setGeometry(15,10,15,15);
  list_online_label->setGeometry(35,8,size().width()-45,20);
  list_view->setGeometry(10,30,size().width()-20,size().height()-100);
}


void ListUsers::RefreshItem(NetListViewItem *item)
{
  QString sql;
  QSqlQuery *q;
  QDate today=QDate::currentDate();

  sql=QString().sprintf("select ID,FULL_NAME,EXPIRATION_DATE,\
                         LAST_ACCESS_DATETIME,LAST_ACCESS_ADDR,ADMIN_PRIV,\
                         ONLINE from USERS where (NAME=\"%s\")",
			(const char *)item->text(1));
  if(list_online_check->isChecked()) {
    sql+="&&(ONLINE=\"Y\")";
  }
  q=new QSqlQuery(sql);
  if(q->first()) {
    item->setId(q->value(0).toInt());
    if(q->value(5).toString()=="Y") {
      item->setPixmap(0,*list_admin_map);
    }
    else {
      item->setPixmap(0,*list_user_map);
    }
    item->setText(2,q->value(1).toString());
    if(q->value(2).toDate().isNull()||(q->value(2).toDate()>=today)) {
      if(q->value(6).toString()=="Y") {
	item->setPixmap(3,*list_greenball_map);
      }
      else {
	item->setPixmap(3,*list_whiteball_map);
      }
      item->setPixmap(4,*list_whiteball_map);
    }
    else {
      item->setPixmap(3,*list_redball_map);
      item->setPixmap(4,*list_redball_map);
    }
    if(q->value(2).isNull()) {
      item->setText(5,tr("[none]"));
    }
    else {
      item->setText(5,q->value(2).toDate().toString("MM/dd/yyyy"));
    }
    if(q->value(3).isNull()) {
      item->setText(6,tr("[never]"));
    }
    else {
      item->setText(6,q->value(3).toDateTime().
		    toString("MM/dd/yyyy - hh:mm:ss"));
    }
    if(q->value(4).isNull()) {
      item->setText(7,tr("[none]"));
    }
    else {
      item->setText(7,q->value(4).toString());
    }
  }
  else {
    delete item;
    delete q;
    return;
  }
  delete q;

  //
  // Refresh Call Status
  //
  sql=QString().sprintf("select START_DATETIME from CALLS where \
                         ((ORIG_UID=%d)||(DEST_UID=%d))\
                         &&(END_DATETIME is null)",item->id(),item->id());
  q=new QSqlQuery(sql);
  if(q->first()) {
    item->setPixmap(4,*list_greenball_map);
  }
  delete q;
}


void ListUsers::RefreshItem(int uid)
{
  QString sql;
  QSqlQuery *q;

  NetListViewItem *item=(NetListViewItem *)list_view->firstChild();
  while(item!=NULL) {
    if(item->id()==uid) {
      RefreshItem(item);
      return;
    }
    item=(NetListViewItem *)item->nextSibling();
  }
  sql=QString().sprintf("select NAME from USERS where (ID=%d)",uid);
  if(list_online_check->isChecked()) {
    sql+="&&(ONLINE=\"Y\")";
  }
  q=new QSqlQuery(sql);
  if(q->first()) {
    item=new NetListViewItem(list_view);
    item->setText(1,q->value(0).toString());
    RefreshItem(item);
  }
  delete q;
}


void ListUsers::RefreshList()
{
  QString sql;
  QSqlQuery *q;
  NetListViewItem *item;
  QDate today=QDate::currentDate();
  int uid=-1;

  item=(NetListViewItem *)list_view->firstChild();
  while(item!=NULL) {
    if(item->isSelected()) {
      uid=item->id();
    }
    item=(NetListViewItem *)item->nextSibling();
  }

  list_view->clear();
  sql="select ID,NAME,FULL_NAME,EXPIRATION_DATE,LAST_ACCESS_DATETIME,\
       LAST_ACCESS_ADDR,ADMIN_PRIV,ONLINE from USERS ";
  if(list_online_check->isChecked()) {
    sql+="where ONLINE=\"Y\" ";
  }
  sql+="order by NAME desc";
  q=new QSqlQuery(sql);
  while (q->next()) {
    item=new NetListViewItem(list_view);
    item->setId(q->value(0).toInt());
    if(q->value(6).toString()=="Y") {
      item->setPixmap(0,*list_admin_map);
    }
    else {
      item->setPixmap(0,*list_user_map);
    }
    item->setText(1,q->value(1).toString());
    item->setText(2,q->value(2).toString());
    if(q->value(3).toDate().isNull()||(q->value(3).toDate()>=today)) {
      if(q->value(7).toString()=="Y") {
	item->setPixmap(3,*list_greenball_map);
      }
      else {
	item->setPixmap(3,*list_whiteball_map);
      }
      item->setPixmap(4,*list_whiteball_map);
    }
    else {
      item->setPixmap(3,*list_redball_map);
      item->setPixmap(4,*list_redball_map);
    }
    if(q->value(3).isNull()) {
      item->setText(5,tr("[none]"));
    }
    else {
      item->setText(5,q->value(3).toDate().toString("MM/dd/yyyy"));
    }
    if(q->value(4).isNull()) {
      item->setText(6,tr("[never]"));
    }
    else {
      item->setText(6,q->value(4).toDateTime().
		    toString("MM/dd/yyyy - hh:mm:ss"));
    }
    if(q->value(5).isNull()) {
      item->setText(7,tr("[none]"));
    }
    else {
      item->setText(7,q->value(5).toString());
    }
    if(item->id()==uid) {
      item->setSelected(true);
      list_view->setCurrentItem(item);
    }
  }
  delete q;

  //
  // Refresh Call Status
  //
  sql="select ORIG_UID,DEST_UID from CALLS where END_DATETIME is null";
  q=new QSqlQuery(sql);
  while(q->next()) {
    RefreshItem(q->value(0).toInt());
    RefreshItem(q->value(1).toInt());
  }
  delete q;
}
