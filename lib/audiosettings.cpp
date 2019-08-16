// audiosettings.cpp
//
// Common Audio Settings
//
//   (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: audiosettings.cpp,v 1.10 2009/01/29 15:31:08 pcvs Exp $
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

#include <qstring.h>

#include <audiosettings.h>


AudioSettings::AudioSettings()
{
  clear();
}


AudioSettings::Format AudioSettings::format(AudioSettings::Direction dir,
					    bool trans) const
{
  return set_format[AutoVector(dir,trans)];
}


void AudioSettings::setFormat(AudioSettings::Direction dir,
			      AudioSettings::Format format)
{
  set_format[dir]=format;
}


unsigned AudioSettings::channels(AudioSettings::Direction dir,bool trans) const
{
  return set_channels[AutoVector(dir,trans)];
}


void AudioSettings::setChannels(AudioSettings::Direction dir,unsigned channels)
{
  set_channels[dir]=channels;
}


unsigned AudioSettings::sampleRate() const
{
  return set_sample_rate;
}


void AudioSettings::setSampleRate(unsigned rate)
{
  set_sample_rate=rate;
}


unsigned AudioSettings::bitRate(AudioSettings::Direction dir,bool trans) const
{
  return set_bit_rate[AutoVector(dir,trans)];
}


void AudioSettings::setBitRate(AudioSettings::Direction dir,unsigned rate)
{
  set_bit_rate[dir]=rate;
}


bool AudioSettings::enableProcessor(AudioSettings::Direction dir,
				    bool trans) const
{
  return set_enable_processor[AutoVector(dir,trans)];
}


void AudioSettings::setEnableProcessor(AudioSettings::Direction dir,bool state)
{
  set_enable_processor[dir]=state;
}


unsigned AudioSettings::streamRatio(AudioSettings::Direction dir,
				    bool trans) const
{
  return set_stream_ratio[AutoVector(dir,trans)];
}


void AudioSettings::setStreamRatio(AudioSettings::Direction dir,unsigned ratio)
{
  set_stream_ratio[dir]=ratio;
}


QString AudioSettings::username() const
{
  return set_username;
}


void AudioSettings::setUsername(const QString &str)
{
  set_username=str;
}


QString AudioSettings::password() const
{
  return set_password;
}


void AudioSettings::setPassword(const QString &passwd)
{
  set_password=passwd;
}


int AudioSettings::rmlPort() const
{
  return set_rml_port;
}


void AudioSettings::setRmlPort(int port)
{
  set_rml_port=port;
}


int AudioSettings::inputGain() const
{
  return set_input_gain;
}


void AudioSettings::setInputGain(int gain)
{
  set_input_gain=gain;
}


bool AudioSettings::useRealtime() const
{
  return set_use_realtime;
}


void AudioSettings::setUseRealtime(bool state)
{
  set_use_realtime=state;
}


QString AudioSettings::gpioCommand(unsigned chan,unsigned state) const
{
  return set_gpio_command[chan][state];
}


void AudioSettings::setGpioCommand(unsigned chan,unsigned state,
				   const QString &str)
{
  set_gpio_command[chan][state]=str;
}


QHostAddress AudioSettings::gpioAddress(unsigned chan,unsigned state) const
{
  return set_gpio_address[chan][state];
}


void AudioSettings::setGpioAddress(unsigned chan,unsigned state,
				   const QHostAddress &addr)
{
  set_gpio_address[chan][state]=addr;
}


Q_UINT16 AudioSettings::gpioPort(unsigned chan,unsigned state) const
{
  return set_gpio_port[chan][state];
}


void AudioSettings::setGpioPort(unsigned chan,unsigned state,Q_UINT16 port)
{
  set_gpio_port[chan][state]=port;
}


void AudioSettings::clear()
{
  for(unsigned i=0;i<2;i++) {
    set_format[i]=AudioSettings::Speex;
    set_channels[i]=0;
    set_bit_rate[i]=0;
    set_enable_processor[i]=false;
    set_stream_ratio[i]=1;
  }
  set_sample_rate=0;
  set_username="";
  set_password="";
  set_rml_port=5859;
  set_input_gain=0;
  set_use_realtime=true;
  for(int i=0;i<32;i++) {
    for(int j=0;j<2;j++) {
      set_gpio_command[i][j]="";
      set_gpio_address[i][j]=QHostAddress();
      set_gpio_port[i][j]=5859;
    }
  }
}


QString AudioSettings::dump(const QString &title) const
{
  QString str;
  str+=QString().sprintf("*** AudioSettings -- %s ***\n",(const char *)title);
  str+=QString().sprintf(" Transmit Settings\n");
  str+=QString().sprintf("   Format: %u\n",format(AudioSettings::Transmit));
  str+=QString().sprintf("   Channels: %u\n",channels(AudioSettings::Transmit));
  str+=QString().sprintf("   Bit Rate: %u\n",bitRate(AudioSettings::Transmit));
  str+=QString().sprintf("   Voice Processor: %u\n",
			 enableProcessor(AudioSettings::Transmit));
  str+=QString().sprintf("   Stream Ratio: %u\n",
			 streamRatio(AudioSettings::Transmit));
  str+=QString().sprintf("   Input Gain: %d dB\n",inputGain()/100);
  if(useRealtime()) {
    str+=QString().sprintf(" Use Realtime: Yes\n");
  }
  else {
    str+=QString().sprintf(" Use Realtime: No\n");
  }
  str+=QString().sprintf(" Receive Settings\n");
  str+=QString().sprintf("   Format: %u\n",format(AudioSettings::Receive));
  str+=QString().sprintf("   Channels: %u\n",channels(AudioSettings::Receive));
  str+=QString().sprintf("   Bit Rate: %u\n",bitRate(AudioSettings::Receive));
  str+=QString().sprintf("   Voice Processor: %u\n",
			 enableProcessor(AudioSettings::Receive));
  str+=QString().sprintf("   Stream Ratio: %u\n",
			 streamRatio(AudioSettings::Receive));
  str+=QString().sprintf(" Global Settings\n");
  str+=QString().sprintf("   Sample Rate: %u\n",sampleRate());
  str+=QString().sprintf("*************************************************\n");
  return str;
}


QString AudioSettings::formatString(AudioSettings::Format fmt)
{
  QString ret="Unknown";
  switch(fmt) {
    case AudioSettings::Layer3:
      ret="MPEG Layer 3";
      break;
      
    case AudioSettings::Speex:
      ret="Speex";
      break;
      
    case AudioSettings::Vorbis:
      ret="Vorbis";
      break;
      
    case AudioSettings::NoFormat:
      ret="No Format";
      break;
  }
  return ret;
}


AudioSettings::Direction AudioSettings::AutoVector(AudioSettings::
						   Direction dir,
						   bool trans) const
{
  if((trans)&&(set_format[AudioSettings::Receive]==AudioSettings::NoFormat)) {
    return AudioSettings::Transmit;
  }
  return dir;
}
