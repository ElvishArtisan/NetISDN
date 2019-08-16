// escape_string.h
//
// Escape non-valid characters in a string.
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//      $Id: escape_string.h,v 1.1 2008/08/09 01:58:06 cvs Exp $
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

#include <qstring.h>

#ifndef ESCAPE_STRING_H
#define ESCAPE_STRING_H

QString EscapeString(const QString &str);

/**
 * Escape a string so it is safe to be used as a SQL (MySQL) column.
 * For details see http://dev.mysql.com/doc/refman/4.1/en/legal-names.html
 * 
 * @param str The string to escape.
 *
 * Returns escaped string
 **/
QString EscapeStringSQLColumn(const QString &str);



#endif 
