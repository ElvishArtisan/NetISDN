// sip_connection.h
//
// Container class for SIP proxy connections.
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


#ifndef SIP_CONNECTION_H
#define SIP_CONNECTION_H

#include <qsocket.h>
#include <qstring.h>
#include <qhostaddress.h>
#include <qdatetime.h>

class SipConnection
{
 public:
  SipConnection(int fd);
  ~SipConnection();
  QString cmdline;
  QSocket *socket();
  int uid() const;
  void setUid(int uid);
  int cid() const;
  void setCid(int cid);
  QString username() const;
  void setUsername(const QString &str);
  QString hash() const;
  void setHash(const QString &str);
  int connectionFd() const;
  void setConnectionFd(int fd);
  Q_UINT16 sourcePort() const;
  void setSourcePort(Q_UINT16 port);
  QHostAddress localAddress() const;
  void setLocalAddress(const QHostAddress &addr);
  QString clientVersion() const;
  void setClientVersion(const QString &str);
  bool zombie() const;
  void setZombie(bool state);
  bool isAuthenticated() const;
  void setIsAuthenticated(bool state);
  QDateTime pingTimestamp() const;
  void setPingTimestamp(const QDateTime &dt);
  void clear();

 private:
  QSocket *conn_socket;
  int conn_uid;
  int conn_cid;
  QString conn_username;
  QString conn_hash;
  int conn_connection_fd;
  Q_UINT16 conn_source_port;
  QHostAddress conn_local_address;
  QString conn_client_version;
  bool conn_zombie;
  bool conn_is_authenticated;
  QDateTime conn_ping_timestamp;
};


#endif  // SIP_CONNECTION_H
