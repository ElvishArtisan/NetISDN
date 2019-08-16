// netisdn.cpp
//
// Desktop NetISDN Client
//
//   (C) Copyright 2002-2004 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: netisdn.cpp,v 1.46 2008/11/14 18:34:02 cvs Exp $
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

#ifdef WIN32
#include <io.h>
#include <Winsock2.h>
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sched.h>
#include <sys/mman.h>
#endif  // WIN32
#include <sys/types.h>

#include <qapplication.h>
#include <qmessagebox.h>
#include <qdir.h>
#include <qsettings.h>

#include <portaudio.h>

#include <cmd_switch.h>
#include <audiosettings_dialog.h>
#include <codec_stats.h>
#include <profile.h>

#include <netconf.h>

#include <netisdn.h>

//
// Icons
//
#include "../icons/netisdn-22x22.xpm"


MainWidget::MainWidget(QWidget *parent,const char *name)
  :QWidget(parent,name)
{
  PaError perr;
  net_username_entered=false;

  //
  // Fix the Window Size
  //
#ifndef RESIZABLE
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());
#endif  // RESIZABLE

  //
  // Load the command-line arguments
  //
  CmdSwitch *cmd=
    new CmdSwitch(qApp->argc(),qApp->argv(),"netisdn",NETISDN_USAGE);
  delete cmd;

  //
  // Generate Fonts
  //
  QFont light_font=QFont("Helvetica",8,QFont::Normal);
  QFont default_font=QFont("Helvetica",12,QFont::Normal);
  default_font.setPixelSize(12);
  setFont(default_font);
  QFont label_font=QFont("Helvetica",14,QFont::Bold);
  label_font.setPixelSize(14);
  QFont meter_font=QFont("Helvetica",16,QFont::Bold);
  meter_font.setPixelSize(16);

  //
  // Set Caption
  //
  setCaption(tr("NetISDN"));

  //
  // RML Sockets
  //
  net_rml_echo_socket=NULL;
  net_rml_noecho_socket=NULL;
  net_rml_return_socket=NULL;
  net_rml_timer=new QTimer(this);
  connect(net_rml_timer,SIGNAL(timeout()),this,SLOT(rmlReadData()));

  //
  // Initialize the audio driver
  //
  if((perr=Pa_Initialize())!=paNoError) {
    QMessageBox::warning(this,tr("NetISDN"),
			 tr("Unable to initialize audio driver!"));
    qApp->quit();
  }

  //
  // Create Icons
  //
  net_netisdn_map=new QPixmap(netisdn_xpm);
  setIcon(*net_netisdn_map);

  //
  // Destination URI
  //
  net_uri_box=new HistoryBox(this,"net_uri_box");
  net_uri_box->setGeometry(55,10,sizeHint().width()-75,20);
  net_uri_label=new QLabel(net_uri_box,tr("&URI:"),
				this,"net_uri_label");
  net_uri_label->setGeometry(10,10,40,20);
  net_uri_label->setFont(label_font);
  net_uri_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  connect(net_uri_box,SIGNAL(textChanged(const QString &)),
	  this,SLOT(hostnameChangedData(const QString &)));

  //
  // Connect Button
  //
  net_connect_button=new QPushButton(tr("Connect"),this,"net_connect_button");
  net_connect_button->setGeometry(55,35,90,30);
  net_connect_button->setDisabled(true);
  connect(net_connect_button,SIGNAL(clicked()),this,SLOT(connectClickedData()));

  //
  // Settings Button
  //
  net_settings_button=
    new QPushButton(tr("Settings"),this,"net_settings_button");
  net_settings_button->setGeometry(163,35,90,30);
  connect(net_settings_button,SIGNAL(clicked()),
	  this,SLOT(settingsClickedData()));

  //
  // Info Button
  //
  net_info_button=
    new QPushButton(tr("Info"),this,"net_info_button");
  net_info_button->setGeometry(270,35,90,30);
  connect(net_info_button,SIGNAL(clicked()),
	  this,SLOT(infoClickedData()));

  //
  // Codec
  //
  net_codec=new Codec(0,this);
  connect(net_codec,
	  SIGNAL(connectionStateChanged(unsigned,Codec::ConnectionState)),
	  this,SLOT(connectionStateChangedData(unsigned,
					       Codec::ConnectionState)));
  connect(net_codec,SIGNAL(locked(unsigned,bool)),
	  this,SLOT(lockedData(unsigned,bool)));
  connect(net_codec,SIGNAL(loopedBack(unsigned,bool)),
	  this,SLOT(loopedBackData(unsigned,bool)));
  connect(net_codec,SIGNAL(error(unsigned,int)),
	  this,SLOT(errorData(unsigned,int)));
  connect(net_codec,SIGNAL(gpioChanged(unsigned,unsigned,bool)),
	  this,SLOT(gpioChangedData(unsigned,unsigned,bool)));

  //
  // Statistics Dialog
  //
  net_stats_dialog=
    new StatsDialog(net_codec->codecStats(),this,"net_stats_dialog");

  //
  // Input Meters
  //
  QLabel *label=new QLabel(tr("Send"),this,"net_send_label");
  label->setGeometry(10,75,70,20);
  label->setFont(meter_font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  net_input_meters[0]=new PlayMeter(0,SegMeter::Right,this);
  net_input_meters[0]->setGeometry(85,75,sizeHint().width()-140,10);
  net_input_meters[0]->setMode(SegMeter::Peak);
  net_input_meters[0]->setLabel(tr("L"));
  net_input_meters[0]->setRange(-3000,0);
  net_input_meters[0]->setHighThreshold(-1200);
  net_input_meters[0]->setClipThreshold(-1200);
  connect(net_codec,SIGNAL(leftInputChanged(unsigned,int)),
	  net_input_meters[0],SLOT(setPeakBar(unsigned,int)));

  net_input_meters[1]=new PlayMeter(0,SegMeter::Right,this);
  net_input_meters[1]->setGeometry(85,85,sizeHint().width()-140,10);
  net_input_meters[1]->setMode(SegMeter::Peak);
  net_input_meters[1]->setLabel(tr("R"));
  net_input_meters[1]->setRange(-3000,0);
  net_input_meters[1]->setHighThreshold(-1200);
  net_input_meters[1]->setClipThreshold(-1200);
  connect(net_codec,SIGNAL(rightInputChanged(unsigned,int)),
	  net_input_meters[1],SLOT(setPeakBar(unsigned,int)));

  //
  // Transmit Light
  //
  net_transmit_label=new QLabel(tr("Tx"),this,"net_transmit_label");
  net_transmit_label->setGeometry(sizeHint().width()-46,73,26,24);
  net_transmit_label->setFont(light_font);
  net_transmit_label->setAlignment(AlignCenter);
  net_transmit_label->setFrameStyle(QFrame::Box|QFrame::Plain);
  net_transmit_label->setLineWidth(1);
  net_transmit_label->setMidLineWidth(1);
  net_light_palette=palette();
  net_light_palette.setColor(QPalette::Active,QColorGroup::Background,green);
  net_light_palette.setColor(QPalette::Inactive,QColorGroup::Background,green);

  //
  // Output Meters
  //
  label=new QLabel(tr("Receive"),this,"net_receive_label");
  label->setGeometry(10,100,70,20);
  label->setFont(meter_font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  net_output_meters[0]=new PlayMeter(0,SegMeter::Right,this);
  net_output_meters[0]->setGeometry(85,100,sizeHint().width()-140,10);
  net_output_meters[0]->setMode(SegMeter::Peak);
  net_output_meters[0]->setLabel(tr("L"));
  net_output_meters[0]->setRange(-3000,0);
  net_output_meters[0]->setHighThreshold(-1200);
  net_output_meters[0]->setClipThreshold(-1200);
  connect(net_codec,SIGNAL(leftOutputChanged(unsigned,int)),
	  net_output_meters[0],SLOT(setPeakBar(unsigned,int)));

  net_output_meters[1]=new PlayMeter(0,SegMeter::Right,this);
  net_output_meters[1]->setGeometry(85,110,sizeHint().width()-140,10);
  net_output_meters[1]->setMode(SegMeter::Peak);
  net_output_meters[1]->setLabel(tr("R"));
  net_output_meters[1]->setRange(-3000,0);
  net_output_meters[1]->setHighThreshold(-1200);
  net_output_meters[1]->setClipThreshold(-1200);
  connect(net_codec,SIGNAL(rightOutputChanged(unsigned,int)),
	  net_output_meters[1],SLOT(setPeakBar(unsigned,int)));

  //
  // Receive Light
  //
  net_receive_label=new QLabel(tr("Rx"),this,"net_receive_label");
  net_receive_label->setGeometry(sizeHint().width()-46,98,26,24);
  net_receive_label->setFont(light_font);
  net_receive_label->setAlignment(AlignCenter);
  net_receive_label->setFrameStyle(QFrame::Box|QFrame::Plain);
  net_receive_label->setLineWidth(1);
  net_receive_label->setMidLineWidth(1);

  //
  // Status Line
  //
  net_status_edit=new QLineEdit(this,"net_status_edit");
  net_status_edit->
    setGeometry(2,sizeHint().height()-22,sizeHint().width()-4,20);
  net_status_edit->setReadOnly(true);
  net_status_edit->setText("Ready.");

  //
  // Load Configuration
  //
  net_settings=new AudioSettings();
  ReadConfig();
  ReadHistory();
  InitRml(net_settings->rmlPort());

  //
  // Stats Timer
  //
  net_stats_timer=new QTimer(this,"net_stats_timer");
  connect(net_stats_timer,SIGNAL(timeout()),this,SLOT(statsData()));

  //
  // Control Connection
  //
  net_control=new ControlConnect(this);
  connect(net_control,SIGNAL(responseReceived(NetConfig::ResponseCode)),
	  this,SLOT(responseReceivedData(NetConfig::ResponseCode)));
  connect(net_control,
	  SIGNAL(mediaStartReceived(Q_UINT16,const QHostAddress &,Q_UINT16,
				    AudioSettings *)),
	  this,
	  SLOT(mediaStartReceivedData(Q_UINT16,const QHostAddress &,Q_UINT16,
				      AudioSettings *)));
  connect(net_control,SIGNAL(networkErrorReceived(QSocket::Error)),
	  this,SLOT(controlErrorData(QSocket::Error)));
  connect(net_control,SIGNAL(mediaStopReceived()),net_codec,SLOT(disconnect()));
  connect(net_codec,
    SIGNAL(codebookChanged(const QString &,const QString &,const QString &)),
    net_control,
    SLOT(sendCodebook(const QString &,const QString &,const QString &)));
  connect(net_control,
    SIGNAL(codebookReceived(const QString &,const QString &,const QString &)),
    net_codec,
    SLOT(setCodebook(const QString &,const QString &,const QString &)));
  connect(net_control,
     SIGNAL(metadataReceived(ControlConnect::MetadataField,const QString &)),
     net_stats_dialog,
     SLOT(updateMetadata(ControlConnect::MetadataField,const QString &)));

  //
  // Set Realtime priority
  //
  SetRealtime(net_settings->useRealtime());

  //
  // Verify username
  //
  if(net_settings->username().isEmpty()) {
    settingsClickedData();
    if(!net_username_entered) {
      exit(256);
    }
  }
  else {
    net_username_entered=true;
    controlRegistrationData();
  }
}


QSize MainWidget::sizeHint() const
{
  return QSize(400,150);
}


QSizePolicy MainWidget::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void MainWidget::hostnameChangedData(const QString &str)
{
  net_connect_button->setDisabled(str.isEmpty());
}


void MainWidget::responseReceivedData(NetConfig::ResponseCode resp)
{
  switch(resp) {
    case NetConfig::ResponseLoginOk:
      net_uri_box->setEnabled(true);
      net_uri_label->setEnabled(true);
      net_connect_button->setDisabled(net_uri_box->currentText().isEmpty());
      break;

    case NetConfig::ResponseUserInvalid:
      QMessageBox::warning(this,"NetISDN - Login Error",
			   tr("The user account is invalid or expired.")+
			   "\n"+tr("Check your Login Name and Password."));
      net_uri_box->setDisabled(true);
      net_uri_label->setDisabled(true);
      net_connect_button->setDisabled(true);
      break;

    case NetConfig::ResponseUserActive:
      QMessageBox::warning(this,"NetISDN - Login Error",
			   tr("The user account is already in use.")+
			   "\n"+tr("Check your Login Name and Password."));
      net_uri_box->setDisabled(true);
      net_uri_label->setDisabled(true);
      net_connect_button->setDisabled(true);
      break;

    case NetConfig::ResponseMalformed:
      QMessageBox::warning(this,"NetISDN - Server Error",
			   tr("The command was misformatted."));
      break;

    case NetConfig::ResponseUnknownCommand:
      QMessageBox::warning(this,"NetISDN - Server Error",
			   tr("The server didn't understand the command."));
      break;

    case NetConfig::ResponseAuthenticationNeeded:
      QMessageBox::warning(this,"NetISDN - Server Error",
			   tr("The server requires authentication."));
      break;

    case NetConfig::ResponseCallProceeding:
      net_uri_box->addToList();
      WriteHistory();
      break;

    case NetConfig::ResponseCallerUnknown:
      QMessageBox::warning(this,"NetISDN - Unknown User",
			   tr("The called user does not exist."));
      break;

    case NetConfig::ResponseCallerOffline:
      QMessageBox::warning(this,"NetISDN - Temporarily Unavailable",
			   tr("The called user is offline.")+
			   "\n"+tr("Try again later."));
      break;

    case NetConfig::ResponseCallerBusy:
      QMessageBox::warning(this,"NetISDN - Busy Here",
			   tr("The called user is busy.")+
			   "\n"+tr("Try again later."));
      break;

    case NetConfig::ResponseFormatUnsupported:
      net_control->endCall();
      net_codec->disconnect();
      QMessageBox::warning(this,"NetISDN - Unsupported Format",
	    tr("The request audio format is not supported by the callee.")+
			   "\n"+tr("Try a different format."));
      break;
  }
}


void MainWidget::mediaStartReceivedData(Q_UINT16 src_port,
					const QHostAddress &addr,
					Q_UINT16 dest_port,
					AudioSettings *settings)
{
  net_remote_username=settings->username();
  net_uri_box->addToList(settings->username());
  settings->setInputGain(net_settings->inputGain());
  net_codec->connectToHost(src_port,addr,dest_port,settings);
}


void MainWidget::controlErrorData(QSocket::Error err)
{
  QString estr;

  switch(err) {
    case QSocket::ErrConnectionRefused:
      estr=tr("Connection Refused");
      break;

    case QSocket::ErrHostNotFound:
      estr=tr("Host Not Found");
      break;

    case QSocket::ErrSocketRead:
      estr=tr("Socket Read Error");
      break;
  }
  QMessageBox::warning(this,"NetISDN",
		       tr("Unable to connect to the NetISDN service!")+
		       "\n"+
		       tr("Check your network connection and try again.")+
		       "\n\n"+
		       tr("Error Message: ")+estr);
}


void MainWidget::controlRegistrationData()
{
  net_control->connectToHost(net_settings->username(),net_settings->password(),
			     NETSIP_HOSTNAME,NETSIP_TCP_PORT);
}


void MainWidget::statsData()
{
  CodecStats *cs=net_codec->codecStats();
  if(net_codec->isLoopedBack()) {
    setCaption("NetISDN - LOOPED BACK");
  }
  else {
    if(!cs->name().isEmpty()) {
      setCaption(QString().sprintf("NetISDN - %s",(const char *)cs->name()));
    }
    else {
      if(!cs->receptionCname().isEmpty()) {
	setCaption(QString().sprintf("NetISDN - %s",
				     (const char *)cs->receptionCname()));
      }
      else {
	setCaption("NetISDN");
      }
    }
  }
}


void MainWidget::gpioChangedData(unsigned id,unsigned line,bool state)
{
  printf("gpioChangedData(%u,%u,%u)\n",id,line,state);
  if(net_settings->gpioCommand(line,state).isEmpty()||
     net_settings->gpioAddress(line,state).isNull()) {
    return;
  }
  net_rml_return_socket->
    writeBlock(net_settings->gpioCommand(line,state),
	       net_settings->gpioCommand(line,state).length(),
	       net_settings->gpioAddress(line,state),
	       net_settings->gpioPort(line,state));
}


void MainWidget::connectClickedData()
{
  switch(net_codec->connectionState()) {
    case Codec::StateConnecting:
    case Codec::StateConnected:
      net_control->endCall();
      break;

    case Codec::StateIdle:
      net_control->initiateCall(net_uri_box->currentText(),net_settings);
      break;
  }
}


void MainWidget::settingsClickedData()
{
  PaDeviceIndex in_pdev=net_codec->inputDevice();
  PaDeviceIndex out_pdev=net_codec->outputDevice();
  AudioSettingsDialog *d=new AudioSettingsDialog(net_settings,net_codec,
						 &in_pdev,&out_pdev,
						 true,this);
  if(d->exec()==0) {
    net_username_entered=true;
    net_codec->setInputDevice(in_pdev);
    net_codec->setOutputDevice(out_pdev);
    WriteConfig();
    SetRealtime(net_settings->useRealtime());
    controlRegistrationData();
  }
  delete d;
}


void MainWidget::connectionStateChangedData(unsigned id,
					    Codec::ConnectionState state)
{
  // printf("connctionStateChanged(%u,%u)\n",id,state);
  switch(state) {
    case Codec::StateConnecting:
    case Codec::StateConnected:
      net_connect_button->setText(tr("Disconnect"));
      net_connect_button->setEnabled(true);
      net_uri_box->setEnabled(true);
      net_uri_label->setEnabled(true);
      net_settings_button->setDisabled(true);
      net_stats_timer->start(NETISDN_STATS_SCAN_RATE);
      net_transmit_label->setPalette(net_light_palette);
      net_status_edit->
	setText(QString().sprintf("Connected to %s.",
				  (const char *)net_remote_username));
      break;

    case Codec::StateIdle:
      net_connect_button->setText(tr("Connect"));
      net_connect_button->setDisabled(net_uri_box->currentText().isEmpty());
      net_uri_box->setEnabled(true);
      net_uri_label->setEnabled(true);
      net_settings_button->setEnabled(true);
      net_transmit_label->setPalette(palette());
      net_stats_timer->stop();
      statsData();
      net_status_edit->setText("Ready.");
      for(int i=0;i<ControlConnect::MetadataLast;i++) {
	net_stats_dialog->updateMetadata((ControlConnect::MetadataField)i,"");
      }
      break;
  }
}


void MainWidget::infoClickedData()
{
  net_stats_dialog->show();
}


void MainWidget::lockedData(unsigned id,bool state)
{
  if(state) {
    net_receive_label->setPalette(net_light_palette);
  }
  else {
    net_receive_label->setPalette(palette());
  }
}


void MainWidget::loopedBackData(unsigned id,bool state)
{
  statsData();
}


void MainWidget::errorData(unsigned id,int err)
{
  printf("errorData(%u,%d)\n",id,err);
}


void MainWidget::rmlReadData()
{
  int n;
  unsigned char buf[1500];
  QString cmd;

  while((n=net_rml_echo_socket->readBlock((char *)buf,1500))>0) {
    for(int i=0;i<n;i++) {
      if(buf[i]=='!') {
	ProcessRml(QStringList::split(" ",cmd),
		   net_rml_echo_socket->peerAddress());
	cmd="";
      }
      else {
	cmd+=buf[i];
      }
    }
  }
  while((n=net_rml_noecho_socket->readBlock((char *)buf,1500))>0) {
    for(int i=0;i<n;i++) {
      if(buf[i]=='!') {
	ProcessRml(QStringList::split(" ",cmd),QHostAddress());
	cmd="";
      }
      else {
	cmd+=buf[i];
      }
    }
  }
}


void MainWidget::ProcessRml(QStringList cmd,const QHostAddress &addr)
{
  if(cmd.size()==0) {
    return;
  }
  if(cmd[0]=="GO") {  // Upconvert from 'old' format
    if(cmd.size()==5) {
      cmd.push_back(cmd[cmd.size()-1]);
      for(unsigned i=3;i>=2;i--) {
	cmd[i+1]=cmd[i];
      }
      cmd[2]="O";
    }
    if(cmd.size()!=6) {
      if(!addr.isNull()) {
	QString ret=cmd.join(" ")+"-!";
	net_rml_return_socket->
	  writeBlock(ret,ret.length(),addr,NETISDN_RETURN_RML_PORT);
      }
      return;
    }
    unsigned matrix=cmd[1].toUInt();
    unsigned line=cmd[3].toUInt();
    unsigned state=cmd[4].toUInt();
    unsigned interval=cmd[5].toUInt();
    if((matrix!=0)||(line==0)||(line>32)||((state!=0)&&(state!=1))) {
      if(!addr.isNull()) {
	QString ret=cmd.join(" ")+"-!";
	net_rml_return_socket->
	  writeBlock(ret,ret.length(),addr,NETISDN_RETURN_RML_PORT);
      }
      return;
    }
    net_codec->sendGpio(line-1,state,interval);
    if(!addr.isNull()) {
      QString ret=cmd.join(" ")+"+!";
      net_rml_return_socket->
	writeBlock(ret,ret.length(),addr,NETISDN_RETURN_RML_PORT);
    }
    return;
  }
  if(!addr.isNull()) {
    QString ret=cmd.join(" ")+"-!";
    net_rml_return_socket->
      writeBlock(ret,ret.length(),addr,NETISDN_RETURN_RML_PORT);
  }
}


void MainWidget::InitRml(int port)
{
  net_rml_timer->stop();
  if(net_rml_echo_socket!=NULL) {
    delete net_rml_echo_socket;
    net_rml_echo_socket=NULL;
  }
  if(net_rml_noecho_socket!=NULL) {
    delete net_rml_noecho_socket;
    net_rml_noecho_socket=NULL;
  }
  if(net_rml_return_socket!=NULL) {
    delete net_rml_return_socket;
    net_rml_return_socket=NULL;
  }
  if(port>=0) {
    net_rml_echo_socket=new QSocketDevice(QSocketDevice::Datagram);
    net_rml_echo_socket->setBlocking(false);
    net_rml_echo_socket->bind(QHostAddress(),port);
    net_rml_noecho_socket=new QSocketDevice(QSocketDevice::Datagram);
    net_rml_noecho_socket->setBlocking(false);
    net_rml_noecho_socket->bind(QHostAddress(),port+1);
    net_rml_return_socket=new QSocketDevice(QSocketDevice::Datagram);
    net_rml_return_socket->setBlocking(false);
    net_rml_return_socket->bind(QHostAddress(),port+2);
    net_rml_timer->start(NETISDN_RML_SCAN_INTERVAL);
  }
}


void MainWidget::SetRealtime(bool state)
{
#ifdef WIN32
  if(state) {
    if(!SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS)) {
      QMessageBox::warning(this,tr("NetISDN - Permissions Error"),
			   tr("Unable to get realtime permissions!"));
    }
  }
  else {
    SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
  }
#else
  struct sched_param sp;
  int ret=0;
  memset(&sp,0,sizeof(sp));
  if(state) {
    sp.sched_priority=50;
    ret|=sched_setscheduler(0,SCHED_FIFO,&sp);
    ret|=mlockall(MCL_CURRENT|MCL_FUTURE);
    if(ret!=0) {
      QMessageBox::warning(this,tr("NetISDN - Permissions Error"),
			   tr("Unable to get realtime permissions!"));
    }
  }
  else {
    sched_setscheduler(0,SCHED_OTHER,&sp);
    munlockall();
  }
#endif  // WIN32
}


