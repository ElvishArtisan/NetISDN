// netconfig.cpp
//
// A container class for a NetISDN Database Configuration
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: netconfig.cpp,v 1.3 2009/01/26 13:05:20 pcvs Exp $
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

#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#endif

#include <qmessagebox.h>
#include <qsettings.h>

#include <profile.h>
#include <netconfig.h>


NetConfig *NetConfiguration() 
{
  static NetConfig *config = NULL;
  if (!config){
    config = new NetConfig();
    config->load();
  }
  return config;
}


NetConfig::NetConfig()
{
  clear();
}


NetConfig::NetConfig(QString filename)
{
  clear();
  conf_filename=filename;
}


QString NetConfig::filename() const
{
  return conf_filename;
}


void NetConfig::setFilename(const QString &filename)
{
  conf_filename=filename;
}


QString NetConfig::globalInterface() const
{
  return conf_global_interface;
}


QString NetConfig::mysqlHostname() const
{
  return conf_mysql_hostname;
}


QString NetConfig::mysqlUsername() const
{
  return conf_mysql_username;
}


QString NetConfig::mysqlDbname() const
{
  return conf_mysql_dbname;
}


QString NetConfig::mysqlPassword() const
{
  return conf_mysql_password;
}


QString NetConfig::mysqlDriver() const
{
  return conf_mysql_driver;
}


void NetConfig::load()
{
  Profile *profile=new Profile();
  profile->setSource(conf_filename);
  conf_global_interface=profile->stringValue("Global","Interface","0.0.0.0");
  conf_mysql_hostname=
    profile->stringValue("mySQL","Hostname",DEFAULT_MYSQL_HOSTNAME);
  conf_mysql_username=
    profile->stringValue("mySQL","Loginname",DEFAULT_MYSQL_USERNAME);
  conf_mysql_dbname=
    profile->stringValue("mySQL","Database",DEFAULT_MYSQL_DATABASE);
  conf_mysql_password=
      profile->stringValue("mySQL","Password",DEFAULT_MYSQL_PASSWORD);
  conf_mysql_driver=
    profile->stringValue("mySQL","Driver",DEFAULT_MYSQL_DRIVER);
  delete profile;
}


void NetConfig::clear()
{
#ifdef WIN32
  QSettings settings;
  settings.insertSearchPath(QSettings::Windows,"/SalemRadioLabs");
  conf_filename=QString().sprintf("%s\\%s",
				  (const char *)settings.
				  readEntry("/NetISDN/InstallDir"),
				  (const char *)RD_WIN_CONF_FILE);  
#else
  conf_filename=NET_CONF_FILE;
#endif
  conf_global_interface="0.0.0.0";
  conf_mysql_hostname="";
  conf_mysql_username="";
  conf_mysql_dbname="";
  conf_mysql_password="";
  conf_mysql_driver="";
}
