// audiosettings_dialog.cpp
//
// Edit an AudioSettings object.
//
//   (C) Copyright 2002-2003,2008 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: audiosettings_dialog.cpp,v 1.25 2008/11/14 18:34:02 cvs Exp $
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
#include <audiosettings_dialog.h>
#include <passwd.h>


AudioSettingsDialog::AudioSettingsDialog(AudioSettings *settings,
					 Codec *codec,
					 PaDeviceIndex *in_pdev,
					 PaDeviceIndex *out_pdev,bool mpeg,
					 QWidget *parent,const char *name)
  : QDialog(parent,name,true)
{
  lib_lib=settings;
  lib_codec=codec;
  lib_input_index=in_pdev;
  lib_output_index=out_pdev;

  //
  // Dialog Name
  //
  setCaption(tr("Edit Settings"));

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
  QFont direction_font=QFont("Helvetica",14,QFont::Bold);
  direction_font.setPixelSize(14);
  QFont button_font=QFont("Helvetica",16,QFont::Bold);
  button_font.setPixelSize(16);

  //
  // Audio Transmission Settings Section
  //
  QLabel *label=new QLabel(tr("Audio Settings"),this);
  label->setGeometry(10,10,sizeHint().width()-20,20);
  label->setFont(button_font);
  label=new QLabel(tr("Transmit"),this);
  label->setGeometry(20,35,sizeHint().width()-20,20);
  label->setFont(direction_font);

  //
  // Transmit Format
  //
  lib_trans_format_box=new QComboBox(this,"lib_trans_format_box");
  lib_trans_format_box->setGeometry(120,57,150,20);
  lib_trans_format_box->setFont(font);
  connect(lib_trans_format_box,SIGNAL(activated(int)),
	  this,SLOT(settingsChangedData(int)));
  QLabel *lib_format_label=new QLabel(lib_trans_format_box,tr("&Algorithm:"),
				      this,"lib_format_label");
  lib_format_label->setGeometry(10,57,105,20);
  lib_format_label->setFont(font);
  lib_format_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Transmit Channels
  //
  lib_trans_channels_box=new QComboBox(this,"lib_trans_channels_box");
  lib_trans_channels_box->setGeometry(120,79,60,20);
  lib_trans_channels_box->setFont(font);
  connect(lib_trans_channels_box,SIGNAL(activated(int)),
	  this,SLOT(settingsChangedData(int)));
  connect(lib_trans_channels_box,SIGNAL(activated(int)),
	  this,SLOT(settingsChangedData(int)));
  QLabel *lib_trans_channels_label=new QLabel(lib_trans_channels_box,
					tr("&Channels:"),
					this,"lib_channels_label");
  lib_trans_channels_label->setGeometry(10,79,105,20);
  lib_trans_channels_label->setFont(font);
  lib_trans_channels_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Transmit Sample Rate
  //
  lib_trans_samprate_box=new QComboBox(this,"lib_trans_samprate_box");
  lib_trans_samprate_box->setGeometry(120,101,100,20);
  lib_trans_samprate_box->setFont(font);
  connect(lib_trans_samprate_box,SIGNAL(activated(int)),
	  this,SLOT(settingsChangedData(int)));
  QLabel *lib_samprate_label=
    new QLabel(lib_trans_samprate_box,tr("&Sample Rate:"),this,
	       "lib_samprate_label");
  lib_samprate_label->setGeometry(10,101,105,20);
  lib_samprate_label->setFont(font);
  lib_samprate_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Transmit Bitrate
  //
  lib_trans_bitrate_box=new QComboBox(this,"lib_trans_bitrate_box");
  lib_trans_bitrate_box->setGeometry(120,123,100,20);
  lib_trans_bitrate_box->setFont(font);
  lib_trans_bitrate_label=new QLabel(lib_trans_bitrate_box,tr("&Bitrate:"),this,
			       "lib_trans_bitrate_label");
  lib_trans_bitrate_label->setGeometry(10,123,105,20);
  lib_trans_bitrate_label->setFont(font);
  lib_trans_bitrate_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);


  //
  // Transmit Hyperstreaming
  //
  lib_trans_hyperstream_box=new QComboBox(this,"lib_trans_hyperstream_box");
  lib_trans_hyperstream_box->setGeometry(120,145,100,20);
  lib_trans_hyperstream_box->setFont(font);
  lib_trans_hyperstream_box->insertItem(tr("Disabled"));
  lib_trans_hyperstream_box->insertItem(tr("Enabled"));
  label=new QLabel(lib_trans_hyperstream_box,tr("&HyperStream:"),this,
		   "lib_trans_hyperstream_label");
  label->setGeometry(10,145,105,20);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Audio Reception Settings Section
  //
  label=new QLabel(tr("Receive"),this);
  label->setGeometry(20,175,sizeHint().width()-20,20);
  label->setFont(direction_font);

  //
  // Receive Format
  //
  lib_recv_format_box=new QComboBox(this,"lib_recv_format_box");
  lib_recv_format_box->setGeometry(120,197,150,20);
  lib_recv_format_box->setFont(font);
  connect(lib_recv_format_box,SIGNAL(activated(int)),
	  this,SLOT(settingsChangedData(int)));
  lib_format_label=new QLabel(lib_recv_format_box,tr("&Algorithm:"),
			      this,"lib_format_label");
  lib_format_label->setGeometry(10,197,105,20);
  lib_format_label->setFont(font);
  lib_format_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Receive Channels
  //
  lib_recv_channels_box=new QComboBox(this,"lib_recv_channels_box");
  lib_recv_channels_box->setGeometry(120,219,60,20);
  lib_recv_channels_box->setFont(font);
  connect(lib_recv_channels_box,SIGNAL(activated(int)),
	  this,SLOT(settingsChangedData(int)));
  connect(lib_recv_channels_box,SIGNAL(activated(int)),
	  this,SLOT(settingsChangedData(int)));
  lib_recv_channels_label=new QLabel(lib_recv_channels_box,
					tr("&Channels:"),
					this,"lib_channels_label");
  lib_recv_channels_label->setGeometry(10,219,105,20);
  lib_recv_channels_label->setFont(font);
  lib_recv_channels_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Receive Bitrate
  //
  lib_recv_bitrate_box=new QComboBox(this,"lib_recv_bitrate_box");
  lib_recv_bitrate_box->setGeometry(120,241,100,20);
  lib_recv_bitrate_box->setFont(font);
  lib_recv_bitrate_label=new QLabel(lib_recv_bitrate_box,tr("&Bitrate:"),this,
			       "lib_recv_bitrate_label");
  lib_recv_bitrate_label->setGeometry(10,241,105,20);
  lib_recv_bitrate_label->setFont(font);
  lib_recv_bitrate_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Receive Hyperstreaming
  //
  lib_recv_hyperstream_box=new QComboBox(this,"lib_recv_hyperstream_box");
  lib_recv_hyperstream_box->setGeometry(120,263,100,20);
  lib_recv_hyperstream_box->setFont(font);
  lib_recv_hyperstream_box->insertItem(tr("Disabled"));
  lib_recv_hyperstream_box->insertItem(tr("Enabled"));
  lib_recv_hyperstream_label=
    new QLabel(lib_recv_hyperstream_box,tr("H&yperStream:"),this,
	       "lib_recv_hyperstream_label");
  lib_recv_hyperstream_label->setGeometry(10,263,105,20);
  lib_recv_hyperstream_label->setFont(font);
  lib_recv_hyperstream_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Devices Section
  //
  label=new QLabel(tr("Audio Devices"),this);
  label->setGeometry(10,293,sizeHint().width()-20,20);
  label->setFont(button_font);

  //
  // Input Device
  //
  lib_input_box=new QComboBox(this,"lib_input_box");
  lib_input_box->setGeometry(120,315,sizeHint().width()-130,20);
  lib_input_box->setFont(font);
  QLabel *lib_input_label=
    new QLabel(lib_input_box,tr("&Input Device:"),this,
	       "lib_input_label");
  lib_input_label->setGeometry(10,315,105,20);
  lib_input_label->setFont(font);
  lib_input_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Local Voice Processor
  //
  lib_trans_preprocess_box=new QCheckBox(this,"lib_trans_preprocess_box");
  lib_trans_preprocess_box->setGeometry(120,337,15,15);
  lib_trans_preprocess_label=
    new QLabel(lib_trans_preprocess_box,tr("Enable &Voice Processor"),this,
	       "lib_trans_preprocess_label");
  lib_trans_preprocess_label->setGeometry(140,335,sizeHint().width()-35,18);
  lib_trans_preprocess_label->setFont(font);
  lib_trans_preprocess_label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

  //
  // Output Device
  //
  lib_output_box=new QComboBox(this,"lib_output_box");
  lib_output_box->setGeometry(120,359,sizeHint().width()-130,20);
  lib_output_box->setFont(font);
  QLabel *lib_output_label=
    new QLabel(lib_output_box,tr("&Output Device:"),this,
	       "lib_output_label");
  lib_output_label->setGeometry(10,359,105,20);
  lib_output_label->setFont(font);
  lib_output_label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Remote Voice Processor
  //
  lib_recv_preprocess_box=new QCheckBox(this,"lib_recv_preprocess_box");
  lib_recv_preprocess_box->setGeometry(120,381,15,15);
  lib_recv_preprocess_label=
    new QLabel(lib_recv_preprocess_box,tr("Enable &Voice Processor"),this,
	       "lib_recv_preprocess_label");
  lib_recv_preprocess_label->setGeometry(140,379,sizeHint().width()-35,18);
  lib_recv_preprocess_label->setFont(font);
  lib_recv_preprocess_label->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

  //
  // Identity Section
  //
  label=new QLabel(tr("Identity Settings"),this);
  label->setGeometry(10,403,sizeHint().width()-20,20);
  label->setFont(button_font);

  //
  // Username
  //
  lib_username_edit=new QLineEdit(this,"lib_username_edit");
  lib_username_edit->setGeometry(120,425,sizeHint().width()-130,20);
  lib_username_edit->setFont(font);
  label=new QLabel(lib_username_edit,tr("&Login Name:"),this,
		   "lib_username_label");
  label->setGeometry(10,425,105,20);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  //  Password Button
  //
  QPushButton *button=new QPushButton(this,"button");
  button->setGeometry(140,447,sizeHint().width()-180,26);
  button->setFont(bold_font);
  button->setText(tr("Change &Password"));
  connect(button,SIGNAL(clicked()),this,SLOT(passwordData()));

  //
  // Display Name
  //
  lib_name_edit=new QLineEdit(this,"lib_name_edit");
  lib_name_edit->setGeometry(120,481,sizeHint().width()-130,20);
  lib_name_edit->setFont(font);
  label=new QLabel(lib_name_edit,tr("Display &Name:"),this,
		   "lib_name_label");
  label->setGeometry(10,481,105,20);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Phone
  //
  lib_phone_edit=new QLineEdit(this,"lib_phone_edit");
  lib_phone_edit->setGeometry(120,503,sizeHint().width()-130,20);
  lib_phone_edit->setFont(font);
  label=new QLabel(lib_phone_edit,tr("&Phone Number:"),this,"lib_phone_label");
  label->setGeometry(10,503,105,20);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  // Email
  //
  lib_email_edit=new QLineEdit(this,"lib_email_edit");
  lib_email_edit->setGeometry(120,525,sizeHint().width()-130,20);
  lib_email_edit->setFont(font);
  label=
    new QLabel(lib_email_edit,tr("&E-mail Address:"),this,"lib_email_label");
  label->setGeometry(10,525,105,20);
  label->setFont(font);
  label->setAlignment(AlignRight|AlignVCenter|ShowPrefix);

  //
  //  Advanced Button
  //
  button=new QPushButton(this,"advanced_button");
  button->setGeometry((sizeHint().width()-200)/2,553,200,50);
  button->setFont(bold_font);
  button->setText(tr("GPIO/&Advanced Settings"));
  connect(button,SIGNAL(clicked()),this,SLOT(advancedData()));

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

  //
  // Transmit Settings
  //
