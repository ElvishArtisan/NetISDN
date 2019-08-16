// serversocket.cpp
//
// Accept incoming socket connections for netsipd(8).
//
//   (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of
//   the License, or (at your option) any later version.
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


#include <qapplication.h>
#include <qobject.h>
#include <qserversocket.h>
#include <qhostaddress.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <math.h>

#include <serversocket.h>

ServerSocket::ServerSocket(Q_UINT16 port,int backlog,QObject *parent,
		     const char *name) 
  : QServerSocket(port,0,parent,name)
{
}


ServerSocket::ServerSocket(const QHostAddress &address,Q_UINT16 port,int backlog,
		     QObject *parent,const char *name) 
  : QServerSocket(address,port,0,parent,name)
{
}


void ServerSocket::newConnection(int fd)
{
  emit connection(fd);
}
