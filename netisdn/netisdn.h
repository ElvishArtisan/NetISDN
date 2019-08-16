// netisdn.h
//
// Desktop NetISDN Client
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: netisdn.h,v 1.27 2008/11/14 18:34:02 cvs Exp $
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

#ifndef NETISDN_H
#define NETISDN_H

#include <qwidget.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qpalette.h>
#include <qsocketdevice.h>
#include <qpixmap.h>

#include <playmeter.h>
#include <codec.h>
#include <audiosettings.h>
#include <historybox.h>
#include <controlconnect.h>

#include <stats_dialog.h>

//
// Default Settings
//
#define NETISDN_DEFAULT_FORMAT AudioSettings::Speex
#define NETISDN_DEFAULT_CHANNELS 1
#define NETISDN_DEFAULT_SAMPRATE 32000
#define NETISDN_DEFAULT_BITRATE 48000
#define NETISDN_DEFAULT_RML_PORT 5858
#define NETISDN_DEFAULT_INPUT_GAIN 0

#define NETISDN_RETURN_RML_PORT 5860
#define NETISDN_RML_SCAN_INTERVAL 100
#define NETISDN_USAGE "\n"
#define NETISDN_STATS_SCAN_RATE 500

class MainWidget : public QWidget
{
  Q_OBJECT
 public:
  MainWidget(QWidget *parent=0,const char *name=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void hostnameChangedData(const QString &str);
  void responseReceivedData(NetConfig::ResponseCode resp);
  void mediaStartReceivedData(Q_UINT16 src_port,const QHostAddress &addr,
			      Q_UINT16 dest_port,AudioSettings *settings);
  void controlErrorData(QSocket::Error err);
  void controlRegistrationData();
  void statsData();
  void gpioChangedData(unsigned id,unsigned line,bool state);
  void connectClickedData();
  void settingsClickedData();
  void infoClickedData();
  void connectionStateChangedData(unsigned id,
				  Codec::ConnectionState state);
  void lockedData(unsigned id,bool state);
  void loopedBackData(unsigned id,bool state);
  void errorData(unsigned id,int err);
  void rmlReadData();

 private:
  void ProcessRml(QStringList cmd,const QHostAddress &addr);
  void InitRml(int port);
  void SetRealtime(bool state);
  void ReadConfig();
  void WriteConfig();
  QString GetConfigFile();
  void ReadHistory();
  void WriteHistory();
  QString GetHistoryFile();
  Codec *net_codec;
  StatsDialog *net_stats_dialog;
  HistoryBox *net_uri_box;
  QLabel *net_uri_label;
  QPushButton *net_connect_button;
  QPushButton *net_settings_button;
  QPushButton *net_info_button;
  QLabel *net_transmit_label;
  QLabel *net_receive_label;
  QPalette net_light_palette;
  QLineEdit *net_status_edit;
  PlayMeter *net_input_meters[2];
  PlayMeter *net_output_meters[2];
  QTimer *net_stats_timer;
  AudioSettings *net_settings;
  ControlConnect *net_control;
  QPixmap *net_netisdn_map;
  QString net_remote_username;
  bool net_username_entered;
  QSocketDevice *net_rml_echo_socket;
  QSocketDevice *net_rml_noecho_socket;
  QSocketDevice *net_rml_return_socket;
  QTimer *net_rml_timer;
};


#endif  // NETISDN_H