#ifdef HAVE_MPEG_L3
  lib_trans_format_box->insertItem(tr("MPEG Layer 3"));
#endif  // HAVE_MPEG_L3
#ifdef HAVE_SPEEX
  lib_trans_format_box->insertItem(tr("Speex"));
#endif  // HAVE_SPEEX
#ifdef HAVE_VORBIS
  lib_trans_format_box->insertItem(tr("Vorbis"));
#endif  // HAVE_VORBIS
  QString str;
  switch(lib_lib->format(AudioSettings::Transmit,false)) {
      case AudioSettings::Vorbis:
	str="Vorbis";
	lib_trans_samprate_box->insertItem("32000");
	lib_trans_samprate_box->insertItem("48000");
	break;

      case AudioSettings::Layer3:
	str="MPEG Layer 3";
	lib_trans_samprate_box->insertItem("32000");
	lib_trans_samprate_box->insertItem("48000");
	break;

      case AudioSettings::Speex:
	str="Speex";
	lib_trans_samprate_box->insertItem("32000");
	break;
  }
  for(int i=0;i<lib_trans_format_box->count();i++) {
    if(lib_trans_format_box->text(i)==str) {
      lib_trans_format_box->setCurrentItem(i);
    }
  }
  lib_trans_channels_box->insertItem("1");
  lib_trans_channels_box->insertItem("2");
  lib_trans_channels_box->
    setCurrentItem(lib_lib->channels(AudioSettings::Transmit,false)-1);
  for(int i=0;i<lib_trans_samprate_box->count();i++) {
    if(lib_trans_samprate_box->text(i).toInt()==
       (int)lib_lib->sampleRate(),false) {
      lib_trans_samprate_box->setCurrentItem(i);
    }
  }
  CheckTransmitSettings(lib_lib->format(AudioSettings::Transmit,false),
			lib_lib->bitRate(AudioSettings::Transmit,false));

  //
  // Receive Settings
  //
  lib_recv_format_box->insertItem(tr("Use Transmit Settings"));
