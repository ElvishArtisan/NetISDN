// meteraverage.h
//
// Average sucessive levels for a meter.
//
//   (C) Copyright 2007 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: meteraverage.h,v 1.2 2008/07/23 19:31:16 cvs Exp $
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
#ifndef METERAVERAGE_H
#define METERAVERAGE_H

#include <queue>


class MeterAverage
{
 public:
  MeterAverage(int maxsize);
  double average() const;
  void addValue(double value);
  void preset(double value);

 private:
  int avg_maxsize;
  double avg_total;
  std::queue<double> avg_values;
};


#endif  // METERAVERAGE_H
