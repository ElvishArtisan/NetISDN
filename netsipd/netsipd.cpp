// netsipd.cpp
//
// SIP server for NetISDN.
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of
//   the License, or (at your option) any later version.
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

#include <signal.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qobject.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <vector>
#include <syslog.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>

#include <qdatetime.h>
#include <qmessagebox.h>

#include <escape_string.h>
#include <controlconnect.h>
#include <netconf.h>
#include <netconfig.h>
#include <sqldb.h>

#include <netsipd.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif  // HOST_NAME_MAX

//
// Global Variables
//
volatile bool restart=false;


void SigHandler(int signum)
{
  switch(signum) {
      case SIGHUP:
	restart=true;
	break;

      case SIGINT:
      case SIGTERM:
	syslog(LOG_DAEMON|LOG_NOTICE,"shutting down normally");
	closelog();
	unlink(NETSIPD_PID_FILE);
	exit(0);
	break;
  }
}


MainObject::MainObject(QObject *parent,const char *name)
  :QObject(parent,name)
{
  struct hostent *hostent;

  //
  // Open the Syslog Connection
  //
  openlog("netsipd",0,LOG_DAEMON);

  //
  // Read Command Switches
  //
  sip_debug=false;
  for(int i=1;i<qApp->argc();i++) {
    if(qApp->argv()[i]==QString("-d")) {
      sip_debug=true;
    }
  }

  //
  // Network Status Parameters
  //
  memset(&sip_status_sockaddr,0,sizeof(sip_status_sockaddr));
  sip_status_sockaddr.sin_family=AF_INET;
  sip_status_sockaddr.sin_port=htons(NETSIP_STATUS_MCAST_PORT);
  hostent=gethostbyname(NETSIP_STATUS_MCAST_ADDR);
  if(hostent!=NULL) {
    sip_status_sockaddr.sin_addr.s_addr=*((u_int32_t *)hostent->h_addr);
  }

  Init();

  //
  // Detach
  //
  if(!sip_debug) {
    daemon(0,0);
  }

  //
  // Log PID
  //
  FILE *pidfile;
  if((pidfile=fopen(NETSIPD_PID_FILE,"w"))==NULL) {
    perror("netsipd");
  }
  else {
    fprintf(pidfile,"%d",getpid());
    fclose(pidfile);
  }

  //
  // Initialize random number generator
  //
  time_t t;
  t=time(&t);
  srand(t);

  signal(SIGHUP,SigHandler);
  signal(SIGINT,SigHandler);
  signal(SIGTERM,SigHandler);
}


void MainObject::newConnection(int fd)
{
  // printf("starting up connection fd=%d\n",fd);
  sip_connections[fd]=new SipConnection(fd);
  sip_ready_mapper->setMapping(sip_connections[fd]->socket(),fd);
  connect(sip_connections[fd]->socket(),SIGNAL(readyRead()),
	  sip_ready_mapper,SLOT(map()));
  sip_kill_mapper->setMapping(sip_connections[fd]->socket(),fd);
  connect(sip_connections[fd]->socket(),SIGNAL(connectionClosed()),
	  sip_kill_mapper,SLOT(map()));
}


void MainObject::socketReady(int fd)
{
  char buf[256];
  int n;

  while((n=sip_connections[fd]->socket()->readBlock(buf,256))>0) {
    for(int i=0;i<n;i++) {
      if(buf[i]=='!') {
	ParseCommand(fd);
      }
      else {
	sip_connections[fd]->cmdline+=buf[i];
      }
    }
  }
}


void MainObject::socketKill(int fd)
{
  if(!sip_connections[fd]->zombie()) {
    // printf("dropping connection fd=%d,uid=%d\n",fd,sip_connections[fd]->uid());
    Logout(fd);
    sip_connections[fd]->setZombie(true);
    sip_zombie_timer->start(10,true);
  }
}