#ifdef HAVE_MPEG_L3
  lib_recv_format_box->insertItem(tr("MPEG Layer 3"));
#endif  // HAVE_MPEG_L3
#ifdef HAVE_SPEEX
  lib_recv_format_box->insertItem(tr("Speex"));
#endif  // HAVE_SPEEX
#ifdef HAVE_VORBIS
  lib_recv_format_box->insertItem(tr("Vorbis"));
#endif  // HAVE_VORBIS
  switch(lib_lib->format(AudioSettings::Receive,false)) {
    case AudioSettings::Vorbis:
      str="Vorbis";
      lib_trans_samprate_box->insertItem("32000");
      lib_trans_samprate_box->insertItem("48000");
      break;
      
    case AudioSettings::Layer3:
      str="MPEG Layer 3";
      break;
      
    case AudioSettings::Speex:
      str="Speex";
      break;
      
    case AudioSettings::NoFormat:
      str="Use Trasnmit Settings";
      break;
  }
  for(int i=0;i<lib_recv_format_box->count();i++) {
    if(lib_recv_format_box->text(i)==str) {
      lib_recv_format_box->setCurrentItem(i);
    }
  }
  lib_recv_channels_box->insertItem("1");
  lib_recv_channels_box->insertItem("2");
  lib_recv_channels_box->
    setCurrentItem(lib_lib->channels(AudioSettings::Receive,false)-1);
  settingsChangedData(0);

  for(PaDeviceIndex i=0;i<Pa_GetDeviceCount();i++) {
    const PaDeviceInfo *info=Pa_GetDeviceInfo(i);
    if((info->maxInputChannels>=2)&&
       (Pa_GetHostApiInfo(info->hostApi)->type!=paMME)&&
       (Pa_GetHostApiInfo(info->hostApi)->type!=paASIO)) {
      lib_input_box->insertItem(info->name);
      lib_input_pdevs.push_back(i);
      if(i==*in_pdev) {
	lib_input_box->setCurrentItem(lib_input_box->count()-1);
      }
    }
    if((info->maxOutputChannels>=2)&&
       (Pa_GetHostApiInfo(info->hostApi)->type!=paMME)&&
       (Pa_GetHostApiInfo(info->hostApi)->type!=paASIO)) {
      lib_output_box->insertItem(info->name);
      lib_output_pdevs.push_back(i);
      if(i==*out_pdev) {
	lib_output_box->setCurrentItem(lib_output_box->count()-1);
      }
    }
  }
  lib_trans_preprocess_box->
    setChecked(lib_lib->enableProcessor(AudioSettings::Transmit,false));
  lib_recv_preprocess_box->
    setChecked(lib_lib->enableProcessor(AudioSettings::Receive,false));
  if(lib_lib->streamRatio(AudioSettings::Transmit,false)>1) {
    lib_trans_hyperstream_box->setCurrentItem(1);
  }
  else {
    lib_trans_hyperstream_box->setCurrentItem(0);
  }
  if(lib_lib->streamRatio(AudioSettings::Receive,false)>1) {
    lib_recv_hyperstream_box->setCurrentItem(1);
  }
  else {
    lib_recv_hyperstream_box->setCurrentItem(0);
  }
  lib_username_edit->setText(lib_lib->username());
  lib_password=lib_lib->password();
  lib_name_edit->setText(lib_codec->rtpSdesField(Codec::SdesName));
  lib_phone_edit->setText(lib_codec->rtpSdesField(Codec::SdesPhone));
  lib_email_edit->setText(lib_codec->rtpSdesField(Codec::SdesEmail));
}


