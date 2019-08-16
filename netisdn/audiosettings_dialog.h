// audiosettings_dialog.h
//
// Edit Audio Settings
//
//   (C) Copyright 2002-2008 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: audiosettings_dialog.h,v 1.14 2008/10/14 13:12:08 cvs Exp $
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

#ifndef AUDIOSETTINGS_DIALOG_H
#define AUDIOSETTINGS_DIALOG_H

#include <vector>

#include <qdialog.h>
#include <qsqldatabase.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qspinbox.h>

#include <portaudio.h>

#include <audiosettings.h>
#include <codec.h>


class AudioSettingsDialog : public QDialog
{
  Q_OBJECT
 public:
  AudioSettingsDialog(AudioSettings *settings,Codec *codec,
		      PaDeviceIndex *in_pdev,
		      PaDeviceIndex *out_pdev,bool mpeg=true,
		      QWidget *parent=0,const char *name=0);
  ~AudioSettingsDialog();
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void settingsChangedData(int n);
  void passwordData();
  void advancedData();
  void okData();
  void cancelData();

 private:
  void CheckTransmitSettings(AudioSettings::Format fmt,int rate);
  void CheckReceiveSettings(AudioSettings::Format fmt,int rate);
  void CheckSettings(AudioSettings::Direction dir,
		     AudioSettings::Format fmt,int rate,
		     QComboBox *samprate_box,QComboBox *channels_box,
		     QComboBox *bitrate_box,
		     QLabel *bitrate_label,int samprate);
  AudioSettings::Format ReadFormat(AudioSettings::Direction dir);
  PaDeviceIndex *lib_input_index;
  PaDeviceIndex *lib_output_index;
  AudioSettings *lib_lib;
  Codec *lib_codec;
  QComboBox *lib_input_box;
  std::vector<PaDeviceIndex> lib_input_pdevs;
  QLabel *lib_trans_preprocess_label;
  QCheckBox *lib_trans_preprocess_box;
  QLabel *lib_recv_preprocess_label;
  QCheckBox *lib_recv_preprocess_box;
  QComboBox *lib_output_box;
  std::vector<PaDeviceIndex> lib_output_pdevs;
  QComboBox *lib_trans_format_box;
  QComboBox *lib_trans_channels_box;
  QLabel *lib_trans_bitrate_label;
  QComboBox *lib_trans_bitrate_box;
  QComboBox *lib_trans_hyperstream_box;
  QComboBox *lib_trans_samprate_box;
  QComboBox *lib_recv_format_box;
  QLabel *lib_recv_channels_label;
  QComboBox *lib_recv_channels_box;
  QLabel *lib_recv_bitrate_label;
  QComboBox *lib_recv_bitrate_box;
  QLabel *lib_recv_hyperstream_label;
  QComboBox *lib_recv_hyperstream_box;
  QLineEdit *lib_username_edit;
  QString lib_password;
  QLineEdit *lib_name_edit;
  QLineEdit *lib_phone_edit;
  QLineEdit *lib_email_edit;
};


#endif  // AUDIOSETTINGS_DIALOG_H
