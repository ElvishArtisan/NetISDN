// stats_dialog.cpp
//
// A statistics display dialog for NetISDN.
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

#include <qtimer.h>
#include <qpainter.h>

#include <stats_dialog.h>


StatsDialog::StatsDialog(CodecStats *stats,QWidget *parent,const char *name)
  : QDialog(parent,name,false)
{
  stats_stats=stats;

  setCaption(tr("NetISDN - v")+VERSION);

  //
  // Fix the Window Size
  //
  setMinimumWidth(sizeHint().width());
  setMaximumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());

  //
  // Generate Fonts
  //
  QFont default_font("helvetica",12,QFont::Normal);
  default_font.setPixelSize(12);
  QFont light_font("helvetica",10,QFont::Normal);
  light_font.setPixelSize(10);
  setFont(default_font);
  QFont label_font("helvetica",12,QFont::Bold);
  label_font.setPixelSize(12);
  QFont header_font("helvetica",14,QFont::Bold);
  header_font.setPixelSize(14);

  //
  // Identity Section
  //
  QLabel *label=new QLabel(tr("Remote User"),this);
  label->setGeometry(10,10,sizeHint().width()-20,20);
  label->setFont(header_font);
  label->setAlignment(AlignVCenter|AlignLeft);

  //
  // Name Field
  //
  stats_name_edit=new QLineEdit(this,"stats_name_edit");
  stats_name_edit->setGeometry(130,30,sizeHint().width()-140,20);
  stats_name_edit->setReadOnly(true);
  label=new QLabel(stats_name_edit,tr("Name:"),this);
  label->setGeometry(10,30,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // E-mail Field
  //
  stats_email_edit=new QLineEdit(this,"stats_email_edit");
  stats_email_edit->setGeometry(130,52,sizeHint().width()-140,20);
  stats_email_edit->setReadOnly(true);
  label=new QLabel(stats_email_edit,tr("E-mail Address:"),this);
  label->setGeometry(10,52,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Phone Field
  //
  stats_phone_edit=new QLineEdit(this,"stats_phone_edit");
  stats_phone_edit->setGeometry(130,74,sizeHint().width()-140,20);
  stats_phone_edit->setReadOnly(true);
  label=new QLabel(stats_phone_edit,tr("Phone Number:"),this);
  label->setGeometry(10,74,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Location Field
  //
  stats_location_edit=new QLineEdit(this,"stats_location_edit");
  stats_location_edit->setGeometry(130,96,sizeHint().width()-140,20);
  stats_location_edit->setReadOnly(true);
  label=new QLabel(stats_location_edit,tr("Location:"),this);
  label->setGeometry(10,96,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Local Tx Connection Section
  //
  label=new QLabel(tr("Outbound Connection"),this);
  label->setGeometry(10,130,sizeHint().width()/2-10,20);
  label->setFont(header_font);
  label->setAlignment(AlignVCenter|AlignLeft);

  //
  // Tx Cname Field
  //
  stats_txcname_edit=new QLineEdit(this,"stats_txcname_edit");
  stats_txcname_edit->setGeometry(70,150,sizeHint().width()/2-80,20);
  stats_txcname_edit->setReadOnly(true);
  label=new QLabel(stats_txcname_edit,tr("CName:"),this);
  label->setGeometry(10,150,55,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Tx GPIO Status
  //
  stats_gpio_palette=palette();
  stats_gpio_palette.setColor(QPalette::Active,QColorGroup::Background,green);
  stats_gpio_palette.setColor(QPalette::Inactive,QColorGroup::Background,green);
  for(int i=0;i<8;i++) {
    for(int j=0;j<2;j++) {
      stats_gpio_trans_label[i+8*j]=
	new QLabel(QString().sprintf("%d",i+8*j+1),this);
      stats_gpio_trans_label[i+8*j]->setGeometry(70+(i%8)*22,176+j*22,17,17);
      stats_gpio_trans_label[i+8*j]->setFont(light_font);
      stats_gpio_trans_label[i+8*j]->setAlignment(AlignCenter);
      stats_gpio_trans_label[i+8*j]->setFrameStyle(QFrame::Box|QFrame::Plain);
      stats_gpio_trans_label[i+8*j]->setLineWidth(1);
      stats_gpio_trans_label[i+8*j]->setMidLineWidth(1);
    }
  }
  label=new QLabel(tr("GPIs:"),this);
  label->setGeometry(10,176,55,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Tx Ssrc
  //
  stats_txssrc_edit=new QLineEdit(this,"stats_txssrc_edit");
  stats_txssrc_edit->setGeometry(130,222,sizeHint().width()/2-140,20);
  stats_txssrc_edit->setReadOnly(true);
  label=new QLabel(stats_txssrc_edit,tr("SSRC:"),this);
  label->setGeometry(10,222,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Tx Sent Field
  //
  stats_txsent_edit=new QLineEdit(this,"stats_txsent_edit");
  stats_txsent_edit->setGeometry(130,244,sizeHint().width()/2-140,20);
  stats_txsent_edit->setReadOnly(true);
  label=new QLabel(stats_txsent_edit,tr("Packets Sent:"),this);
  label->setGeometry(10,244,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Tx Lost Field
  //
  stats_txlost_edit=new QLineEdit(this,"stats_txlost_edit");
  stats_txlost_edit->setGeometry(130,266,sizeHint().width()/2-140,20);
  stats_txlost_edit->setReadOnly(true);
  label=new QLabel(stats_txlost_edit,tr("Packets Lost:"),this);
  label->setGeometry(10,266,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Tx Percentage Lost Field
  //
  stats_txpercent_edit=new QLineEdit(this,"stats_txpercent_edit");
  stats_txpercent_edit->setGeometry(130,288,sizeHint().width()/2-140,20);
  stats_txpercent_edit->setReadOnly(true);
  label=new QLabel(stats_txpercent_edit,tr("% Lost:"),this);
  label->setGeometry(10,288,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Tx Percentage Lost Field
  //
  stats_txcpuload_edit=new QLineEdit(this,"stats_txcpuload_edit");
  stats_txcpuload_edit->setGeometry(130,310,sizeHint().width()/2-140,20);
  stats_txcpuload_edit->setReadOnly(true);
  label=new QLabel(stats_txcpuload_edit,tr("% CPU Load:"),this);
  label->setGeometry(10,310,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Remote Rx Connection Section
  //
  label=new QLabel(tr("Inbound Connection"),this);
  label->setGeometry(sizeHint().width()/2+10,130,sizeHint().width()/2-10,20);
  label->setFont(header_font);
  label->setAlignment(AlignVCenter|AlignLeft);

  //
  // Rx Cname Field
  //
  stats_rxcname_edit=new QLineEdit(this,"stats_rxcname_edit");
  stats_rxcname_edit->
    setGeometry(sizeHint().width()/2+70,150,sizeHint().width()/2-80,20);
  stats_rxcname_edit->setReadOnly(true);
  label=new QLabel(stats_rxcname_edit,tr("CName:"),this);
  label->setGeometry(sizeHint().width()/2+10,152,55,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Rx GPIO Status
  //
  for(int i=0;i<8;i++) {
    for(int j=0;j<2;j++) {
      stats_gpio_recv_label[i+8*j]=
	new QLabel(QString().sprintf("%d",i+8*j+1),this);
      stats_gpio_recv_label[i+8*j]->
	setGeometry(sizeHint().width()/2+70+(i%8)*22,176+j*22,17,17);
      stats_gpio_recv_label[i+8*j]->setFont(light_font);
      stats_gpio_recv_label[i+8*j]->setAlignment(AlignCenter);
      stats_gpio_recv_label[i+8*j]->setFrameStyle(QFrame::Box|QFrame::Plain);
      stats_gpio_recv_label[i+8*j]->setLineWidth(1);
      stats_gpio_recv_label[i+8*j]->setMidLineWidth(1);
    }
  }
  label=new QLabel(tr("GPOs:"),this);
  label->setGeometry(sizeHint().width()/2+10,176,55,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Rx Ssrc
  //
  stats_rxssrc_edit=new QLineEdit(this,"stats_rxssrc_edit");
  stats_rxssrc_edit->
    setGeometry(sizeHint().width()/2+130,222,sizeHint().width()/2-140,20);
  stats_rxssrc_edit->setReadOnly(true);
  label=new QLabel(stats_rxssrc_edit,tr("SSRC:"),this);
  label->setGeometry(sizeHint().width()/2+10,222,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Rx Sent Field
  //
  stats_rxsent_edit=new QLineEdit(this,"stats_rxsent_edit");
  stats_rxsent_edit->
    setGeometry(sizeHint().width()/2+130,244,sizeHint().width()/2-140,20);
  stats_rxsent_edit->setReadOnly(true);
  label=new QLabel(stats_rxsent_edit,tr("Packets Sent:"),this);
  label->setGeometry(sizeHint().width()/2+10,244,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Rx Received Field
  //
  stats_rxrecp_edit=new QLineEdit(this,"stats_rxrecp_edit");
  stats_rxrecp_edit->
    setGeometry(sizeHint().width()/2+130,266,sizeHint().width()/2-140,20);
  stats_rxrecp_edit->setReadOnly(true);
  label=new QLabel(stats_rxrecp_edit,tr("Packets Received:"),this);
  label->setGeometry(sizeHint().width()/2+10,266,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Rx Lost Field
  //
  stats_rxlost_edit=new QLineEdit(this,"stats_rxlost_edit");
  stats_rxlost_edit->
    setGeometry(sizeHint().width()/2+130,288,sizeHint().width()/2-140,20);
  stats_rxlost_edit->setReadOnly(true);
  label=new QLabel(stats_rxlost_edit,tr("Packets Lost:"),this);
  label->setGeometry(sizeHint().width()/2+10,288,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Rx Percentage Lost Field
  //
  stats_rxpercent_edit=new QLineEdit(this,"stats_rxpercent_edit");
  stats_rxpercent_edit->
    setGeometry(sizeHint().width()/2+130,310,sizeHint().width()/2-140,20);
  stats_rxpercent_edit->setReadOnly(true);
  label=new QLabel(stats_rxpercent_edit,tr("% Lost:"),this);
  label->setGeometry(sizeHint().width()/2+10,310,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  //
  // Rx Ringbuffer Frames Field
  //
  stats_rxplloffset_edit=new QLineEdit(this,"stats_rxplloffset_edit");
  stats_rxplloffset_edit->
    setGeometry(sizeHint().width()/2+130,332,sizeHint().width()/2-140,20);
  stats_rxplloffset_edit->setReadOnly(true);
  label=new QLabel(stats_rxplloffset_edit,tr("PLL Offset:"),this);
  label->setGeometry(sizeHint().width()/2+10,332,115,20);
  label->setFont(label_font);
  label->setAlignment(AlignVCenter|AlignRight);

  QTimer *timer=new QTimer(this,"population_timer");
  connect(timer,SIGNAL(timeout()),this,SLOT(updateData()));
  timer->start(STATSDIALOG_UPDATE_INTERVAL);
}


QSize StatsDialog::sizeHint() const
{
  return QSize(500,360);
} 


QSizePolicy StatsDialog::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void StatsDialog::updateMetadata(ControlConnect::MetadataField mf,
				 const QString &str)
{
  switch(mf) {
    case ControlConnect::MetadataName:
      stats_name_edit->setText(str);
      break;

    case ControlConnect::MetadataEmail:
      stats_email_edit->setText(str);
      break;

    case ControlConnect::MetadataPhone:
      stats_phone_edit->setText(str);
      break;

    case ControlConnect::MetadataLocation:
      stats_location_edit->setText(str);
      break;

    case ControlConnect::MetadataUnknown:
    case ControlConnect::MetadataLast:
      break;
  }
}


void StatsDialog::updateData()
{
  uint32_t mask=0;

  stats_txcname_edit->setText(stats_stats->transmitCname());
  if(stats_stats->transmitSsrc()==0) {
    stats_txssrc_edit->setText("");
  }
  else {
    stats_txssrc_edit->setText(GetSsrcString(stats_stats->transmitSsrc()));
  }
  stats_txsent_edit->
    setText(QString().sprintf("%u",stats_stats->transmitPacketsSent()));
  stats_txlost_edit->
    setText(QString().sprintf("%u",stats_stats->transmitPacketsLost()));
  stats_txpercent_edit->
    setText(QString().sprintf("%5.1lf",
			      100.0*stats_stats->transmitFractionLost()).
	    stripWhiteSpace());
  stats_txcpuload_edit->
    setText(QString().sprintf("%5.1lf",
			      100.0*(stats_stats->transmitCpuLoad())).
	    stripWhiteSpace());

  stats_rxcname_edit->setText(stats_stats->receptionCname());
  if(stats_stats->receptionSsrc()==0) {
    stats_rxssrc_edit->setText("");
  }
  else {
    stats_rxssrc_edit->setText(GetSsrcString(stats_stats->receptionSsrc()));
  }
  stats_rxsent_edit->
    setText(QString().sprintf("%u",stats_stats->receptionPacketsSent()));
  stats_rxrecp_edit->
    setText(QString().sprintf("%u",stats_stats->receptionPacketsReceived()));
  stats_rxlost_edit->
    setText(QString().sprintf("%u",stats_stats->receptionPacketsLost()));
  if(stats_stats->receptionPacketsSent()>0) {
    stats_rxpercent_edit->
      setText(QString().sprintf("%5.1lf",100.0*
				((double)stats_stats->receptionPacketsLost()/
				 (double)stats_stats->receptionPacketsSent())).
	      stripWhiteSpace());
  }
  else {
    stats_rxpercent_edit->setText("0.0");
  }
  stats_rxplloffset_edit->
    setText(QString().sprintf("%5.1lf",stats_stats->receptionPllOffset()));
  for(unsigned i=0;i<16;i++) {
    mask=1<<i;
    if((mask&stats_stats->transmitGpioMask())==0) {
      stats_gpio_trans_label[i]->setPalette(palette());
    }
    else {
      stats_gpio_trans_label[i]->setPalette(stats_gpio_palette);
    }
    if((mask&stats_stats->receptionGpioMask())==0) {
      stats_gpio_recv_label[i]->setPalette(palette());
    }
    else {
      stats_gpio_recv_label[i]->setPalette(stats_gpio_palette);
    }
  }
}


void StatsDialog::paintEvent(QPaintEvent *e)
{
  QPainter *p=new QPainter(this);
  p->setPen(black);
  p->setBrush(black);
  p->moveTo(sizeHint().width()/2,132);
  p->lineTo(sizeHint().width()/2,sizeHint().height()-10);
  delete p;
}


QString StatsDialog::GetSsrcString(unsigned ssrc)
{
  return QString().sprintf("%02X:%02X:%02X:%02X",
			   0xff&(ssrc>>24),
			   0xff&(ssrc>>16),
			   0xff&(ssrc>>8),
			   0xff&ssrc);
}