void MainObject::zombieData()
{
  for(std::map<int,SipConnection *>::iterator ci=sip_connections.begin();
      ci!=sip_connections.end();++ci) {
    if(ci->second->zombie()) {
      ci->second->socket()->disconnect();
      delete ci->second;
      sip_connections.erase(ci);
    }
  }
}


void MainObject::heartbeatData()
{
  QString sql;
  NetSqlQuery *q;

  sql="select DB from VERSION";
  q=new NetSqlQuery(sql);
  if(!q->first()) {
    syslog(LOG_DAEMON|LOG_ERR,"unable to access database");
  }
  delete q;
}


void MainObject::pingTimerData()
{
  QDateTime dt=QDateTime(QDate::currentDate(),QTime::currentTime()).
    addSecs(-NETSIPD_PING_TIMEOUT/1000);
  for(std::map<int,SipConnection *>::iterator ci=sip_connections.begin();
      ci!=sip_connections.end();++ci) {
    if(ci->second->pingTimestamp()<dt) {
      Logout(ci->first);
      ci->second->socket()->disconnect();
      delete ci->second;
      sip_connections.erase(ci);
    }
    else {
      SendCommand(ci->first,"PG!");
    }
  }
}


void MainObject::Login(int fd,QStringList *cmd)
{
  QString sql;
  NetSqlQuery *q;
  QHostAddress addr;

  if(cmd->size()!=4) {
    SendCommand(fd,QString().sprintf("ER %u!",
				     NetConfig::ResponseMalformed));
    socketKill(fd);
    return;
  }
  
  //
  // Get UID
  //
  QString chal=GenerateChallenge();
  SendCommand(fd,"LI "+chal+"!");
  sql="select ID,NAME,PASSWORD from USERS where NAME=\""+(*cmd)[1]+"\"";
  q=new NetSqlQuery(sql);
  if(q->first()) {
    sip_connections[fd]->setUid(q->value(0).toInt());
    sip_connections[fd]->setUsername(q->value(1).toString());
    addr.setAddress((*cmd)[2]);
    sip_connections[fd]->setLocalAddress(addr);
    sip_connections[fd]->setClientVersion((*cmd)[3]);
    sip_connections[fd]->
      setHash(ControlConnect::passwordHash(q->value(2).toString(),chal));
  }
  delete q;
}


void MainObject::Password(int fd,QStringList *cmd)
{
  QString sql;
  NetSqlQuery *q;
  GeoIPRecord *georec;
  QString location=tr("Private/Unknown");

  if(cmd->size()!=2) {
    SendCommand(fd,QString().sprintf("ER %u!",
				     NetConfig::ResponseMalformed));
    socketKill(fd);
    return;
  }
  if(sip_connections[fd]->uid()==0) {  // User invalid
    SendCommand(fd,QString().sprintf("ER %u!",
				     NetConfig::ResponseUserInvalid));
    socketKill(fd);
    return;
  }
  if((*cmd)[1]!=sip_connections[fd]->hash()) {  // Password invalid
    SendCommand(fd,QString().sprintf("ER %u!",
				     NetConfig::ResponseUserInvalid));
    socketKill(fd);
    return;
  }
  if(GetFdByUid(sip_connections[fd]->uid(),fd)>=0) {  // Already online
    SendCommand(fd,QString().sprintf("ER %u!",NetConfig::ResponseUserActive));
    socketKill(fd);
    return;
  }
  sip_connections[fd]->setIsAuthenticated(true);

  //
  // Lookup GeoIP Data
  //
  if((georec=GeoIP_record_by_addr(sip_geoip,sip_connections[fd]->socket()->
				  peerAddress().toString()))!=NULL) {
    location=QString(georec->city)+", "+QString(georec->region)+", "+
      QString(georec->country_code);
  }

  sql=QString().sprintf("update USERS set LAST_ACCESS_DATETIME=now(),\
                         LAST_ACCESS_ADDR=\"%s\",ONLINE=\"Y\",\
                         CLIENT_VERSION=\"%s\",LOCATION=\"%s\" where ID=%d",
			(const char *)sip_connections[fd]->socket()->
			peerAddress().toString(),
			(const char *)sip_connections[fd]->clientVersion(),
			(const char *)EscapeString(location),
			sip_connections[fd]->uid());
  q=new NetSqlQuery(sql);
  delete q;
  SendCommand(fd,QString().sprintf("OK %u!",NETSIP_REGISTRATION_INTERVAL));
  SendUidUpdate(sip_connections[fd]->uid());
}