void MainWidget::ReadConfig()
{
  QString enc;
  QHostAddress addr;

  net_settings->setFormat(AudioSettings::Transmit,AudioSettings::Layer3);
#ifdef WIN32
  QSettings s;
  s.setPath("Paravel Systems","NetISDN",QSettings::User);
  s.insertSearchPath(QSettings::Windows,"/Paravel Systems");
  enc=NetBase64ToString(s.readEntry("/NetISDN/Settings/Login"));
  net_settings->setFormat(AudioSettings::Transmit,(AudioSettings::Format)s.
			  readNumEntry("/NetISDN/Settings/Format",
				       NETISDN_DEFAULT_FORMAT));
  net_settings->setChannels(AudioSettings::Transmit,
			    s.readNumEntry("/NetISDN/Settings/Channels",
					   NETISDN_DEFAULT_CHANNELS));
  net_settings->setSampleRate(s.readNumEntry("/NetISDN/Settings/SampleRate",
					     NETISDN_DEFAULT_SAMPRATE));
  net_settings->setBitRate(AudioSettings::Transmit,
			   s.readNumEntry("/NetISDN/Settings/BitRate",
					  NETISDN_DEFAULT_BITRATE));
  net_settings->setFormat(AudioSettings::Receive,(AudioSettings::Format)s.
			  readNumEntry("/NetISDN/Settings/ReceiveFormat",
				       NETISDN_DEFAULT_FORMAT));
  net_settings->setChannels(AudioSettings::Receive,
			    s.readNumEntry("/NetISDN/Settings/ReceiveChannels",
					   NETISDN_DEFAULT_CHANNELS));
  net_settings->setBitRate(AudioSettings::Receive,
			   s.readNumEntry("/NetISDN/Settings/ReceiveBitRate",
					  NETISDN_DEFAULT_BITRATE));
  net_settings->setRmlPort(s.readNumEntry("/NetISDN/Settings/RmlPort",
					  NETISDN_DEFAULT_RML_PORT));
  net_codec->setRtpSdesField(Codec::SdesName,
				    s.readEntry("/NetISDN/Settings/RtpName"));
  net_codec->setRtpSdesField(Codec::SdesPhone,
				    s.readEntry("/NetISDN/Settings/RtpPhone"));
  net_codec->setRtpSdesField(Codec::SdesEmail,
				    s.readEntry("/NetISDN/Settings/RtpEmail"));
  net_settings->setStreamRatio(AudioSettings::Transmit,
			       s.readNumEntry("/NetISDN/Settings/StreamRatio"));
  net_settings->setStreamRatio(AudioSettings::Receive,
		    s.readNumEntry("/NetISDN/Settings/ReceiveStreamRatio"));
  net_settings->setInputGain(s.readNumEntry("/NetISDN/Settings/InputGain",
			       NETISDN_DEFAULT_INPUT_GAIN));
  net_settings->
    setUseRealtime(s.readNumEntry("/NetISDN/Settings/UseRealtime",0));
  net_codec->
    setReceiveBuffers(s.readNumEntry("/NetISDN/Settings/ReceiveBuffers",
				     CODEC_DEFAULT_BUFFER_SETPOINT));
  net_codec->
    setInputDevice(s.readNumEntry("/NetISDN/Settings/InputDevice"));
  net_codec->
    setOutputDevice(s.readNumEntry("/NetISDN/Settings/OutputDevice"));
  net_settings->
    setEnableProcessor(AudioSettings::Transmit,
		       s.readNumEntry("/NetISDN/Settings/EnableProcessor"));
  net_settings->
    setEnableProcessor(AudioSettings::Receive,
		  s.readNumEntry("/NetISDN/Settings/ReceiveEnableProcessor"));
  for(int i=0;i<32;i++) {
    net_settings->setGpioCommand(i,0,s.readEntry(QString().
		       sprintf("/NetISDN/Settings/GpioOffCommand%d",i+1)));
    net_settings->setGpioCommand(i,1,s.readEntry(QString().
		       sprintf("/NetISDN/Settings/GpioOnCommand%d",i+1)));
    addr.setAddress(s.readEntry(QString().
		       sprintf("/NetISDN/Settings/GpioOffAddress%d",i+1)));
    net_settings->setGpioAddress(i,0,addr);
    addr.setAddress(s.readEntry(QString().
		       sprintf("/NetISDN/Settings/GpioOnAddress%d",i+1)));
    net_settings->setGpioAddress(i,1,addr);
    net_settings->setGpioPort(i,0,s.readNumEntry(QString().
		       sprintf("/NetISDN/Settings/GpioOffPort%d",i+1)));
    net_settings->setGpioPort(i,1,s.readNumEntry(QString().
		       sprintf("/NetISDN/Settings/GpioOnPort%d",i+1)));
  }

#else
  QString filename=GetConfigFile();
  if(filename.isEmpty()) {
    QMessageBox::warning(this,tr("NetISDN"),
			 tr("Unable to create configuration directory!"));
    return;
  }
  Profile *p=new Profile();
  p->setSource(filename);
  enc=NetBase64ToString(p->stringValue("NetISDN","Login",""));
  net_settings->setFormat(AudioSettings::Transmit,
			  (AudioSettings::Format)p->intValue("NetISDN","Format",
						 NETISDN_DEFAULT_FORMAT));
  net_settings->
    setChannels(AudioSettings::Transmit,
		p->intValue("NetISDN","Channels",NETISDN_DEFAULT_CHANNELS));
  net_settings->
    setSampleRate(p->intValue("NetISDN","SampleRate",NETISDN_DEFAULT_SAMPRATE));
  net_settings->
    setBitRate(AudioSettings::Transmit,
	       p->intValue("NetISDN","BitRate",NETISDN_DEFAULT_BITRATE));
  net_settings->
    setFormat(AudioSettings::Receive,
	      (AudioSettings::Format)p->intValue("NetISDN","ReceiveFormat",
						 NETISDN_DEFAULT_FORMAT));
  net_settings->
    setChannels(AudioSettings::Receive,p->intValue("NetISDN","ReceiveChannels",
						   NETISDN_DEFAULT_CHANNELS));
  net_settings->
    setBitRate(AudioSettings::Receive,
	       p->intValue("NetISDN","ReceiveBitRate",NETISDN_DEFAULT_BITRATE));
  net_settings->
    setRmlPort(p->intValue("NetISDN","RmlPort",NETISDN_DEFAULT_RML_PORT));
  net_codec->setRtpSdesField(Codec::SdesName,
				    p->stringValue("NetISDN","RtpName",""));
  net_codec->setRtpSdesField(Codec::SdesPhone,
				    p->stringValue("NetISDN","RtpPhone",""));
  net_codec->setRtpSdesField(Codec::SdesEmail,
				    p->stringValue("NetISDN","RtpEmail",""));
  net_codec->
      setInputDevice(p->intValue("NetISDN","InputDevice",
				 net_codec->inputDevice()));
  net_settings->setStreamRatio(AudioSettings::Transmit,
			    p->intValue("NetISDN","StreamRatio",
					net_settings->
					streamRatio(AudioSettings::Transmit)));
  net_settings->setStreamRatio(AudioSettings::Receive,
			    p->intValue("NetISDN","ReceiveStreamRatio",
					net_settings->
					streamRatio(AudioSettings::Receive)));
  net_settings->setInputGain(p->intValue("NetISDN","InputGain",
					 net_settings->inputGain(),
					 NETISDN_DEFAULT_INPUT_GAIN));
  net_settings->
    setUseRealtime(p->intValue("NetISDN","UseRealtime",net_settings->
			       useRealtime()));
  net_codec->setReceiveBuffers(p->intValue("NetISDN","ReceiveBuffers",
					   net_codec->receiveBuffers()));
  net_codec->
      setOutputDevice(p->intValue("NetISDN","OutputDevice",
				 net_codec->outputDevice()));
  net_settings->
    setEnableProcessor(AudioSettings::Transmit,
		       p->intValue("NetISDN","EnableProcessor",net_settings->
				   enableProcessor(AudioSettings::Transmit)));
  net_settings->
    setEnableProcessor(AudioSettings::Receive,
		       p->intValue("NetISDN","ReceiveEnableProcessor",
				   net_settings->
				   enableProcessor(AudioSettings::Receive)));
  for(int i=0;i<32;i++) {
    net_settings->setGpioCommand(i,0,p->
	    stringValue("NetISDN",QString().sprintf("GpioOffCommand%d",i+1),
			net_settings->gpioCommand(i,0)));
    net_settings->setGpioCommand(i,1,p->
	    stringValue("NetISDN",QString().sprintf("GpioOnCommand%d",i+1),
			net_settings->gpioCommand(i,1)));

    addr.setAddress(p->stringValue("NetISDN",
				   QString().sprintf("GpioOffAddress%d",i+1),
				   net_settings->gpioAddress(i,0).toString()));
    net_settings->setGpioAddress(i,0,addr);
    addr.setAddress(p->stringValue("NetISDN",
				   QString().sprintf("GpioOnAddress%d",i+1),
				   net_settings->gpioAddress(i,1).toString()));
    net_settings->setGpioAddress(i,1,addr);

    net_settings->setGpioPort(i,0,p->
	    intValue("NetISDN",QString().sprintf("GpioOffPort%d",i+1),
		     net_settings->gpioPort(i,0)));
    net_settings->setGpioPort(i,1,p->
	    intValue("NetISDN",QString().sprintf("GpioOnPort%d",i+1),
		     net_settings->gpioPort(i,1)));
  }

  delete p;
#endif  // WIN32
  int n;
  if((n=enc.find(":"))>=0) {
    net_settings->setUsername(enc.left(n));
    net_settings->setPassword(enc.right(enc.length()-n-1));
  }
  net_codec->setTransmitCname(net_settings->username()+"@netisdn.com");
}


