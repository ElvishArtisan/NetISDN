// audiosettings.h
//
// A container class for audio settings.
//
//   (C) Copyright 2002-2003,2008 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: audiosettings.h,v 1.11 2009/01/29 15:31:08 pcvs Exp $
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

#ifndef AUDIOSETTINGS_H
#define AUDIOSETTINGS_H

#include <qstring.h>
#include <qhostaddress.h>

class AudioSettings
{
 public:
  enum Format {NoFormat=0,Layer3=3,Vorbis=4,Speex=5};
  enum Direction {Transmit=0,Receive=1};
  AudioSettings();
  AudioSettings::Format format(AudioSettings::Direction dir,
			       bool trans=true) const;
  void setFormat(AudioSettings::Direction dir,AudioSettings::Format format);
  unsigned channels(AudioSettings::Direction dir,bool trans=true) const;
  void setChannels(AudioSettings::Direction dir,unsigned channels);
  unsigned sampleRate() const;
  void setSampleRate(unsigned rate);
  unsigned bitRate(AudioSettings::Direction dir,bool trans=true) const;
  void setBitRate(AudioSettings::Direction dir,unsigned rate);
  bool enableProcessor(AudioSettings::Direction dir,bool trans=true) const;
  void setEnableProcessor(AudioSettings::Direction dir,bool state);
  unsigned streamRatio(AudioSettings::Direction dir,bool trans=true) const;
  void setStreamRatio(AudioSettings::Direction dir,unsigned ratio);
  QString username() const;
  void setUsername(const QString &str);
  QString password() const;
  void setPassword(const QString &passwd);
  int rmlPort() const;
  void setRmlPort(int port);
  int inputGain() const;
  void setInputGain(int gain);
  bool useRealtime() const;
  void setUseRealtime(bool state);
  QString gpioCommand(unsigned chan,unsigned state) const;
  void setGpioCommand(unsigned chan,unsigned state,const QString &str);
  QHostAddress gpioAddress(unsigned chan,unsigned state) const;
  void setGpioAddress(unsigned chan,unsigned state,const QHostAddress &addr);
  Q_UINT16 gpioPort(unsigned chan,unsigned state) const;
  void setGpioPort(unsigned chan,unsigned state,Q_UINT16 port);
  void clear();
  QString dump(const QString &title) const;
  static QString formatString(AudioSettings::Format fmt);

 private:
  AudioSettings::Direction AutoVector(AudioSettings::Direction dir,
				      bool trans) const;
  AudioSettings::Format set_format[2];
  unsigned set_channels[2];
  unsigned set_sample_rate;
  unsigned set_bit_rate[2];
  bool set_enable_processor[2];
  unsigned set_stream_ratio[2];
  QString set_username;
  QString set_password;
  int set_rml_port;
  int set_input_gain;
  bool set_use_realtime;
  QString set_gpio_command[32][2];
  QHostAddress set_gpio_address[32][2];
  Q_UINT16 set_gpio_port[32][2];
};


#endif  // AUDIOSETTINGS_H