void MainObject::InitiateCall(int fd,QStringList *cmd)
{
  QString sql;
  NetSqlQuery *q;

  if(cmd->size()!=14) {
    SendCommand(fd,QString().sprintf("ER %u!",
				     NetConfig::ResponseMalformed));
    return;
  }
  
  //
  // Get UID
  //
  sql="select ID from USERS where NAME=\""+(*cmd)[1]+"\"";
  q=new NetSqlQuery(sql);
  if(!q->first()) {  // No such user
    SendCommand(fd,QString().sprintf("ER %u!",
				     NetConfig::ResponseCallerUnknown));
    delete q;
    return;
  }
  int dest_uid=q->value(0).toInt();
  delete q;
  
  //
  // Find connection
  //
  int dest_fd=-1;
  for(std::map<int,SipConnection *>::iterator it=sip_connections.begin();
      it!=sip_connections.end();it++) {
    if(it->second->uid()==dest_uid) {
      dest_fd=it->first;
    }
  }
  if(dest_fd<0) {  // User not online
    SendCommand(fd,QString().sprintf("ER %u!",
				     NetConfig::ResponseCallerOffline));
    return;
  }
  if(sip_connections[dest_fd]->connectionFd()>=0) {  // User busy
    SendCommand(fd,QString().sprintf("ER %u!",NetConfig::ResponseCallerBusy));
    return;        
  }
  
  //
  // Setup the connection and log the call
  //
  QDateTime dt=QDateTime(QDate::currentDate(),QTime::currentTime());
  SendCommand(fd,QString().sprintf("ER %u!",
				   NetConfig::ResponseCallProceeding));
  sip_connections[fd]->setConnectionFd(dest_fd);
  sip_connections[dest_fd]->setConnectionFd(fd);

  //
  // Generate addresses and ports
  //
  QHostAddress src_addr;
  QHostAddress dest_addr;
  if(sip_connections[fd]->socket()->peerAddress()==
     sip_connections[dest_fd]->socket()->peerAddress()) {
    src_addr=sip_connections[fd]->localAddress();
    dest_addr=sip_connections[dest_fd]->localAddress();
  }
  else {
    src_addr=sip_connections[fd]->socket()->peerAddress();
    dest_addr=sip_connections[dest_fd]->socket()->peerAddress();
  }
  sip_connections[fd]->setSourcePort(GeneratePort(src_addr));
  sip_connections[dest_fd]->setSourcePort(GeneratePort(dest_addr));

  //
  // Log the call
  //
  sql=QString().sprintf("insert into CALLS set ORIG_UID=%d,\
                         ORIG_ADDR=\"%s\",\
                         ORIG_PORT=%u,\
                         DEST_UID=%d,\
                         DEST_ADDR=\"%s\",\
                         DEST_PORT=%u,\
                         START_DATETIME=\"%s\",\
                         FORMAT=%u,\
                         SAMPLE_RATE=%u,\
                         CHANNELS=%u,\
                         BIT_RATE=%u,\
                         DEST_FORMAT=%u,\
                         DEST_SAMPLE_RATE=%u,\
                         DEST_CHANNELS=%u,\
                         DEST_BIT_RATE=%u",
			sip_connections[fd]->uid(),
			(const char *)src_addr.toString(),
			sip_connections[fd]->sourcePort(),
			sip_connections[dest_fd]->uid(),
			(const char *)dest_addr.toString(),
			sip_connections[dest_fd]->sourcePort(),
			(const char *)dt.toString("yyyy-MM-dd hh:mm:ss"),
			(*cmd)[2].toUInt(),
			(*cmd)[3].toUInt(),
			(*cmd)[4].toUInt(),
			(*cmd)[5].toUInt(),
			(*cmd)[8].toUInt(),
			(*cmd)[9].toUInt(),
			(*cmd)[10].toUInt(),
			(*cmd)[11].toUInt());
  q=new NetSqlQuery(sql);
  delete q;
  sql=QString().sprintf("select ID from CALLS where ORIG_UID=%d \
                         order by START_DATETIME desc",
			sip_connections[fd]->uid());
  q=new NetSqlQuery(sql);
  if(q->first()) {
    sip_connections[fd]->setCid(q->value(0).toInt());
    sip_connections[dest_fd]->setCid(q->value(0).toInt());
  }
  delete q;

  //
  // Start the encoders
  //
  // 0 - "CA"
  // 1 - <username>
  // 2 - <src-format>
  // 3 - <src-samp-rate>
  // 4 - <src-chans>
  // 5 - <src-quality>
  // 6 - <src-proc>
  // 7 - <src-ratio>
  // 8 - <dest-format>
  // 9 - <dest-samp-rate>
  // 10 - <dest-chans>
  // 11 - <dest-quality>
  // 12 - <dest-proc>
  // 13 - <dest-ratio>
  //
  SendCommand(fd,"ME "+sip_connections[dest_fd]->username()+
	      QString().sprintf(" %u ",sip_connections[fd]->sourcePort())+
	      dest_addr.toString()+
	      QString().sprintf(" %u %u %u %u %u %u %u %u %u %u %d!",
				sip_connections[dest_fd]->sourcePort(), // 4
				(*cmd)[2].toUInt(),  // 5
				(*cmd)[3].toUInt(),  // 6
				(*cmd)[4].toUInt(),  // 7
				(*cmd)[5].toUInt(),  // 8
				(*cmd)[6].toUInt(),  // 9
				(*cmd)[7].toUInt(),  // 10
				(*cmd)[8].toUInt(), // 11
				(*cmd)[9].toUInt(), // 12
				(*cmd)[10].toUInt(), // 13
				(*cmd)[11].toUInt())); // 14
  SendCommand(dest_fd,"ME "+sip_connections[fd]->username()+
	      QString().sprintf(" %u ",sip_connections[dest_fd]->sourcePort())+
	      src_addr.toString()+
	      QString().sprintf(" %u %u %u %u %u %u %u %u %u %u %u!",
				sip_connections[fd]->sourcePort(),
				(*cmd)[8].toUInt(),
				(*cmd)[9].toUInt(),
				(*cmd)[10].toUInt(),
				(*cmd)[11].toUInt(),
				(*cmd)[12].toUInt(),
				(*cmd)[13].toUInt(),
				(*cmd)[2].toUInt(),
				(*cmd)[3].toUInt(),
				(*cmd)[4].toUInt(),
				(*cmd)[5].toUInt()));

  //
  // Send metadata
  //
  SendMetadata(sip_connections[fd]->uid(),dest_fd);
  SendMetadata(sip_connections[dest_fd]->uid(),fd);
  SendUidUpdate(sip_connections[fd]->uid());
  SendUidUpdate(sip_connections[dest_fd]->uid());
  SendCidUpdate(sip_connections[fd]->cid());
}


