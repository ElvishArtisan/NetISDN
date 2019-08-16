// codecstats.cpp
//
// A container class for codec metadata and statistics.
//
//   (C) Copyright 2008-2019 Fred Gleason <fredg@paravelsystems.com>
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

#include <stdint.h>

#include <codec_stats.h>

CodecStats::CodecStats()
{
  clearAll();
}


QString CodecStats::transmitCname() const
{
  return codec_transmit_cname;
}


void CodecStats::setTransmitCname(const QString &str)
{
  codec_transmit_cname=str;
}


QString CodecStats::receptionCname() const
{
  return codec_reception_cname;
}


void CodecStats::setReceptionCname(const QString &str)
{
  codec_reception_cname=str;
}


unsigned CodecStats::transmitSsrc() const
{
  return codec_transmit_ssrc;
}


void CodecStats::setTransmitSsrc(unsigned ssrc)
{
  codec_transmit_ssrc=ssrc;
}


unsigned CodecStats::receptionSsrc() const
{
  return codec_reception_ssrc;
}


void CodecStats::setReceptionSsrc(unsigned ssrc)
{
  codec_reception_ssrc=ssrc;
}


QString CodecStats::name() const
{
  return codec_name;
}


void CodecStats::setName(const QString &str)
{
  codec_name=str;
}


QString CodecStats::email() const
{
  return codec_email;
}


void CodecStats::setEmail(const QString &str)
{
  codec_email=str;
}


QString CodecStats::phone() const
{
  return codec_phone;
}


void CodecStats::setPhone(const QString &str)
{
  codec_phone=str;
}


QString CodecStats::location() const
{
  return codec_location;
}


void CodecStats::setLocation(const QString &str)
{
  codec_location=str;
}


QString CodecStats::tool() const
{
  return codec_tool;
}


void CodecStats::setTool(const QString &str)
{
  codec_tool=str;
}


QString CodecStats::note() const
{
  return codec_note;
}


void CodecStats::setNote(const QString &str)
{
  codec_note=str;
}


unsigned CodecStats::transmitPacketsSent() const
{
  return codec_transmit_packets_sent;
}


void CodecStats::setTransmitPacketsSent(unsigned packets)
{
  codec_transmit_packets_sent=packets;
}


unsigned CodecStats::transmitPacketsLost() const
{
  return codec_transmit_packets_lost;
}


void CodecStats::setTransmitPacketsLost(unsigned packets)
{
  codec_transmit_packets_lost=packets;
}


double CodecStats::transmitFractionLost() const
{
  return codec_transmit_fraction_lost;
}


void CodecStats::setTransmitFractionLost(double ratio)
{
  codec_transmit_fraction_lost=ratio;
}


unsigned CodecStats::receptionPacketsSent() const
{
  return codec_reception_packets_sent;
}


void CodecStats::setReceptionPacketsSent(unsigned packets)
{
  codec_reception_packets_sent=packets;
}


unsigned CodecStats::receptionPacketsReceived() const
{
  return codec_reception_packets_received;
}


void CodecStats::setReceptionPacketsReceived(unsigned packets)
{
  codec_reception_packets_received=packets;
}


unsigned CodecStats::receptionPacketsLost() const
{
  return codec_reception_packets_lost;
}


void CodecStats::setReceptionPacketsLost(unsigned packets)
{
  codec_reception_packets_lost=packets;
}


double CodecStats::receptionPllOffset() const
{
  return codec_reception_pll_offset;
}


void CodecStats::setReceptionPllOffset(double offset)
{
  codec_reception_pll_offset=offset;
}


double CodecStats::transmitCpuLoad() const
{
  return codec_transmit_cpu_load;
}


void CodecStats::setTransmitCpuLoad(double load)
{
  codec_transmit_cpu_load=load;
}


uint32_t CodecStats::transmitGpioMask() const
{
  return codec_transmit_gpio_mask;
}


void CodecStats::setTransmitGpioMask(uint32_t mask)
{
  codec_transmit_gpio_mask=mask;
}


uint32_t CodecStats::receptionGpioMask() const
{
  return codec_reception_gpio_mask;
}


void CodecStats::setReceptionGpioMask(uint32_t mask)
{
  codec_reception_gpio_mask=mask;
}


void CodecStats::clearTransmit()
{
  codec_transmit_cname="";
  codec_transmit_ssrc=0;
  codec_transmit_packets_sent=0;
  codec_transmit_packets_lost=0;
  codec_transmit_fraction_lost=0.0;
  codec_transmit_cpu_load=0.0;
  codec_transmit_gpio_mask=0;
}


void CodecStats::clearReception()
{
  codec_name="";
  codec_email="";
  codec_phone="";
  codec_location="";
  codec_tool="";
  codec_note="";
  codec_reception_cname="";
  codec_reception_ssrc=0;
  codec_reception_packets_sent=0;
  codec_reception_packets_received=0;
  codec_reception_packets_lost=0;
  codec_reception_pll_offset=0.0;
  codec_reception_gpio_mask=0;
}


void CodecStats::clearAll()
{
  clearTransmit();
  clearReception();
}
