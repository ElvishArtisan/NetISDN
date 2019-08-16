// codecstats.h
//
// A container class for codec metadata and statistics.
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: codec_stats.h,v 1.6 2008/10/14 14:06:17 cvs Exp $
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

#ifndef CODECSTATS_H
#define CODECSTATS_H

#ifdef WIN32
#include <win32_types.h>
#endif  // WIN32

#include <qstring.h>

class CodecStats
{
  public:
  CodecStats();
  QString transmitCname() const;
  void setTransmitCname(const QString &str);
  QString receptionCname() const;
  void setReceptionCname(const QString &str);
  unsigned transmitSsrc() const;
  void setTransmitSsrc(unsigned ssrc);
  unsigned receptionSsrc() const;
  void setReceptionSsrc(unsigned ssrc);
  QString name() const;
  void setName(const QString &str);
  QString email() const;
  void setEmail(const QString &str);
  QString phone() const;
  void setPhone(const QString &str);
  QString location() const;
  void setLocation(const QString &str);
  QString tool() const;
  void setTool(const QString &str);
  QString note() const;
  void setNote(const QString &str);
  unsigned transmitPacketsSent() const;
  void setTransmitPacketsSent(unsigned packets);
  unsigned transmitPacketsLost() const;
  void setTransmitPacketsLost(unsigned packets);
  double transmitFractionLost() const;
  void setTransmitFractionLost(double ratio);
  unsigned receptionPacketsSent() const;
  void setReceptionPacketsSent(unsigned packets);
  unsigned receptionPacketsReceived() const;
  void setReceptionPacketsReceived(unsigned packets);
  unsigned receptionPacketsLost() const;
  void setReceptionPacketsLost(unsigned packets);
  double receptionPllOffset() const;
  void setReceptionPllOffset(double offset);
  double transmitCpuLoad() const;
  void setTransmitCpuLoad(double load);
  uint32_t transmitGpioMask() const;
  void setTransmitGpioMask(uint32_t mask);
  uint32_t receptionGpioMask() const;
  void setReceptionGpioMask(uint32_t mask);
  void clearTransmit();
  void clearReception();
  void clearAll();

 private:
  QString codec_transmit_cname;
  QString codec_reception_cname;
  unsigned codec_transmit_ssrc;
  unsigned codec_reception_ssrc;
  QString codec_name;
  QString codec_email;
  QString codec_phone;
  QString codec_location;
  QString codec_tool;
  QString codec_note;
  unsigned codec_transmit_packets_sent;
  unsigned codec_transmit_packets_lost;
  double codec_transmit_fraction_lost;
  unsigned codec_reception_packets_sent;
  unsigned codec_reception_packets_received;
  unsigned codec_reception_packets_lost;
  double codec_reception_pll_offset;
  double codec_transmit_cpu_load;
  uint32_t codec_transmit_gpio_mask;
  uint32_t codec_reception_gpio_mask;
};


#endif  // CODECSTATS_H