void MainObject::EndCall(int fd)
{
  int dest_fd=-1;
  QString sql;
  NetSqlQuery *q;
  int src_uid=sip_connections[fd]->uid();
  int dest_uid=0;
  int cid;

  if((dest_fd=sip_connections[fd]->connectionFd())>=0) {
    dest_uid=sip_connections[dest_fd]->uid();
    SendCommand(dest_fd,"MS!");
    sip_connections[dest_fd]->setConnectionFd(-1);
    sip_connections[dest_fd]->setCid(-1);
    sip_connections[dest_fd]->setSourcePort(0);
  }
  cid=sip_connections[fd]->cid();
  sql=QString().sprintf("update CALLS set END_DATETIME=now() where ID=%d",
			sip_connections[fd]->cid());
  q=new NetSqlQuery(sql);
  delete q;
  SendCommand(fd,"MS!");
  sip_connections[fd]->setConnectionFd(-1);
  sip_connections[fd]->setCid(-1);  
  sip_connections[fd]->setSourcePort(0);
  SendUidUpdate(src_uid);
  SendUidUpdate(dest_uid);
  SendCidUpdate(cid);
}


void MainObject::ForwardMessage(int fd,QStringList *cmd)
{
  if(sip_connections[fd]->connectionFd()<0) {
    return;
  }
  QString msg;
  for(unsigned i=0;i<cmd->size();i++) {
    msg+=((*cmd)[i]+" ");
  }
  msg=msg.left(msg.length()-1)+"!";
  SendCommand(sip_connections[fd]->connectionFd(),msg);
}


