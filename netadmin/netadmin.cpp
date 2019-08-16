// netadmin.cpp
//
// The Administrator Utility for NetISDN.
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: netadmin.cpp,v 1.3 2009/01/26 13:05:20 pcvs Exp $
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


#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>

#include <qapplication.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qfiledialog.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <qtimer.h>

#include <dbversion.h>
#include <cmd_switch.h>
#include <escape_string.h>
#include <netconf.h>

#include <login.h>
#include <list_users.h>
#include <list_calls.h>
#include <netadmin.h>
#include <opendb.h>
#include <createdb.h>

//
// Icons
//
#include "../icons/netisdn-22x22.xpm"

//
// Global Classes
//
NetConfig *admin_config;
int global_status_socket=-1;

void SigHandler(int signo)
{
  pid_t pLocalPid;

  switch(signo) {
      case SIGCHLD:
	pLocalPid=waitpid(-1,NULL,WNOHANG);
	while(pLocalPid>0) {
	  pLocalPid=waitpid(-1,NULL,WNOHANG);
	}
	signal(SIGCHLD,SigHandler);
	break;
  }
}


MainWidget::MainWidget(QWidget *parent,const char *name)
  :QWidget(parent,name)
{
  QString str;
  QString sql;
  QSqlQuery *q;
  sockaddr_in sa;
  ip_mreqn mreq;
  long sockopt;
  struct hostent *hostent=NULL;

  //
  // Read Command Options
  //
  CmdSwitch *cmd=
      new CmdSwitch(qApp->argc(),qApp->argv(),"netadmin",NETADMIN_USAGE);
  delete cmd;

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  //
  // Create Fonts
  //
  QFont font=QFont("Helvetica",12,QFont::Bold);
  font.setPixelSize(12);
  QFont default_font("Helvetica",12,QFont::Normal);
  default_font.setPixelSize(12);
  qApp->setFont(default_font);

  //
  // Create And Set Icon
  //
  admin_netisdn_map=new QPixmap(netisdn_xpm);
  setIcon(*admin_netisdn_map);

  //
  // Load Configs
  //
  admin_config=new NetConfig();
  admin_config->load();

  setCaption("NetAdmin");

  //
  // Status Update Socket
  //
  if((global_status_socket=socket(PF_INET,SOCK_DGRAM,0))<0) {
    QMessageBox::warning(this,tr("NetAdmin - Network Error"),
			 tr("Unable to open status update socket!"));
  }

  sockopt=O_NONBLOCK;
  fcntl(global_status_socket,F_SETFL,sockopt);

  sockopt=1;
  setsockopt(global_status_socket,IPPROTO_IP,IP_MULTICAST_LOOP,
	     &sockopt,sizeof(sockopt));

  memset(&mreq,0,sizeof(mreq));
  hostent=gethostbyname(NETSIP_STATUS_MCAST_ADDR);
  if(hostent!=NULL ) {
    mreq.imr_multiaddr.s_addr=*((u_int32_t *)hostent->h_addr);
  }
  hostent=gethostbyname(admin_config->globalInterface());
  if(hostent!=NULL) {
    mreq.imr_address.s_addr=*((u_int32_t *)hostent->h_addr);
  }
  mreq.imr_ifindex=0;
  setsockopt(global_status_socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,
	     &mreq,sizeof(mreq));

  memset(&sa,0,sizeof(sa));
  sa.sin_family=AF_INET;
  sa.sin_port=htons(NETSIP_STATUS_MCAST_PORT);
  sa.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(global_status_socket,(sockaddr *)&sa,sizeof(sa))<0) {
    QMessageBox::warning(this,tr("NetAdmin - Network Error"),
			 tr("Unable to set options for status update socket!"));
  }

  //
  // Open Database
  //
  if(!OpenDb(admin_config->mysqlDbname(),admin_config->mysqlUsername(),
	     admin_config->mysqlPassword(),admin_config->mysqlHostname(),
	     admin_config->mysqlDriver())) {
    exit(1);
  }

  //
  // Log In
  //
  Login *login=new Login(&admin_username,&admin_password,this,"login");
  if(login->exec()!=0) {
    exit(0);
  }
  sql=QString().sprintf("select ADMIN_PRIV from USERS where (NAME=\"%s\")&&\
                         (PASSWORD=\"%s\")",
			(const char *)EscapeString(admin_username),
			(const char *)EscapeString(admin_password));
  q=new QSqlQuery(sql);
  if(q->first()) {
    if(q->value(0).toString()!="Y") {
      QMessageBox::warning(this,tr("Insufficient Priviledges"),
         tr("This account has insufficient priviledges for this operation."));
      exit(256);
    }
  }
  else {
    QMessageBox::warning(this,"Login Failed","Login Failed!.\n");
    exit(256);
  }
  delete q;

  //
  // User Labels
  //
  QLabel *name_label=new QLabel(this,"name_label");
  name_label->setGeometry(0,5,sizeHint().width(),20);
  name_label->setAlignment(Qt::AlignVCenter|Qt::AlignCenter);
  name_label->setFont(font);
  name_label->setText(QString().sprintf("USER: %s",
					(const char *)admin_username));

  //
  // Manage Users Button
  //
  QPushButton *users_button=new QPushButton(this,"users_button");
  users_button->setGeometry(10,30,80,60);
  users_button->setFont(font);
  users_button->setText(tr("Manage\n&Users"));
  connect(users_button,SIGNAL(clicked()),this,SLOT(manageUsersData()));

  //
  // Manage Calls Button
  //
  QPushButton *calls_button=new QPushButton(this,"calls_button");
  calls_button->setGeometry(100,30,80,60);
  calls_button->setFont(font);
  calls_button->setText(tr("Manage\n&Calls"));
  connect(calls_button,SIGNAL(clicked()),this,SLOT(listCallsData()));

  //
  // Backup Database Button
  //
  QPushButton *backup_button=new QPushButton(this,"backup_button");
  backup_button->setGeometry(10,100,80,60);
  backup_button->setFont(font);
  backup_button->setText(tr("&Backup\nDatabase"));
  connect(backup_button,SIGNAL(clicked()),this,SLOT(backupData()));

  //
  // Restore Database Button
  //
  QPushButton *restore_button=new QPushButton(this,"restore_button");
  restore_button->setGeometry(100,100,80,60);
  restore_button->setFont(font);
  restore_button->setText(tr("&Restore\nDatabase"));
  connect(restore_button,SIGNAL(clicked()),this,SLOT(restoreData()));
  
  //
  // Quit Button
  //
  QPushButton *quit_button=new QPushButton(this,"quit_button");
  quit_button->setGeometry(10,sizeHint().height()-70,sizeHint().width()-20,60);
  quit_button->setFont(font);
  quit_button->setText(tr("&Quit"));
  connect(quit_button,SIGNAL(clicked()),this,SLOT(quitMainWidget()));

  signal(SIGCHLD,SigHandler);
}


