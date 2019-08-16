// controlconnect.cpp
//
// (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
// Abstract a NetISDN Control Connection
//
//  $id:$
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

#include <openssl/md5.h>

#include <qstringlist.h>

#include <netconf.h>
#include <controlconnect.h>

ControlConnect::ControlConnect(QObject *parent)
  : QObject(parent)
{
  conn_tcp_port=0;
  conn_connected_once=false;
  conn_socket=new QSocket(this);
  connect(conn_socket,SIGNAL(connected()),this,SLOT(connectedData()));
  connect(conn_socket,SIGNAL(connectionClosed()),
	  this,SLOT(connectionClosedData()));
  connect(conn_socket,SIGNAL(readyRead()),this,SLOT(readyReadData()));
  connect(conn_socket,SIGNAL(error(int)),this,SLOT(errorData(int)));

  //
  // Timers
  //
  conn_reauthenticate_timer=new QTimer(this);
  connect(conn_reauthenticate_timer,SIGNAL(timeout()),
	  this,SLOT(connectedData()));
  conn_reconnect_timer=new QTimer(this);
  connect(conn_reconnect_timer,SIGNAL(timeout()),this,SLOT(reconnectData()));
}


void ControlConnect::connectToHost(const QString &username,
				   const QString &passwd,
				   const QString &hostname,Q_UINT16 tcp_port)
{
  conn_username=username;
  conn_password=passwd;
  conn_hostname=hostname;
  conn_tcp_port=tcp_port;
  if(conn_socket->state()==QSocket::Connected) {
    connectedData();
  }
  else {
    conn_socket->connectToHost(hostname,tcp_port);
  }
}


void ControlConnect::initiateCall(const QString &username,
				  AudioSettings *settings)
{
  SendCommand("CA "+username+
	      QString().
	      sprintf(" %u %u %u %u %u %u %u %u %u %u %u %u!",
		      settings->format(AudioSettings::Transmit),
		      settings->sampleRate(),
		      settings->channels(AudioSettings::Transmit),
		      settings->bitRate(AudioSettings::Transmit),
		      settings->enableProcessor(AudioSettings::Transmit),
		      settings->streamRatio(AudioSettings::Transmit),
		      settings->format(AudioSettings::Receive),
		      settings->sampleRate(),
		      settings->channels(AudioSettings::Receive),
		      settings->bitRate(AudioSettings::Receive),
		      settings->enableProcessor(AudioSettings::Receive),
		      settings->streamRatio(AudioSettings::Receive)));
}


void ControlConnect::endCall()
{
  SendCommand("BY!");
}


QString ControlConnect::passwordHash(const QString &passwd,const QString &chal)
{
  char plaintext[2048];
  unsigned char hashdata[MD5_DIGEST_LENGTH];
  QString ret;

  sprintf(plaintext,"%s|%s",(const char *)chal,(const char *)passwd);
  MD5((unsigned char *)plaintext,strlen(plaintext),hashdata);
  for(int i=0;i<MD5_DIGEST_LENGTH;i++) {
    ret+=QString().sprintf("%02x",hashdata[i]&0xff);
  }
  return ret;
}


void ControlConnect::sendMetadata(ControlConnect::MetadataField mf,
				  const QString &str)
{
  switch(mf) {
    case ControlConnect::MetadataName:
      SendCommand("MD FN "+NetStringToBase64(str)+"!");
      break;

    case ControlConnect::MetadataEmail:
      SendCommand("MD MA "+NetStringToBase64(str)+"!");
      break;

    case ControlConnect::MetadataPhone:
      SendCommand("MD PH "+NetStringToBase64(str)+"!");
      break;

    case ControlConnect::MetadataLocation:
    case ControlConnect::MetadataUnknown:
    case ControlConnect::MetadataLast:
      break;
  }
}


void ControlConnect::sendCodebook(const QString &start_pkt,
				  const QString &comment_pkt,
				  const QString &codebook_pkt)
{
  SendCommand("CB "+start_pkt+" "+comment_pkt+" "+codebook_pkt+"!");
}


void ControlConnect::connectedData()
{
  conn_connected_once=true;
#ifdef WIN32
  SendCommand("LI "+conn_username+" "+conn_socket->address().toString()+" "+
	      " windows-"+VERSION+"!");
#else
  SendCommand("LI "+conn_username+" "+conn_socket->address().toString()+" "+
	      " linux-"+VERSION+"!");
#endif  // WIN32
}


void ControlConnect::reconnectData()
{
  connectToHost(conn_username,conn_password,conn_hostname,conn_tcp_port);
}


void ControlConnect::connectionClosedData()
{
  if(conn_reauthenticate_timer->isActive()) {
    conn_reauthenticate_timer->stop();
  }
  conn_reconnect_timer->start(NETSIP_RECONNECT_INTERVAL*1000,true);
}


void ControlConnect::readyReadData()
{
  char buf[256];
  int n;

  while((n=conn_socket->readBlock(buf,256))>0) {
    for(int i=0;i<n;i++) {
      if(buf[i]=='!') {
	ParseCommand();
      }
      else {
	conn_cmdline+=buf[i];
      }
    }
  }
}


void ControlConnect::errorData(int err)
{
  if(conn_connected_once) {
    connectionClosedData();
    return;
  }
  emit networkErrorReceived((QSocket::Error)err);
}


void ControlConnect::Challenge(QStringList *cmd)
{
  if(cmd->size()!=2) {
    return;
  }
  SendCommand(QString().sprintf("PW %s!",
	(const char *)ControlConnect::passwordHash(conn_password,(*cmd)[1])));
}