AudioSettingsDialog::~AudioSettingsDialog()
{
  delete lib_trans_channels_box;
  delete lib_trans_samprate_box;
  delete lib_trans_bitrate_box;
}


QSize AudioSettingsDialog::sizeHint() const
{
  return QSize(400,673);
} 


QSizePolicy AudioSettingsDialog::sizePolicy() const
{
  return QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}


void AudioSettingsDialog::settingsChangedData(int n)
{
  bool state;
  state=(lib_trans_channels_box->currentItem()==0);
  lib_trans_preprocess_box->setEnabled(state);
  lib_trans_preprocess_label->setEnabled(state);

  state=(lib_recv_channels_box->currentItem()==0);
  if(lib_recv_format_box->currentItem()==0) {
    state&=(lib_trans_channels_box->currentItem()==0);
  }
  lib_recv_preprocess_box->setEnabled(state);
  lib_recv_preprocess_label->setEnabled(state);

  CheckTransmitSettings(ReadFormat(AudioSettings::Transmit),
			lib_lib->bitRate(AudioSettings::Transmit,false));
  CheckReceiveSettings(ReadFormat(AudioSettings::Receive),
		       lib_lib->bitRate(AudioSettings::Receive,false));
}


void AudioSettingsDialog::passwordData()
{
  Passwd *p=new Passwd(&lib_password,this);
  p->exec();
  delete p;
}


