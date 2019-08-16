// controlconnect.h
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


#ifndef CONTROLCONNECT_H
#define CONTROLCONNECT_H

#include <sys/types.h>
#include <vector>
#ifdef WIN32
#include <win32_types.h>
#endif

#include <qobject.h>
#include <qsocket.h>
#include <qstringlist.h>
#include <qhostaddress.h>
#include <qtimer.h>

#include <audiosettings.h>
#include <netconfig.h>

class ControlConnect : public QObject
{
  Q_OBJECT;
 public:
  enum MetadataField {MetadataUnknown=0,MetadataName=1,MetadataEmail=2,
		      MetadataPhone=3,MetadataLocation=4,MetadataLast=5};
  ControlConnect(QObject *parent);
  void connectToHost(const QString &username,const QString &passwd,
		     const QString &hostname,Q_UINT16 tcp_port);
  void initiateCall(const QString &username,AudioSettings *settings);
  void endCall();
  static QString passwordHash(const QString &passwd,const QString &chal);

 public slots:
  void sendMetadata(ControlConnect::MetadataField mf,const QString &str);
  void sendCodebook(const QString &start_pkt,const QString &comment_pkt,
		    const QString &codebook_pkt);

 signals:
  void responseReceived(NetConfig::ResponseCode resp);
  void mediaStartReceived(Q_UINT16 src_port,const QHostAddress &addr,
			  Q_UINT16 dest_port,AudioSettings *settings);
  void mediaStopReceived();
  void codebookReceived(const QString &start_pkg,const QString &comment_pkt,
			const QString &codebook_pkg);
  void metadataReceived(ControlConnect::MetadataField mf,const QString &str);
  void networkErrorReceived(QSocket::Error err);

 private slots:
  void connectedData();
  void reconnectData();
  void connectionClosedData();
  void readyReadData();
  void errorData(int err);

 private:
  void Challenge(QStringList *cmd);
  void Login(QStringList *cmd);
  void Response(QStringList *cmd);
  void MediaStart(QStringList *cmd);
  void MediaStop(QStringList *cmd);
  void CodeBook(QStringList *cmd);
  void Metadata(QStringList *cmd);
  void ParseCommand();
  void SendCommand(const QString &cmd);
  QSocket *conn_socket;
  QString conn_cmdline;
  QString conn_username;
  QString conn_password;
  QString conn_hostname;
  Q_UINT16 conn_tcp_port;
  QTimer *conn_reauthenticate_timer;
  QTimer *conn_reconnect_timer;
  bool conn_connected_once;
};


#endif  // CONTROLCONNECT_H