void MainObject::ReceiveMetadata(int fd,QStringList *cmd)
{
  QString str;
  QString sql;
  NetSqlQuery *q;

  switch(cmd->size()) {
    case 2:
      break;

    case 3:
      str=(*cmd)[2];
      break;

    default:
      return;
  }
  if((*cmd)[1]=="FN") {
    sql="update USERS set FULL_NAME=\""+NetBase64ToString((*cmd)[2])+
      "\" where ID="+QString().sprintf("%d",sip_connections[fd]->uid());
    q=new NetSqlQuery(sql);
    delete q;
  }
  if((*cmd)[1]=="MA") {
    sql="update USERS set EMAIL=\""+NetBase64ToString((*cmd)[2])+
      "\" where ID="+QString().sprintf("%d",sip_connections[fd]->uid());
    q=new NetSqlQuery(sql);
    delete q;
  }
  if((*cmd)[1]=="PH") {
    sql="update USERS set PHONE_NUMBER=\""+NetBase64ToString((*cmd)[2])+
      "\" where ID="+QString().sprintf("%d",sip_connections[fd]->uid());
    q=new NetSqlQuery(sql);
    delete q;
  }
  SendUidUpdate(sip_connections[fd]->uid());
}


void MainObject::Logout(int fd)
{
  QString sql;
  NetSqlQuery *q;
  int uid;

  //
  // Clear a user, *without* closing the socket connection
  //
  if((!sip_connections[fd]->isAuthenticated())||
     (sip_connections[fd]->uid()<=0)) {
    return;
  }
  if(sip_connections[fd]->cid()>=0) {
    EndCall(fd);
  }
  if(sip_connections[fd]->connectionFd()>=0) {
    sip_connections[sip_connections[fd]->connectionFd()]->setConnectionFd(-1);
  }
  sip_connections[fd]->setConnectionFd(-1);
  sql=QString().sprintf("update USERS set LAST_ACCESS_DATETIME=now(),\
                         ONLINE=\"N\" where ID=%d",
			sip_connections[fd]->uid());
  q=new NetSqlQuery(sql);
  delete q;
  uid=sip_connections[fd]->uid();
  sip_connections[fd]->setUid(0);
  sip_connections[fd]->setUsername("");
  sip_connections[fd]->setHash("");
  sip_connections[fd]->setLocalAddress(QHostAddress());
  SendUidUpdate(uid);
}


void MainObject::SendUidUpdate(int uid)
{
  char buf[32];
  sprintf(buf,"u%d",uid);
  sendto(sip_status_socket,buf,strlen(buf),0,
	 (sockaddr *)&sip_status_sockaddr,sizeof(sip_status_sockaddr));
}


void MainObject::SendCidUpdate(int cid)
{
  char buf[32];
  sprintf(buf,"c%d",cid);
  sendto(sip_status_socket,buf,strlen(buf),0,
	 (sockaddr *)&sip_status_sockaddr,sizeof(sip_status_sockaddr));
}