void MainWidget::WriteConfig()
{
  QString enc=NetStringToBase64(net_settings->username()+":"+
				net_settings->password());
#ifdef WIN32
  QSettings s;
  s.setPath("Paravel Systems","NetISDN",QSettings::User);
  s.insertSearchPath(QSettings::Windows,"/Paravel Systems");
  s.writeEntry("/NetISDN/Settings/Login",enc);
  s.writeEntry("/NetISDN/Settings/Format",
	       (int)net_settings->format(AudioSettings::Transmit,false));
  s.writeEntry("/NetISDN/Settings/Channels",
	       (int)net_settings->channels(AudioSettings::Transmit,false));
  s.writeEntry("/NetISDN/Settings/SampleRate",
	       (int)net_settings->sampleRate());
  s.writeEntry("/NetISDN/Settings/BitRate",
	       (int)net_settings->bitRate(AudioSettings::Transmit,false));
  s.writeEntry("/NetISDN/Settings/ReceiveFormat",
	       (int)net_settings->format(AudioSettings::Receive,false));
  s.writeEntry("/NetISDN/Settings/ReceiveChannels",
	       (int)net_settings->channels(AudioSettings::Receive,false));
  s.writeEntry("/NetISDN/Settings/BitRate",
	       (int)net_settings->bitRate(AudioSettings::Receive,false));
  s.writeEntry("/NetISDN/Settings/RmlPort",(int)net_settings->rmlPort());
  s.writeEntry("/NetISDN/Settings/RtpName",net_codec->
	       rtpSdesField(Codec::SdesName));
  s.writeEntry("/NetISDN/Settings/RtpPhone",net_codec->
	       rtpSdesField(Codec::SdesPhone));
  s.writeEntry("/NetISDN/Settings/RtpEmail",net_codec->
	       rtpSdesField(Codec::SdesEmail));
  s.writeEntry("/NetISDN/Settings/StreamRatio",
	       (int)net_settings->streamRatio(AudioSettings::Transmit,false));
  s.writeEntry("/NetISDN/Settings/InputGain",net_settings->inputGain());
  s.writeEntry("/NetISDN/Settings/ReceiveStreamRatio",
	       (int)net_settings->streamRatio(AudioSettings::Receive,false));
  s.writeEntry("/NetISDN/Settings/UseRealtime",net_settings->useRealtime());
  s.writeEntry("/NetISDN/Settings/ReceiveBuffers",
	       (int)net_codec->receiveBuffers());
  s.writeEntry("/NetISDN/Settings/InputDevice",net_codec->inputDevice());
  s.writeEntry("/NetISDN/Settings/OutputDevice",
	       net_codec->outputDevice());
  s.writeEntry("/NetISDN/Settings/EnableProcessor",
	       net_settings->enableProcessor(AudioSettings::Transmit,false));
  s.writeEntry("/NetISDN/Settings/ReceiveEnableProcessor",
	       net_settings->enableProcessor(AudioSettings::Receive,false));
  for(int i=0;i<32;i++) {
    s.writeEntry(QString().sprintf("/NetISDN/Settings/GpioOffCommand%d",i+1),
		 net_settings->gpioCommand(i,0));
    s.writeEntry(QString().sprintf("/NetISDN/Settings/GpioOnCommand%d",i+1),
		 net_settings->gpioCommand(i,1));
    s.writeEntry(QString().sprintf("/NetISDN/Settings/GpioOffAddress%d",i+1),
		 net_settings->gpioAddress(i,0).toString());
    s.writeEntry(QString().sprintf("/NetISDN/Settings/GpioOnAddress%d",i+1),
		 net_settings->gpioCommand(i,1));
    s.writeEntry(QString().sprintf("/NetISDN/Settings/GpioOffPort%d",i+1),
		 net_settings->gpioPort(i,0));
    s.writeEntry(QString().sprintf("/NetISDN/Settings/GpioOnPort%d",i+1),
		 net_settings->gpioPort(i,1));
  }

#else
  QString filename=GetConfigFile();
  if(filename.isEmpty()) {
    QMessageBox::warning(this,tr("NetISDN"),
			 tr("Unable to create configuration directory!"));
    return;
  }
  FILE *f=fopen(filename,"w");
  if(f==NULL) {
    QMessageBox::warning(this,tr("NetISDN"),
			 tr("Unable to write configuration file!"));
    return;
  }
  fprintf(f,"[NetISDN]\n");
  fprintf(f,"Login=%s\n",(const char *)enc);
  fprintf(f,"Format=%d\n",net_settings->format(AudioSettings::Transmit,false));
  fprintf(f,"Channels=%d\n",net_settings->
	  channels(AudioSettings::Transmit,false));
  fprintf(f,"SampleRate=%d\n",net_settings->sampleRate());
  fprintf(f,"BitRate=%d\n",net_settings->
	  bitRate(AudioSettings::Transmit,false));
  fprintf(f,"ReceiveFormat=%d\n",net_settings->
	  format(AudioSettings::Receive,false));
  fprintf(f,"ReceiveChannels=%d\n",
	  net_settings->channels(AudioSettings::Receive,false));
  fprintf(f,"ReceiveBitRate=%d\n",
	  net_settings->bitRate(AudioSettings::Receive,false));
  fprintf(f,"RmlPort=%d\n",net_settings->rmlPort());
  fprintf(f,"RtpName=%s\n",(const char *)net_codec->
	  rtpSdesField(Codec::SdesName));
  fprintf(f,"RtpPhone=%s\n",(const char *)net_codec->
	  rtpSdesField(Codec::SdesPhone));
  fprintf(f,"RtpEmail=%s\n",(const char *)net_codec->
	  rtpSdesField(Codec::SdesEmail));
  fprintf(f,"StreamRatio=%d\n",
	  net_settings->streamRatio(AudioSettings::Transmit,false));
  fprintf(f,"InputGain=%d\n",net_settings->inputGain());
  fprintf(f,"ReceiveStreamRatio=%d\n",
	  net_settings->streamRatio(AudioSettings::Receive,false));
  fprintf(f,"UseRealtime=%d\n",net_settings->useRealtime());
  fprintf(f,"ReceiveBuffers=%u\n",net_codec->receiveBuffers());
  fprintf(f,"InputDevice=%d\n",net_codec->inputDevice());
  fprintf(f,"OutputDevice=%d\n",net_codec->outputDevice());
  fprintf(f,"EnableProcessor=%d\n",
	  net_settings->enableProcessor(AudioSettings::Transmit,false));
  fprintf(f,"ReceiveEnableProcessor=%d\n",
	  net_settings->enableProcessor(AudioSettings::Receive,false));
  for(int i=0;i<32;i++) {
    fprintf(f,"GpioOffCommand%d=%s\n",i+1,
	    (const char *)net_settings->gpioCommand(i,0));
    fprintf(f,"GpioOnCommand%d=%s\n",i+1,
	    (const char *)net_settings->gpioCommand(i,1));

    fprintf(f,"GpioOffAddress%d=%s\n",i+1,
	    (const char *)net_settings->gpioAddress(i,0).toString());
    fprintf(f,"GpioOnAddress%d=%s\n",i+1,
	    (const char *)net_settings->gpioAddress(i,1).toString());
    fprintf(f,"GpioOffPort%d=%d\n",i+1,net_settings->gpioPort(i,0));
    fprintf(f,"GpioOnPort%d=%d\n",i+1,net_settings->gpioPort(i,1));
  }
  fclose(f);
#endif  // WIN32
  net_control->sendMetadata(ControlConnect::MetadataName,
			    net_codec->rtpSdesField(Codec::SdesName));
  net_control->sendMetadata(ControlConnect::MetadataEmail,
			    net_codec->rtpSdesField(Codec::SdesEmail));
  net_control->sendMetadata(ControlConnect::MetadataPhone,
			    net_codec->rtpSdesField(Codec::SdesPhone));
  net_codec->setTransmitCname(net_settings->username()+"@netisdn.com");
}