QSize MainWidget::sizeHint() const
{
  return QSize(190,250);
}


QSizePolicy MainWidget::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void MainWidget::manageUsersData()
{
  ListUsers *list_users=new ListUsers(this);
  list_users->exec();
  delete list_users;
}


void MainWidget::listCallsData()
{
  ListCalls *list_calls=new ListCalls(this);
  list_calls->exec();
  delete list_calls;
}


void MainWidget::backupData()
{
  QString filename;
  QString cmd;
  int status;

  filename=QFileDialog::getSaveFileName(NetGetHomeDir(),
				      tr("Rivendell Database Backup (*.sql)"),
					this);
  if(filename.isEmpty()) {
    return;
  }
  if(filename.right(4)!=QString(".sql")) {
    filename+=".sql";
  }
  cmd=QString().sprintf("mysqldump -c %s -h %s -u %s -p%s > %s",
			(const char *)admin_config->mysqlDbname(),
			(const char *)admin_config->mysqlHostname(),
			(const char *)admin_config->mysqlUsername(),
			(const char *)admin_config->mysqlPassword(),
			(const char *)filename);
  status=system((const char *)cmd);
  if(WEXITSTATUS(status)!=0) {
    unlink((const char *)filename);
    QMessageBox::warning(this,tr("Backup Error"),
			 tr("Unable to create backup!"));
    return;
  }
  QMessageBox::information(this,tr("Backup Complete"),
			   tr("Backup completed successfully."));
}


