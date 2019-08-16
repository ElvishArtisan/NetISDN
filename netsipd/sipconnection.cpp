// sip_connection.cpp
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

#include <sipconnection.h>

SipConnection::SipConnection(int fd)
{
  clear();
  conn_socket=new QSocket();
  conn_socket->setSocket(fd);
}


SipConnection::~SipConnection()
{
  delete conn_socket;
}


QSocket *SipConnection::socket()
{
  return conn_socket;
}


int SipConnection::uid() const
{
  return conn_uid;
}


void SipConnection::setUid(int uid)
{
  conn_uid=uid;
}


int SipConnection::cid() const
{
  return conn_cid;
}


void SipConnection::setCid(int cid)
{
  conn_cid=cid;
}


QString SipConnection::username() const
{
  return conn_username;
}


void SipConnection::setUsername(const QString &str)
{
  conn_username=str;
}


QString SipConnection::hash() const
{
  return conn_hash;
}


void SipConnection::setHash(const QString &str)
{
  conn_hash=str;
}


int SipConnection::connectionFd() const
{
  return conn_connection_fd;
}


void SipConnection::setConnectionFd(int fd)
{
  conn_connection_fd=fd;
}


Q_UINT16 SipConnection::sourcePort() const
{
  return conn_source_port;
}


void SipConnection::setSourcePort(Q_UINT16 port)
{
  conn_source_port=port;
}


QHostAddress SipConnection::localAddress() const
{
  return conn_local_address;
}


void SipConnection::setLocalAddress(const QHostAddress &addr)
{
  conn_local_address=addr;
}


QString SipConnection::clientVersion() const
{
  return conn_client_version;
}


void SipConnection::setClientVersion(const QString &str)
{
  conn_client_version=str;
}


bool SipConnection::zombie() const
{
  return conn_zombie;
}


void SipConnection::setZombie(bool state)
{
  conn_zombie=state;
}


bool SipConnection::isAuthenticated() const
{
  return conn_is_authenticated;
}


void SipConnection::setIsAuthenticated(bool state)
{
  conn_is_authenticated=state;
}


QDateTime SipConnection::pingTimestamp() const
{
  return conn_ping_timestamp;
}


void SipConnection::setPingTimestamp(const QDateTime &dt)
{
  conn_ping_timestamp=dt;
}


void SipConnection::clear()
{
  conn_socket=NULL;
  conn_uid=0;
  conn_cid=-1;
  conn_username="";
  conn_connection_fd=-1;
  conn_source_port=0;
  conn_local_address=QHostAddress();
  conn_client_version="0.0.0";
  conn_zombie=false;
  conn_is_authenticated=false;
  conn_ping_timestamp=QDateTime(QDate::currentDate(),QTime::currentTime());
};