void MainObject::SendMetadata(int id,int fd)
{
  QString sql;
  NetSqlQuery *q;

  sql=QString().sprintf("select FULL_NAME,PHONE_NUMBER,EMAIL,LOCATION \
                         from USERS where ID=%d",id);
  q=new NetSqlQuery(sql);
  if(q->first()) {
    SendCommand(fd,"MD FN "+NetStringToBase64(q->value(0).toString())+"!");
    SendCommand(fd,"MD MA "+NetStringToBase64(q->value(1).toString())+"!");
    SendCommand(fd,"MD PH "+NetStringToBase64(q->value(2).toString())+"!");
    SendCommand(fd,"MD LC "+NetStringToBase64(q->value(3).toString())+"!");
  }
  delete q;
}


void MainObject::UpdatePing(int id)
{
  sip_connections[id]->
    setPingTimestamp(QDateTime(QDate::currentDate(),QTime::currentTime()));
}


void MainObject::ParseCommand(int fd)
{
  SipConnection *conn=sip_connections[fd];
  QStringList cmds=QStringList::split(" ",conn->cmdline.stripWhiteSpace());

  //printf("CMDLINE: %s\n",(const char *)conn->cmdline);

  conn->cmdline="";

  if(cmds[0]=="LI") {   // Login
    Login(fd,&cmds);
    return;
  }
  if(cmds[0]=="PW") {   // Challenge response
    Password(fd,&cmds);
    return;
  }
  if(!conn->isAuthenticated()) {  // All other commands require a login
    SendCommand(fd,QString().sprintf("ER %u!",
				     NetConfig::ResponseAuthenticationNeeded));
    socketKill(fd);
    return;
  }
  if(cmds[0]=="CA") {   // Initiate Call
    InitiateCall(fd,&cmds);
    return;
  }
  if(cmds[0]=="BY") {   // End Call
    EndCall(fd);
    return;
  }
  if(cmds[0]=="CB") {   // Forward Vorbis Initialization Data
    ForwardMessage(fd,&cmds);
    return;
  }
  if(cmds[0]=="MD") {   // Metadata Update
    ReceiveMetadata(fd,&cmds);
    return;
  }
  if(cmds[0]=="ER") {   // Forward Error Message
    ForwardMessage(fd,&cmds);
    return;
  }
  if(cmds[0]=="PG") {   // Ping Response
    UpdatePing(fd);
    return;
  }
  SendCommand(fd,QString().sprintf("ER %u!",NetConfig::ResponseUnknownCommand));
}


void MainObject::SendCommand(int fd,const QString &cmd)
{
  sip_connections[fd]->socket()->writeBlock(cmd,cmd.length());
}



int MainObject::GetFdByUid(int uid,int except_fd) const
{
  for(std::map<int,SipConnection *>::const_iterator it=sip_connections.begin();
      it!=sip_connections.end();it++) {
    if((it->second->uid()==uid)&&(it->first!=except_fd)) {
      return it->first;
    }
  }
  return -1;
}


QString MainObject::GenerateChallenge()
{
  QString ret;

  for(unsigned i=0;i<4;i++) {
    ret+=QString().sprintf("%08x",rand());
  }
  return ret;
}


Q_UINT16 MainObject::GeneratePort(const QHostAddress &addr)
{
  Q_UINT16 port=NETSIP_RTP_PORT;
  bool found=true;

  while(found) {
    found=false;
    for(std::map<int,SipConnection *>::const_iterator it=
	  sip_connections.begin();
	it!=sip_connections.end();it++) {
      if((it->second->socket()->peerAddress()==addr)&&
	 (it->second->sourcePort()==port)) {
	port+=2;
	found=true;
      }
    }
  }

  return port;
}


