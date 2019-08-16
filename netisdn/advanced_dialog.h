// advanced_dialog.h
//
// Edit Audio Settings
//
//   (C) Copyright 2002-2008 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: advanced_dialog.h,v 1.5 2008/11/11 17:41:02 cvs Exp $
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

#ifndef ADVANCED_DIALOG_H
#define ADVANCED_DIALOG_H

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


class AdvancedDialog : public QDialog
{
  Q_OBJECT
 public:
  AdvancedDialog(AudioSettings *settings,Codec *codec,
		 QWidget *parent=0,const char *name=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 private slots:
  void rmlPortChangedData(int v);
  void okData();
  void cancelData();

 private:
  AudioSettings *lib_lib;
  Codec *lib_codec;
  void ShowBitRates(AudioSettings::Format fmt,int rate);
  QComboBox *lib_loopback_box;
  QSpinBox *lib_rml_port_spin;
  QLineEdit *lib_gpiocmd_edit[32][2];
  QLineEdit *lib_gpioaddr_edit[32][2];
  QSpinBox *lib_gpioport_spin[32][2];
  QSpinBox *lib_input_gain_spin;
  QSpinBox *lib_bufferpackets_spin;
  QComboBox *lib_realtime_box;
};


#endif  // ADVANCED_DIALOG_H
