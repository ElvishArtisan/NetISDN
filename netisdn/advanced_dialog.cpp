// advanced_dialog.cpp
//
// Edit an AudioSettings object.
//
//   (C) Copyright 2002-2003,2008 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: advanced_dialog.cpp,v 1.5 2008/11/11 17:41:02 cvs Exp $
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Library General Public License 
//   version 2 as published by the Free Software Foundation.
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

#include <qdialog.h>
#include <qstring.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qtextedit.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <math.h>

#include <advanced_dialog.h>
#include <passwd.h>


AdvancedDialog::AdvancedDialog(AudioSettings *settings,Codec *codec,
			       QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  lib_lib=settings;
  lib_codec=codec;

  //
  // Dialog Name
  //
  setCaption(tr("NetISDN - GPIO/Advanced Settings"));

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
  QFont font=QFont("Helvetica",12,QFont::Normal);
  font.setPixelSize(12);
  QFont bold_font=QFont("Helvetica",12,QFont::Bold);
  bold_font.setPixelSize(12);
  QFont button_font=QFont("Helvetica",16,QFont::Bold);
  button_font.setPixelSize(16);

  //
  // GPIO Section
  //
  QLabel *label=new QLabel(tr("GPIO Settings"),this);
  label->setGeometry(30,10,sizeHint().width()-20,20);
  label->setFont(button_font);

  label=new QLabel(tr("Command"),this);
  label->setGeometry(55,35,120,20);
  label->setFont(bold_font);
  label->setAlignment(AlignCenter);

  label=new QLabel(tr("IP Address"),this);
  label->setGeometry(185,35,120,20);
  label->setFont(bold_font);
  label->setAlignment(AlignCenter);

  label=new QLabel(tr("UDP Port"),this);
  label->setGeometry(315,35,60,20);
  label->setFont(bold_font);
  label->setAlignment(AlignCenter);

  label=new QLabel(tr("Command"),this);
  label->setGeometry(455,35,120,20);
  label->setFont(bold_font);
  label->setAlignment(AlignCenter);

  label=new QLabel(tr("IP Address"),this);
  label->setGeometry(585,35,120,20);
  label->setFont(bold_font);
  label->setAlignment(AlignCenter);

  label=new QLabel(tr("UDP Port"),this);
  label->setGeometry(715,35,60,20);
  label->setFont(bold_font);
  label->setAlignment(AlignCenter);

  for(int i=0;i<8;i++) {
    for(int j=0;j<2;j++) {
      lib_gpiocmd_edit[i+8*j][1]=new QLineEdit(this);
      lib_gpiocmd_edit[i+8*j][1]->setGeometry(55+j*400,55+i*50,120,20);
      lib_gpiocmd_edit[i+8*j][1]->setFont(font);
    
      lib_gpioaddr_edit[i+8*j][1]=new QLineEdit(this);
      lib_gpioaddr_edit[i+8*j][1]->setGeometry(185+j*400,55+i*50,120,20);
      lib_gpioaddr_edit[i+8*j][1]->setFont(font);
    
      lib_gpioport_spin[i+8*j][1]=new QSpinBox(this);
      lib_gpioport_spin[i+8*j][1]->setGeometry(315+j*400,55+i*50,65,20);
      lib_gpioport_spin[i+8*j][1]->setFont(font);
      lib_gpioport_spin[i+8*j][1]->setRange(0,65535);

      label=new QLabel(lib_gpiocmd_edit[i+8*j][1],
		       QString().sprintf("%d On",i+8*j+1),this);
      label->setGeometry(10+j*400,55+i*50,40,20);
      label->setFont(bold_font);
      label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  
      lib_gpiocmd_edit[i+8*j][0]=new QLineEdit(this);
      lib_gpiocmd_edit[i+8*j][0]->setGeometry(55+j*400,77+i*50,120,20);
      lib_gpiocmd_edit[i+8*j][0]->setFont(font);
    
      lib_gpioaddr_edit[i+8*j][0]=new QLineEdit(this);
      lib_gpioaddr_edit[i+8*j][0]->setGeometry(185+j*400,77+i*50,120,20);
      lib_gpioaddr_edit[i+8*j][0]->setFont(font);
    
      lib_gpioport_spin[i+8*j][0]=new QSpinBox(this);
      lib_gpioport_spin[i+8*j][0]->setGeometry(315+j*400,77+i*50,65,20);
      lib_gpioport_spin[i+8*j][0]->setFont(font);
      lib_gpioport_spin[i+8*j][0]->setRange(0,65535);
    
      label=new QLabel(lib_gpiocmd_edit[i+8*j][0],
		       QString().sprintf("%d Off",i+8*j+1),this);
      label->setGeometry(10+j*400,77+i*50,40,20);
      label->setFont(bold_font);
      label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
    }  
  }

  //
  // Advanced Section
  //
  label=new QLabel(tr("Advanced Settings"),this);
  label->setGeometry(30,460,sizeHint().width()-20,20);
  label->setFont(button_font);

  //
  // Loopback Mode
  //
  lib_loopback_box=new QComboBox(this,"lib_loopback_box");
  lib_loopback_box->setGeometry(120,487,120,20);
  lib_loopback_box->setFont(font);
  lib_loopback_box->insertItem(tr("Normal"));
  lib_loopback_box->insertItem(tr("Data Loopback"));
  label=
    new QLabel(lib_loopback_box,tr("Loopback Mode:"),this,
	       "lib_loopback_label");
  label->setGeometry(10,487,105,20);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // RML Port
  //
  lib_rml_port_spin=new QSpinBox(this,"lib_rml_port_spin");
  lib_rml_port_spin->setGeometry(120,509,65,20);
  lib_rml_port_spin->setRange(-2,0xffff);
  lib_rml_port_spin->setLineStep(2);
  lib_rml_port_spin->setSpecialValueText(tr("None"));
  connect(lib_rml_port_spin,SIGNAL(valueChanged(int)),
	  this,SLOT(rmlPortChangedData(int)));
  label=
    new QLabel(lib_rml_port_spin,tr("RML Port:"),this,
	       "lib_rml_port_label");
  label->setGeometry(10,509,105,20);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Input Level
  //
  lib_input_gain_spin=new QSpinBox(this,"lib_input_gain_spin");
  lib_input_gain_spin->setGeometry(120,531,40,20);
  lib_input_gain_spin->setRange(-20,20);
  label=
    new QLabel(lib_input_gain_spin,tr("Input Gain:"),this,
	       "lib_input_gain_label");
  label->setGeometry(10,531,105,20);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  label=
    new QLabel(lib_input_gain_spin,tr("dB"),this,
	       "lib_input_gain_label");
  label->setGeometry(165,531,30,20);
  label->setFont(font);
  label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

  //
  // Buffer Packets
  //
  lib_bufferpackets_spin=new QSpinBox(this,"lib_bufferpackets_spin");
  lib_bufferpackets_spin->setGeometry(120,553,60,20);
  lib_bufferpackets_spin->setRange(2,100);
  label=new QLabel(lib_bufferpackets_spin,tr("Receive Buffer"),
		   this,"lib_bufferpackets_label");
  label->setGeometry(10,553,105,20);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);
  label=
    new QLabel(lib_input_gain_spin,tr("packets"),this,
	       "lib_input_gain_label");
  label->setGeometry(185,553,50,20);
  label->setFont(font);
  label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

  //
  // Use Realtime
  //
  lib_realtime_box=new QComboBox(this,"lib_realtime_box");
  lib_realtime_box->setGeometry(120,575,60,20);
  lib_realtime_box->setFont(font);
  lib_realtime_box->insertItem(tr("No"));
  lib_realtime_box->insertItem(tr("Yes"));
  label=
    new QLabel(lib_realtime_box,tr("Use Realtime:"),this,"lib_realtime_label");
  label->setGeometry(10,575,105,20);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  //  Ok Button
  //
  QPushButton *ok_button=new QPushButton(this,"ok_button");
  ok_button->setGeometry(sizeHint().width()-180,sizeHint().height()-60,80,50);
  ok_button->setDefault(true);
  ok_button->setFont(button_font);
  ok_button->setText(tr("&OK"));
  connect(ok_button,SIGNAL(clicked()),this,SLOT(okData()));

  //
  //  Cancel Button
  //
  QPushButton *cancel_button=new QPushButton(this,"cancel_button");
  cancel_button->
    setGeometry(sizeHint().width()-90,sizeHint().height()-60,80,50);
  cancel_button->setFont(button_font);
  cancel_button->setText(tr("&Cancel"));
  connect(cancel_button,SIGNAL(clicked()),this,SLOT(cancelData()));

  //
  // Populate Fields
  //
  for(int i=0;i<16;i++) {
    for(int j=0;j<2;j++) {
      lib_gpiocmd_edit[i][j]->setText(settings->gpioCommand(i,j));
      lib_gpioaddr_edit[i][j]->setText(settings->gpioAddress(i,j).toString());
      lib_gpioport_spin[i][j]->setValue(settings->gpioPort(i,j));
    }
  }
  lib_loopback_box->setCurrentItem(lib_codec->isLoopedBack());
  lib_rml_port_spin->setValue(lib_lib->rmlPort());
  lib_input_gain_spin->setValue(lib_lib->inputGain()/100);
  lib_bufferpackets_spin->setValue(lib_codec->receiveBuffers());
  lib_realtime_box->setCurrentItem(lib_lib->useRealtime());
}


QSize AdvancedDialog::sizeHint() const
{
  return QSize(800,611);
} 


QSizePolicy AdvancedDialog::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void AdvancedDialog::rmlPortChangedData(int v)
{
  if((v%2)!=0) {
    lib_rml_port_spin->setValue(2*(v/2));
  }
}


void AdvancedDialog::okData()
{
  QHostAddress addr;

  for(int i=0;i<16;i++) {
    for(int j=0;j<2;j++) {
      lib_lib->setGpioCommand(i,j,lib_gpiocmd_edit[i][j]->text());
      addr.setAddress(lib_gpioaddr_edit[i][j]->text());
      lib_lib->setGpioAddress(i,j,addr);
      lib_lib->setGpioPort(i,j,lib_gpioport_spin[i][j]->value());
    }
  }
  lib_codec->setLoopback(lib_loopback_box->currentItem());
  lib_lib->setRmlPort(lib_rml_port_spin->value());
  lib_lib->setInputGain(lib_input_gain_spin->value()*100);
  lib_codec->setReceiveBuffers(lib_bufferpackets_spin->value());
  lib_lib->setUseRealtime(lib_realtime_box->currentItem());

  done(0);
}


void AdvancedDialog::cancelData()
{
  done(1);
}
