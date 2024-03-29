// profileline.cpp
//
// A container class for profile lines.
//
// (C) Copyright 2002-2003 Fred Gleason <fredg@paravelsystems.com>
//
//    $Id: profileline.cpp,v 1.1 2008/06/09 13:59:14 cvs Exp $
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
//


#include <profileline.h>

ProfileLine::ProfileLine()
{
  clear();
}


QString ProfileLine::tag() const
{
  return line_tag;
}


void ProfileLine::setTag(QString tag)
{
  line_tag=tag;
}


QString ProfileLine::value() const
{
  return line_value;
}


void ProfileLine::setValue(QString value)
{
  line_value=value;
}


void ProfileLine::clear()
{
  line_tag="";
  line_value="";
}