void AudioSettingsDialog::advancedData()
{
  AdvancedDialog *p=new AdvancedDialog(lib_lib,lib_codec,this);
  p->exec();
  delete p;
}


void AudioSettingsDialog::okData()
{
  if(lib_username_edit->text().isEmpty()) {
    QMessageBox::warning(this,tr("NetISDN - Missing Username"),
			 tr("You must supply a valid username!"));
    return;
  }
  unsigned rate=0;
  lib_lib->setFormat(AudioSettings::Transmit,
		     ReadFormat(AudioSettings::Transmit));
  lib_lib->setChannels(AudioSettings::Transmit,
		       lib_trans_channels_box->currentItem()+1);
  rate=0;
  sscanf(lib_trans_bitrate_box->currentText(),"%d",&rate);
  lib_lib->setBitRate(AudioSettings::Transmit,rate*1000);
  lib_lib->setFormat(AudioSettings::Receive,
		     ReadFormat(AudioSettings::Receive));
  lib_lib->setChannels(AudioSettings::Receive,
		       lib_recv_channels_box->currentItem()+1);
  rate=0;
  if(!lib_recv_bitrate_box->currentText().isEmpty()) {
    sscanf(lib_recv_bitrate_box->currentText(),"%d",&rate);
  }
  lib_lib->setBitRate(AudioSettings::Receive,rate*1000);
  sscanf(lib_trans_samprate_box->currentText(),"%d",&rate);
  lib_lib->setSampleRate(rate);
  if(lib_input_box->count()>0) {
    *lib_input_index=lib_input_pdevs[lib_input_box->currentItem()];
  }
  if(lib_output_box->count()>0) {
    *lib_output_index=lib_output_pdevs[lib_output_box->currentItem()];
  }
  lib_lib->setEnableProcessor(AudioSettings::Transmit,
			      lib_trans_preprocess_box->isChecked());
  lib_lib->setEnableProcessor(AudioSettings::Receive,
			      lib_recv_preprocess_box->isChecked());
  switch(lib_trans_hyperstream_box->currentItem()) {
    case 1:
      lib_lib->setStreamRatio(AudioSettings::Transmit,2);
      break;

    default:
      lib_lib->setStreamRatio(AudioSettings::Transmit,1);
      break;
  }
  switch(lib_recv_hyperstream_box->currentItem()) {
    case 1:
      lib_lib->setStreamRatio(AudioSettings::Receive,2);
      break;

    default:
      lib_lib->setStreamRatio(AudioSettings::Receive,1);
      break;
  }
  lib_lib->setUsername(lib_username_edit->text());
  lib_lib->setPassword(lib_password);
  lib_codec->setRtpSdesField(Codec::SdesName,lib_name_edit->text());
  lib_codec->setRtpSdesField(Codec::SdesPhone,lib_phone_edit->text());
  lib_codec->setRtpSdesField(Codec::SdesEmail,lib_email_edit->text());

  done(0);
}


void AudioSettingsDialog::cancelData()
{
  done(1);
}


void AudioSettingsDialog::CheckTransmitSettings(AudioSettings::Format fmt,
						int rate)
{
  CheckSettings(AudioSettings::Transmit,fmt,rate,lib_trans_channels_box,
		lib_trans_samprate_box,
		lib_trans_bitrate_box,lib_trans_bitrate_label,
		lib_trans_samprate_box->currentText().toInt());
}