QString MainWidget::GetConfigFile()
{
  QString path;

  if(getenv("HOME")==NULL) {
    path="/";
  }
  else {
    path=getenv("HOME");
  }
  path+="/.netisdn";
  QDir dir(path);
  if(!dir.exists()) {
    if(!dir.mkdir(path)) {
      return QString();
    }
  }
  return path+"/settings.conf";
}


void MainWidget::ReadHistory()
{
  QString str;
#ifdef WIN32
  QSettings s;
  s.setPath("Paravel Systems","NetISDN",QSettings::User);
  s.insertSearchPath(QSettings::Windows,"/Paravel Systems");
  for(int i=0;i<net_uri_box->maxItems();i++) {
    str=s.readEntry(QString().sprintf("/NetISDN/History/Uri%d",i+1));
    if(!str.isEmpty()) {
      net_uri_box->insertItem(str);
    }
  }
#else
  QString filename=GetHistoryFile();
  if(filename.isEmpty()) {
    QMessageBox::warning(this,tr("NetISDN"),
			 tr("Unable to create configuration directory!"));
    return;
  }
  Profile *p=new Profile();
  p->setSource(filename);
  for(int i=0;i<net_uri_box->maxItems();i++) {
    str=p->stringValue("History",QString().sprintf("Uri%d",i+1));
    if(!str.isEmpty()) {
      net_uri_box->insertItem(str);
    }
  }
  delete p;
#endif  // WIN32
}


