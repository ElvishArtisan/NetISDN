// meteraverage.cpp
//
// Average sucessive levels for a meter.
//
//   (C) Copyright 2007 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: meteraverage.cpp,v 1.3 2008/08/05 16:21:02 cvs Exp $
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

#include <meteraverage.h>


MeterAverage::MeterAverage(int maxsize)
{
  avg_maxsize=maxsize;
  avg_total=0.0;
}


double MeterAverage::average() const
{
  if(avg_values.size()==0) {
    return 0.0;
  }
  return avg_total/((double)avg_values.size());
}


void MeterAverage::addValue(double value)
{
  avg_total+=value;
  avg_values.push(value);
  int size=avg_values.size()-avg_maxsize;
  for(int i=0;i<size;i++) {
    avg_total-=avg_values.front();
    avg_values.pop();
  }
}


void MeterAverage::preset(double value)
{
  while(avg_values.size()>0) {
    avg_values.pop();
  }
  for(int i=0;i<avg_maxsize;i++) {
    avg_values.push(value);
  }
  avg_total=value*avg_maxsize;
}