void MainWidget::restoreData()
{
  QString filename;
  QString cmd;
  int status;
  QSqlQuery *q;
  int ver=NET_VERSION_DATABASE;

  if(QMessageBox::warning(NULL,tr("Restore Database"),
			  tr("WARNING: This operation will COMPLETELY\nOVERWRITE the existing Rivendell Database!\nDo you want to continue?"),
			  QMessageBox::Yes,QMessageBox::No)!=
     QMessageBox::Yes) {
    return;
  }      
  filename=QFileDialog::getOpenFileName(NetGetHomeDir(),
				    tr("Rivendell Database Backup (*.sql)"),
					this);
  if(filename.isEmpty()) {
    return;
  }
  ClearTables();
  cmd=QString().sprintf("cat %s | mysql %s -h %s -u %s -p%s",
			(const char *)filename,
			(const char *)admin_config->mysqlDbname(),
			(const char *)admin_config->mysqlHostname(),
			(const char *)admin_config->mysqlUsername(),
			(const char *)admin_config->mysqlPassword());
  status=system((const char *)cmd);
  if(WEXITSTATUS(status)!=0) {
    QMessageBox::warning(this,tr("Restore Error"),
			 tr("Unable to restore backup!"));
    return;
  }
  q=new QSqlQuery("select DB from VERSION");
  if(q->first()) {
    ver=q->value(0).toInt();
  }
  delete q;
  UpdateDb(ver);
  QMessageBox::information(this,tr("Restore Complete"),
			   tr("Restore completed successfully."));
}


void MainWidget::quitMainWidget()
{
  exit(0);
}


void MainWidget::ClearTables()
{
  QSqlQuery *q1;

  QString sql="show tables";
  QSqlQuery *q=new QSqlQuery(sql);
  while(q->next()) {
    sql=QString().sprintf("drop table %s",
			  (const char *)q->value(0).toString());
    q1=new QSqlQuery(sql);
    delete q1;
  }
  delete q;
}


int gui_main(int argc,char *argv[])
{
  QApplication a(argc,argv);
  
  //
  // Load Translations
  //
  QTranslator qt(0);
  qt.load(QString(QTDIR)+QString("/translations/qt_")+QTextCodec::locale(),
	  ".");
  a.installTranslator(&qt);

  QTranslator rd(0);
  rd.load(QString(PREFIX)+QString("/share/rivendell/librd_")+
	     QTextCodec::locale(),".");
  a.installTranslator(&rd);

  QTranslator rdhpi(0);
  rdhpi.load(QString(PREFIX)+QString("/share/rivendell/librdhpi_")+
	     QTextCodec::locale(),".");
  a.installTranslator(&rdhpi);

  QTranslator tr(0);
  tr.load(QString(PREFIX)+QString("/share/rivendell/netadmin_")+
	  QTextCodec::locale(),".");
  a.installTranslator(&tr);

  //
  // Start Event Loop
  //
  MainWidget *w=new MainWidget(NULL,"main");
/*
  if(exiting) {
      exit(0);
  }
*/
  a.setMainWidget(w);
  w->setGeometry(QRect(QPoint(0,0),w->sizeHint()));
  w->show();
  return a.exec();
}


int cmdline_main(int argc,char *argv[])
{
  QApplication a(argc,argv,false);
  
  //
  // Load Configs
  //
  admin_config=new NetConfig();
  admin_config->load();

  //
  // Open Database
  //
  if(!OpenDb(admin_config->mysqlDbname(),admin_config->mysqlUsername(),
	     admin_config->mysqlPassword(),admin_config->mysqlHostname(),
	     admin_config->mysqlDriver())) {
    return 1;
  }

  return 0;
}


int main(int argc,char *argv[])
{
  int ret;
  bool found_check_db=false;

  for(int i=0;i<argc;i++) {
    if(!strcmp(argv[i],"--check-db")) {
      found_check_db=true;
    }
  }
  if(found_check_db) {
    ret=cmdline_main(argc,argv);
  }
  else {
    ret=gui_main(argc,argv);
  }
  return ret;
}
