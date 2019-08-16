// netsipd.h
//
// SIP server for NetISDN
//
//   (C) Copyright 2008-2009 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef NETSIPD_H
#define NETSIPD_H

#include <map>
#include <vector>

#include <netinet/ip.h>
#include <netinet/in.h>

#include <GeoIPCity.h>

#include <qobject.h>
#include <qserversocket.h>
#include <qsqldatabase.h>
#include <qsignalmapper.h>
#include <qsocketdevice.h>

#include <netconfig.h>

#include <sipconnection.h>
#include <serversocket.h>

#define NETSIPD_PID_FILE "/var/run/netsipd.pid"
#define NETSIPD_HEARTBEAT_INTERVAL 60000
#define NETSIPD_PING_INTERVAL 2000
#define NETSIPD_PING_TIMEOUT 6000

class MainObject : public QObject
{
  Q_OBJECT
 public:
  MainObject(QObject *parent=0,const char *name=0);

 private slots:
  void newConnection(int fd);
  void socketReady(int fd);
  void socketKill(int fd);
  void zombieData();
  void heartbeatData();
  void pingTimerData();

 private:
  void Login(int fd,QStringList *cmd);
  void Password(int fd,QStringList *cmd);
  void InitiateCall(int fd,QStringList *cmd);
  void EndCall(int fd);
  void ForwardMessage(int fd,QStringList *cmd);
  void ReceiveMetadata(int fd,QStringList *cmd);
  void Logout(int fd);
  void SendUidUpdate(int uid);
  void SendCidUpdate(int cid);
  void SendMetadata(int id,int fd);
  void UpdatePing(int id);
  void ParseCommand(int fd);
  void SendCommand(int fd,const QString &cmd);
  int GetFdByUid(int uid,int except_fd=-1) const;
  QString GenerateChallenge();
  Q_UINT16 GeneratePort(const QHostAddress &addr);
  void Init(bool initial_startup=false);
  void Release();
  bool sip_debug;
  std::map<int,SipConnection *> sip_connections;
  QSignalMapper *sip_ready_mapper;
  QSignalMapper *sip_kill_mapper;
  QTimer *sip_zombie_timer;
  NetConfig *sip_config;
  QSqlDatabase *sip_database;
  ServerSocket *sip_server;
  GeoIP *sip_geoip;
  QTimer *sip_heartbeat_timer;
  int sip_status_socket;
  sockaddr_in sip_status_sockaddr;
  QTimer *sip_ping_timer;
};


#endif  // NETSIPD_H