void MainObject::Init(bool initial_startup)
{
  sockaddr_in sa;

  //
  // Load Configuration
  //
  sip_config=NetConfiguration(); 
  sip_config->load();

  //
  // GeoIP Database
  //
  QString geoip_path=QString(NETSIP_GEOIP_DATADIR)+QString("/GeoLiteCity.dat");
  if((sip_geoip=GeoIP_open(geoip_path,GEOIP_STANDARD|GEOIP_CHECK_CACHE))==
     NULL) {
    syslog(LOG_DAEMON|LOG_CRIT,"unable to open geoip database, exiting");
    if(initial_startup||sip_debug) {
      fprintf(stderr,"netsipd: unable to open geoip database\n");
    }
    exit(256);
  }
    
  //
  // Start the Database Connection
  //
  QString err;
  QSqlDatabase *db = NetInitDb (&err);
  if(!db) {
    exit(0);
  }

  //
  // Start the Status Socket
  //
  sip_status_socket=socket(PF_INET,SOCK_DGRAM,0);
  struct hostent *hostent=gethostbyname(sip_config->globalInterface());
  u_int32_t addr=htonl(INADDR_ANY);
  if(hostent!=NULL ) {
    addr=*(u_int32_t *)hostent->h_addr;
  }
  memset(&sa,0,sizeof(sa));
  sa.sin_family=AF_INET;
  sa.sin_port=0;
  sa.sin_addr.s_addr=addr;
  bind(sip_status_socket,(sockaddr *)&sa,sizeof(sa));

  //
  // Start Up Server Socket
  //
  sip_ready_mapper=new QSignalMapper(this);
  connect(sip_ready_mapper,SIGNAL(mapped(int)),this,SLOT(socketReady(int)));
  sip_kill_mapper=new QSignalMapper(this);
  connect(sip_kill_mapper,SIGNAL(mapped(int)),this,SLOT(socketKill(int)));
  sip_server=new ServerSocket(NETSIP_TCP_PORT,0,this);
  if(!sip_server->ok()) {
    syslog(LOG_DAEMON|LOG_CRIT,"unable to bind server socket, exiting");
    exit(256);
    if(initial_startup||sip_debug) {
      fprintf(stderr,"netsipd: unable to bind server socket\n");
    }
  }
  connect(sip_server,SIGNAL(connection(int)),this,SLOT(newConnection(int)));
  sip_zombie_timer=new QTimer(this,"sip_zombie_timer");
  connect(sip_zombie_timer,SIGNAL(timeout()),this,SLOT(zombieData()));

  //
  // Start MySQL Heartbeat
  //
  sip_heartbeat_timer=new QTimer(this);
  connect(sip_heartbeat_timer,SIGNAL(timeout()),this,SLOT(heartbeatData()));
  sip_heartbeat_timer->start(NETSIPD_HEARTBEAT_INTERVAL);

  //
  // Start Ping Timer
  //
  sip_ping_timer=new QTimer(this);
  connect(sip_ping_timer,SIGNAL(timeout()),this,SLOT(pingTimerData()));
  sip_ping_timer->start(NETSIPD_PING_INTERVAL);

  syslog(LOG_DAEMON|LOG_INFO,"started normally");
}


void MainObject::Release()
{
  //
  // Stop Ping Timer
  //
  sip_ping_timer->stop();
  delete sip_ping_timer;

  //
  // Stop Heartbeat
  //
  sip_heartbeat_timer->stop();
  delete sip_heartbeat_timer;

  //
  // Shut down network connections
  //
  for(std::map<int,SipConnection *>::iterator ci=sip_connections.begin();
      ci!=sip_connections.end();++ci) {
    delete ci->second;
  }
  sip_connections.clear();
  delete sip_zombie_timer;
  sip_zombie_timer=NULL;
  delete sip_server;
  sip_server=NULL;
  delete sip_ready_mapper;
  sip_ready_mapper=NULL;
  delete sip_kill_mapper;
  sip_kill_mapper=NULL;
  close(sip_status_socket);
  sip_status_socket=-1;

  //
  // Shut Down Database Connection
  //
  NetFreeDb();
/*
  sip_database->removeDatabase(sip_config->mysqlDbname());
  sip_database=NULL;
*/

  //
  // Shutdown GeoIP
  //
  GeoIP_delete(sip_geoip);
  sip_geoip=NULL;

  //
  // Free Configuration
  //
  delete sip_config;
  sip_config=NULL;
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv,false);
  new MainObject(NULL,"main");
  return a.exec();
}
