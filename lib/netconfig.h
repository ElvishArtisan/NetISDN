// netconfig.h
//
// A container class for a NetISDN Database Configuration
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: netconfig.h,v 1.17 2009/01/29 18:16:28 pcvs Exp $
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

#ifndef NETCONFIG_H
#define NETCONFIG_H

#define NET_CONF_FILE "/etc/netisdn.conf"
#define DEFAULT_LOGIN_NAME "admin"
#define DEFAULT_PASSWORD ""
#define DEFAULT_FULLNAME "Administrator Account"
#define DEFAULT_MYSQL_HOSTNAME "localhost"
#define DEFAULT_MYSQL_USERNAME "netuser"
#define DEFAULT_MYSQL_PASSWORD "letmein"
#define DEFAULT_MYSQL_DATABASE "NetISDN"
#define DEFAULT_MYSQL_DRIVER "QMYSQL3"
#define NETSIP_HOSTNAME "directory.netisdn.com"
#define NETSIP_TCP_PORT 8448
#define NETSIP_RTP_PORT 5004
#define NETSIP_REGISTRATION_INTERVAL 300
#define NETSIP_RECONNECT_INTERVAL 10
#define NETSIP_GEOIP_DATADIR "/usr/share/GeoIP"
#define NETSIP_STATUS_MCAST_ADDR "224.0.1.0"
#define NETSIP_STATUS_MCAST_PORT 6765

#include <qstring.h>

class NetConfig
{
 public:
  enum ResponseCode {ResponseLoginOk=0,ResponseUserInvalid=1,
		     ResponseUserActive=2,ResponseMalformed=3,
		     ResponseUnknownCommand=4,ResponseAuthenticationNeeded=5,
		     ResponseCallProceeding=10,
		     ResponseCallerUnknown=11,ResponseCallerOffline=12,
		     ResponseCallerBusy=13,ResponseFormatUnsupported=14};
  NetConfig();
  NetConfig(QString filename);
  QString filename() const;
  void setFilename(const QString &name);
  QString globalInterface() const;
  QString mysqlHostname() const;
  QString mysqlUsername() const;
  QString mysqlDbname() const;
  QString mysqlPassword() const;
  QString mysqlDriver() const;
  void load();
  void clear();

 private:
  QString conf_filename;
  QString conf_global_interface;
  QString conf_mysql_hostname;
  QString conf_mysql_username;
  QString conf_mysql_dbname;
  QString conf_mysql_password;
  QString conf_mysql_driver;
};

NetConfig *NetConfiguration();


#endif  // NETCONFIG_H