void ControlConnect::Login(QStringList *cmd)
{
  bool ok=false;

  if(cmd->size()!=2) {
    emit responseReceived(NetConfig::ResponseMalformed);
    return;
  }
  int secs=(*cmd)[1].toInt(&ok);
  if(!ok) {
    emit responseReceived(NetConfig::ResponseMalformed);
    return;
  }
  conn_reauthenticate_timer->start(secs*1000,true);
  emit responseReceived(NetConfig::ResponseLoginOk);
}


void ControlConnect::Response(QStringList *cmd)
{
  if(cmd->size()!=2) {
    return;
  }
  emit responseReceived((NetConfig::ResponseCode)(*cmd)[1].toUInt());
}


void ControlConnect::MediaStart(QStringList *cmd)
{
  if(cmd->size()!=15) {
    return;
  }
  QHostAddress addr;
  Q_UINT16 src_port;
  Q_UINT16 dest_port;
  bool ok=false;
  addr.setAddress((*cmd)[3]);
  if(addr.isNull()) {
    return;
  }
  src_port=(*cmd)[2].toInt(&ok);
  if(!ok) {
    return;
  }
  dest_port=(*cmd)[4].toInt(&ok);
  if(!ok) {
    return;
  }
  AudioSettings *settings=new AudioSettings();
  settings->setUsername((*cmd)[1]);
  settings->setFormat(AudioSettings::Transmit,
		      (AudioSettings::Format)((*cmd)[5].toUInt()));
  settings->setSampleRate((*cmd)[6].toUInt());
  settings->setChannels(AudioSettings::Transmit,(*cmd)[7].toUInt());
  settings->setBitRate(AudioSettings::Transmit,(*cmd)[8].toUInt());
  settings->setEnableProcessor(AudioSettings::Transmit,(*cmd)[9].toUInt());
  settings->setStreamRatio(AudioSettings::Transmit,(*cmd)[10].toUInt());

  settings->setFormat(AudioSettings::Receive,
		      (AudioSettings::Format)((*cmd)[11].toUInt()));
  settings->setChannels(AudioSettings::Receive,(*cmd)[13].toUInt());
  settings->setBitRate(AudioSettings::Receive,(*cmd)[14].toUInt());
#ifndef HAVE_MPEG_L3
  if((settings->format(AudioSettings::Receive)==AudioSettings::Layer3)||
     (settings->format(AudioSettings::Receive)==AudioSettings::Layer3)) {
    delete settings;
    SendCommand(QString().sprintf("ER %d!",
				  NetConfig::ResponseFormatUnsupported));
    return;
  }
#endif  // HAVE_MPEG_L3
  emit mediaStartReceived(src_port,addr,dest_port,settings);
  delete settings;
}


void ControlConnect::MediaStop(QStringList *cmd)
{
  emit mediaStopReceived();
}


void ControlConnect::CodeBook(QStringList *cmd)
{
  if(cmd->size()!=4) {
    return;
  }
  emit codebookReceived((*cmd)[1],(*cmd)[2],(*cmd)[3]);
}


void ControlConnect::Metadata(QStringList *cmd)
{
  switch(cmd->size()) {
    case 2:
      if((*cmd)[1]=="FN") {
	emit metadataReceived(ControlConnect::MetadataName,"");
      }
      if((*cmd)[1]=="MA") {
	emit metadataReceived(ControlConnect::MetadataEmail,"");
      }
      if((*cmd)[1]=="PH") {
	emit metadataReceived(ControlConnect::MetadataPhone,"");
      }
      if((*cmd)[1]=="LC") {
	emit metadataReceived(ControlConnect::MetadataLocation,"");
      }
      break;

    case 3:
      if((*cmd)[1]=="FN") {
	emit metadataReceived(ControlConnect::MetadataName,
			      NetBase64ToString((*cmd)[2]));
      }
      if((*cmd)[1]=="MA") {
	emit metadataReceived(ControlConnect::MetadataEmail,
			      NetBase64ToString((*cmd)[2]));
      }
      if((*cmd)[1]=="PH") {
	emit metadataReceived(ControlConnect::MetadataPhone,
			      NetBase64ToString((*cmd)[2]));
      }
      if((*cmd)[1]=="LC") {
	emit metadataReceived(ControlConnect::MetadataLocation,
			      NetBase64ToString((*cmd)[2]));
      }
      break;
  }
}


void ControlConnect::ParseCommand()
{
  QStringList cmds=QStringList::split(" ",conn_cmdline);

  // printf("CMDLINE: %s\n",(const char *)conn_cmdline);

  conn_cmdline="";

  if(cmds.size()<1) {
    return;
  }
  if(cmds[0]=="LI") {  // Challenge
    Challenge(&cmds);
  }
  if(cmds[0]=="OK") {  // Login Successful Code
    Login(&cmds);
  }
  if(cmds[0]=="ER") {  // Error Code
    Response(&cmds);
  }
  if(cmds[0]=="ME") {  // Media Start
    MediaStart(&cmds);
  }
  if(cmds[0]=="MS") {  // Media Stop
    MediaStop(&cmds);
  }
  if(cmds[0]=="MD") {  // Metadata
    Metadata(&cmds);
  }
  if(cmds[0]=="CB") {  // Vorbis Initialization Data
    CodeBook(&cmds);
  }
  if(cmds[0]=="PG") {  // Ping
    SendCommand("PG!");
  }
}


void ControlConnect::SendCommand(const QString &cmd)
{
  // printf("SendCommand(%s)\n",(const char *)cmd);
  conn_socket->writeBlock(cmd,cmd.length());
}