void MainWidget::WriteHistory()
{
#ifdef WIN32
  QSettings s;
  s.setPath("Paravel Systems","NetISDN",QSettings::User);
  s.insertSearchPath(QSettings::Windows,"/Paravel Systems");
  for(int i=0;i<net_uri_box->count();i++) {
    s.writeEntry(QString().sprintf("/NetISDN/History/Uri%d",i+1),
		 net_uri_box->text(i));
  }
  for(int i=net_uri_box->count();i<net_uri_box->maxItems();i++) {
    s.removeEntry(QString().sprintf("/NetISDN/History/Uri%d",i+1));
  }
#else
  QString filename=GetHistoryFile();
  if(filename.isEmpty()) {
    QMessageBox::warning(this,tr("NetISDN"),
			 tr("Unable to create configuration directory!"));
    return;
  }
  FILE *f=fopen(filename,"w");
  if(f==NULL) {
    QMessageBox::warning(this,tr("NetISDN"),
			 tr("Unable to write history file!"));
    return;
  }
  fprintf(f,"[History]\n");
  for(int i=0;i<net_uri_box->count();i++) {
    fprintf(f,"Uri%d=%s\n",i+1,(const char *)net_uri_box->text(i));
  }
  fclose(f);
#endif  // WIN32
}


QString MainWidget::GetHistoryFile()
{
  QString path;

  if(getenv("HOME")==NULL) {
    path="/";
  }
  else {
    path=getenv("HOME");
  }
  path+="/.netisdn";
  QDir dir(path);
  if(!dir.exists()) {
    if(!dir.mkdir(path)) {
      return QString();
    }
  }
  return path+"/history.conf";
}


int main(int argc,char *argv[])
{
  QApplication a(argc,argv);
  
  MainWidget *w=new MainWidget(NULL,"main");
  a.setMainWidget(w);
  w->setGeometry(QRect(QPoint(w->geometry().x(),w->geometry().y()),
		 w->sizeHint()));
  w->show();
  return a.exec();
}