void AudioSettingsDialog::CheckReceiveSettings(AudioSettings::Format fmt,
					       int rate)
{
  lib_recv_channels_box->setDisabled(fmt==AudioSettings::NoFormat);
  lib_recv_channels_label->setDisabled(fmt==AudioSettings::NoFormat);
  lib_recv_bitrate_box->setDisabled(fmt==AudioSettings::NoFormat);
  lib_recv_bitrate_label->setDisabled(fmt==AudioSettings::NoFormat);
  lib_recv_hyperstream_box->setDisabled(fmt==AudioSettings::NoFormat);
  lib_recv_hyperstream_label->setDisabled(fmt==AudioSettings::NoFormat);
  if(fmt!=AudioSettings::NoFormat) {
    CheckSettings(AudioSettings::Receive,fmt,rate,lib_recv_channels_box,NULL,
		  lib_recv_bitrate_box,
		  lib_recv_bitrate_label,
		  lib_trans_samprate_box->currentText().toInt());
  }
}
  

void AudioSettingsDialog::CheckSettings(AudioSettings::Direction dir,
					AudioSettings::Format fmt,int rate,
					QComboBox *channels_box,
					QComboBox *samprate_box,
					QComboBox *bitrate_box,
					QLabel *bitrate_label,int samprate)
{
  int channels=channels_box->currentText().toInt();
  bitrate_box->clear();
  switch(fmt) {
      case AudioSettings::Vorbis:
	if(samprate_box!=NULL) {
	  samprate_box->clear();
	  samprate_box->insertItem("32000");
	  samprate_box->insertItem("48000");
	}
	channels_box->clear();
	channels_box->insertItem("1");
	channels_box->insertItem("2");
	bitrate_box->setDisabled(true);
	bitrate_label->setText(tr("Bitrate"));
	switch(channels) {
	  case 1:
	    bitrate_box->setEnabled(true);
	    bitrate_box->insertItem("64 kbps");
	    bitrate_box->insertItem("96 kbps");
	    bitrate_box->insertItem("128 kbps");
	    bitrate_box->insertItem("160 kbps");
	    bitrate_box->insertItem("192 kbps");
	    bitrate_box->insertItem("224 kbps");
	    bitrate_box->insertItem("256 kbps");
	    bitrate_box->insertItem("288 kbps");
	    bitrate_box->insertItem("320 kbps");
	    bitrate_box->insertItem("352 kbps");
	    bitrate_box->insertItem("384 kbps");
	    switch(lib_lib->bitRate(dir)) {
	      case 64000:
		bitrate_box->setCurrentItem(0);
		break;
		
	      case 96000:
		bitrate_box->setCurrentItem(1);
		break;
		
	      case 128000:
		bitrate_box->setCurrentItem(2);
		break;
		
	      case 160000:
		bitrate_box->setCurrentItem(3);
		break;
		
	      case 192000:
		bitrate_box->setCurrentItem(4);
		break;
		
	      case 224000:
		bitrate_box->setCurrentItem(5);
		break;
		
	      case 256000:
		bitrate_box->setCurrentItem(6);
		break;
		
	      case 288000:
		bitrate_box->setCurrentItem(7);
		break;
		
	      case 320000:
		bitrate_box->setCurrentItem(8);
		break;

	      case 352000:
		bitrate_box->setCurrentItem(9);
		break;

	      case 384000:
		bitrate_box->setCurrentItem(10);
		break;
	    }
	    break;
	  case 2:
	    bitrate_box->setEnabled(true);
	    bitrate_box->insertItem("64 kbps");
	    bitrate_box->insertItem("96 kbps");
	    bitrate_box->insertItem("128 kbps");
	    bitrate_box->insertItem("160 kbps");
	    bitrate_box->insertItem("192 kbps");
	    bitrate_box->insertItem("224 kbps");
	    bitrate_box->insertItem("256 kbps");
	    bitrate_box->insertItem("288 kbps");
	    bitrate_box->insertItem("320 kbps");
	    bitrate_box->insertItem("352 kbps");
	    bitrate_box->insertItem("384 kbps");
	    switch(lib_lib->bitRate(dir)) {
	      case 64000:
		bitrate_box->setCurrentItem(0);
		break;
		
	      case 96000:
		bitrate_box->setCurrentItem(1);
		break;
		
	      case 128000:
		bitrate_box->setCurrentItem(2);
		break;
		
	      case 160000:
		bitrate_box->setCurrentItem(3);
		break;
		
	      case 192000:
		bitrate_box->setCurrentItem(4);
		break;
		
	      case 224000:
		bitrate_box->setCurrentItem(5);
		break;
		
	      case 256000:
		bitrate_box->setCurrentItem(6);
		break;
		
	      case 288000:
		bitrate_box->setCurrentItem(7);
		break;
		
	      case 320000:
		bitrate_box->setCurrentItem(8);
		break;

	      case 352000:
		bitrate_box->setCurrentItem(9);
		break;

	      case 384000:
		bitrate_box->setCurrentItem(10);
		break;
	    }
	    break;
	}
	break;

      case AudioSettings::Speex:
	if(samprate_box!=NULL) {
	  samprate_box->clear();
	  samprate_box->insertItem("32000");
	}
	channels_box->clear();
	channels_box->insertItem("1");
	bitrate_box->setEnabled(true);
	bitrate_label->setText(tr("Quality"));
	bitrate_box->clear();
	bitrate_box->insertItem("0");
	bitrate_box->insertItem("1");
	bitrate_box->insertItem("2");
	bitrate_box->insertItem("3");
	bitrate_box->insertItem("4");
	bitrate_box->insertItem("5");
	bitrate_box->insertItem("6");
	bitrate_box->insertItem("7");
	bitrate_box->insertItem("8");
	bitrate_box->insertItem("9");
	bitrate_box->insertItem("10");
	if((rate>=0)&&(rate<=10000)) {
	  bitrate_box->setCurrentItem(rate/1000);
	}
	break;

      case AudioSettings::Layer3:
	bitrate_box->setEnabled(true);
	if(samprate_box!=NULL) {
	  samprate_box->clear();
	  samprate_box->insertItem("32000");
	  samprate_box->insertItem("48000");
	}
	channels_box->clear();
	channels_box->insertItem("1");
	channels_box->insertItem("2");
	bitrate_label->setText(tr("Bitrate"));
	switch(samprate) {
	  case 32000:
	    switch(channels) {
	      case 1:
		bitrate_box->insertItem("48 kbps");
		bitrate_box->insertItem("56 kbps");
		bitrate_box->insertItem("64 kbps");
		bitrate_box->insertItem("80 kbps");
		bitrate_box->insertItem("96 kbps");
		bitrate_box->insertItem("112 kbps");
		bitrate_box->insertItem("128 kbps");
		bitrate_box->insertItem("160 kbps");
		bitrate_box->insertItem("192 kbps");
		bitrate_box->insertItem("224 kbps");
		bitrate_box->insertItem("256 kbps");
		bitrate_box->insertItem("320 kbps");
		switch(lib_lib->bitRate(dir)) {
		  case 48000:
		    bitrate_box->setCurrentItem(0);
		    break;
		    
		  case 56000:
		    bitrate_box->setCurrentItem(1);
		    break;
		    
		  case 64000:
		    bitrate_box->setCurrentItem(2);
		    break;
		    
		  case 80000:
		    bitrate_box->setCurrentItem(3);
		    break;
		    
		  case 96000:
		    bitrate_box->setCurrentItem(4);
		    break;
		    
		  case 112000:
		    bitrate_box->setCurrentItem(5);
		    break;
		    
		  case 128000:
		    bitrate_box->setCurrentItem(6);
		    break;
		    
		  case 160000:
		    bitrate_box->setCurrentItem(7);
		    break;
		    
		  case 192000:
		    bitrate_box->setCurrentItem(8);
		    break;
		    
		  case 224000:
		    bitrate_box->setCurrentItem(9);
		    break;
		    
		  case 256000:
		    bitrate_box->setCurrentItem(10);
		    break;
		    
		  case 320000:
		    bitrate_box->setCurrentItem(11);
		    break;
		}
		break;

	      case 2:
		bitrate_box->setEnabled(true);
		bitrate_box->insertItem("128 kbps");
		bitrate_box->insertItem("160 kbps");
		bitrate_box->insertItem("192 kbps");
		bitrate_box->insertItem("224 kbps");
		bitrate_box->insertItem("256 kbps");
		bitrate_box->insertItem("320 kbps");
		switch(lib_lib->bitRate(dir)) {
		  case 128000:
		    bitrate_box->setCurrentItem(0);
		    break;
		    
		  case 160000:
		    bitrate_box->setCurrentItem(1);
		    break;
		    
		  case 192000:
		    bitrate_box->setCurrentItem(2);
		    break;
		    
		  case 224000:
		    bitrate_box->setCurrentItem(3);
		    break;
		    
		  case 256000:
		    bitrate_box->setCurrentItem(4);
		    break;
		    
		  case 320000:
		    bitrate_box->setCurrentItem(5);
		    break;
		}
		break;
	    }
	    break;
	  case 48000:
	    switch(channels) {
	      case 1:
		bitrate_box->setEnabled(true);
		bitrate_box->insertItem("64 kbps");
		bitrate_box->insertItem("80 kbps");
		bitrate_box->insertItem("96 kbps");
		bitrate_box->insertItem("112 kbps");
		bitrate_box->insertItem("128 kbps");
		bitrate_box->insertItem("160 kbps");
		bitrate_box->insertItem("192 kbps");
		bitrate_box->insertItem("224 kbps");
		bitrate_box->insertItem("256 kbps");
		bitrate_box->insertItem("320 kbps");
		switch(lib_lib->bitRate(dir)) {
		  case 64000:
		    bitrate_box->setCurrentItem(0);
		    break;
		    
		  case 80000:
		    bitrate_box->setCurrentItem(1);
		    break;
		    
		  case 96000:
		    bitrate_box->setCurrentItem(2);
		    break;
		    
		  case 112000:
		    bitrate_box->setCurrentItem(3);
		    break;
		    
		  case 128000:
		    bitrate_box->setCurrentItem(4);
		    break;
		    
		  case 160000:
		    bitrate_box->setCurrentItem(5);
		    break;
		    
		  case 192000:
		    bitrate_box->setCurrentItem(6);
		    break;
		    
		  case 224000:
		    bitrate_box->setCurrentItem(7);
		    break;
		    
		  case 256000:
		    bitrate_box->setCurrentItem(8);
		    break;
		    
		  case 320000:
		    bitrate_box->setCurrentItem(9);
		    break;
		}
		break;

	      case 2:
		bitrate_box->setEnabled(true);
		bitrate_box->insertItem("128 kbps");
		bitrate_box->insertItem("160 kbps");
		bitrate_box->insertItem("192 kbps");
		bitrate_box->insertItem("224 kbps");
		bitrate_box->insertItem("256 kbps");
		bitrate_box->insertItem("320 kbps");
		switch(lib_lib->bitRate(dir)) {
		  case 128000:
		    bitrate_box->setCurrentItem(0);
		    break;
		    
		  case 160000:
		    bitrate_box->setCurrentItem(1);
		    break;
		    
		  case 192000:
		    bitrate_box->setCurrentItem(2);
		    break;
		    
		  case 224000:
		    bitrate_box->setCurrentItem(3);
		    break;
		    
		  case 256000:
		    bitrate_box->setCurrentItem(4);
		    break;
		    
		  case 320000:
		    bitrate_box->setCurrentItem(5);
		    break;
		}
		break;
	    }
	    break;
	}
	break;
  }
  if(samprate_box!=NULL) {
    for(int i=0;i<samprate_box->count();i++) {
      if(samprate==samprate_box->text(i).toInt()) {
	samprate_box->setCurrentItem(i);
      }
    }
  }
  for(int i=0;i<channels_box->count();i++) {
    if(channels==channels_box->text(i).toInt()) {
      channels_box->setCurrentItem(i);
    }
  }
  if((ReadFormat(AudioSettings::Transmit)==AudioSettings::Speex)||
     (ReadFormat(AudioSettings::Receive)==AudioSettings::Speex)) {
    if(lib_trans_samprate_box->count()==2) {
      lib_trans_samprate_box->removeItem(1);
    }
  }
  else {
    if(lib_trans_samprate_box->count()==1) {
      lib_trans_samprate_box->insertItem("48000");
    }
  }
}


AudioSettings::Format AudioSettingsDialog::ReadFormat(AudioSettings::
						      Direction dir)
{
  switch(dir) {
    case AudioSettings::Transmit:
      if(lib_trans_format_box->currentText()==tr("Vorbis")) {
	return AudioSettings::Vorbis;
      }
      if(lib_trans_format_box->currentText()==tr("MPEG Layer 3")) {
	return AudioSettings::Layer3;
      }
      if(lib_trans_format_box->currentText()==tr("Speex")) {
	return AudioSettings::Speex;
      }
      break;

    case AudioSettings::Receive:
      if(lib_recv_format_box->currentText()==tr("Use Transmit Settings")) {
	return AudioSettings::NoFormat;
      }
      if(lib_recv_format_box->currentText()==tr("Vorbis")) {
	return AudioSettings::Vorbis;
      }
      if(lib_recv_format_box->currentText()==tr("MPEG Layer 3")) {
	return AudioSettings::Layer3;
      }
      if(lib_recv_format_box->currentText()==tr("Speex")) {
	return AudioSettings::Speex;
      }
      break;
  }
  return AudioSettings::Speex;
}
