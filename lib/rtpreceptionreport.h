// rtpreceptionreport.h
//
// (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
// A container class for RTCP reception reports
//
// For information on these data structures, see RFC 3550.
//
//  $id:$
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


#ifndef RTPRECEPTIONREPORT_H
#define RTPRECEPTIONREPORT_H

#include <sys/types.h>
#include <vector>
#ifdef WIN32
#include <win32_types.h>
#include <Winsock2.h>
#else
#include <stdint.h>
#endif

#include <rtpreceptionblock.h>

#define RTPRECEPTIONREPORT_BASE_SIZE 2
#define RTPRECEPTIONREPORT_MAX_BLOCKS 31
#define RTPRECEPTIONREPORT_PT 201

class RTPReceptionReport
{
 public:
  RTPReceptionReport(uint32_t ssrc,unsigned version=2);
  ~RTPReceptionReport();
  unsigned version() const;
  bool paddingBit() const;
  void setPaddingBit(bool state);
  uint32_t ssrc() const;
  void setSsrc(uint32_t ssrc);
  unsigned receptionBlocks() const;
  RTPReceptionBlock *receptionBlock(unsigned n);
  bool addReceptionBlock(RTPReceptionBlock *block);
  int readBlock(uint32_t *data);
  int writeBlock(uint32_t *data,unsigned len) const;
  void clear();

 private:
  unsigned rtp_version;
  bool rtp_padding_bit;
  uint32_t rtp_ssrc;
  std::vector<RTPReceptionBlock *> rtp_reception_blocks;
};


#endif  // RTPRECEPTIONREPORT_H
