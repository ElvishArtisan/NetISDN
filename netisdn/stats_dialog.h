// stats_dialog.h
//
// A statistics display dialog for NetISDN.
//
//   (C) Copyright 2008-2019 Fred Gleason <fredg@paravelsystems.com>
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

#ifndef STATS_DIALOG_H
#define STATS_DIALOG_H

#include <stdint.h>

#include <qdialog.h>
#include <qlineedit.h>
#include <qlabel.h>

#include <codec_stats.h>
#include <controlconnect.h>

#define STATSDIALOG_UPDATE_INTERVAL 500

class StatsDialog : public QDialog
{
  Q_OBJECT
 public:
  StatsDialog(CodecStats *stats,QWidget *parent=0,const char *name=0);
  QSize sizeHint() const;
  QSizePolicy sizePolicy() const;

 public slots:
  void updateMetadata(ControlConnect::MetadataField mf,const QString &str);

 private slots:
  void updateData();

 protected:
  void paintEvent(QPaintEvent *e);

 private:
  QString GetSsrcString(unsigned ssrc);
  CodecStats *stats_stats;
  QLineEdit *stats_name_edit;
  QLineEdit *stats_email_edit;
  QLineEdit *stats_phone_edit;
  QLineEdit *stats_location_edit;
  QLineEdit *stats_txcname_edit;
  QLineEdit *stats_txssrc_edit;
  QLineEdit *stats_txsent_edit;
  QLineEdit *stats_txlost_edit;
  QLineEdit *stats_txpercent_edit;
  QLineEdit *stats_txcpuload_edit;
  QLineEdit *stats_rxcname_edit;
  QLineEdit *stats_rxssrc_edit;
  QLineEdit *stats_rxsent_edit;
  QLineEdit *stats_rxrecp_edit;
  QLineEdit *stats_rxlost_edit;
  QLineEdit *stats_rxpercent_edit;
  QLineEdit *stats_rxplloffset_edit;
  QLabel *stats_gpio_trans_label[16];
  QLabel *stats_gpio_recv_label[16];
  QPalette stats_gpio_palette;
};


#endif  // STATS_DIALOG_H
