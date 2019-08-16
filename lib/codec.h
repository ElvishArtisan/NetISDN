// codec.h
//
// (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
// A realtime MPEG Layer 3 Codec.
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


#ifndef CODEC_H
#define CODEC_H

#include <limits.h>

#include <qobject.h>
#include <qsocketdevice.h>
#include <qhostaddress.h>
#include <qtimer.h>
#include <qdatetime.h>

#ifdef WIN32
#include <win32_types.h>
#ifdef HAVE_MPEG_L3
#include <lame.h>
#endif  // HAVE_MPEG_L3
#else
#ifdef HAVE_MPEG_L3
#include <lame/lame.h>
#endif  // HAVE_MPEG_L3
#endif  // WIN32
#ifdef HAVE_MPEG_L3
#include <mad.h>
#endif  // HAVE_MPEG_L3
#ifdef HAVE_SPEEX
#include <speex/speex.h>
#include <speex/speex_preprocess.h>
#endif  // HAVE_SPEEX
#ifdef HAVE_VORBIS
#include <vorbis/vorbisenc.h>
#endif  // HAVE_VORBIS
#include <portaudio.h>
#include <samplerate.h>

#include <oneshot.h>
#include <rtpheader.h>
#include <rtpsourcedescription.h>
#include <codec_stats.h>
#include <meteraverage.h>
#include <audiosettings.h>
#include <ringbuffer.h>

#define CODEC_DEFAULT_TIMEOUT 5000
#define CODEC_DEFAULT_SPEEX_QUALITY 3
#define CODEC_MAX_PACKET_SIZE 1500
#define CODEC_RING_SIZE 1048576
#define CODEC_DEBOUNCE_INTERVAL 2000

//
// The preferred number of packets in the ringbuffer 
//
#define CODEC_VORBIS_SAMPLE_BLOCK_SIZE 256

//
// Phase Locked Loop Parameters
//
#define CODEC_DEFAULT_BUFFER_SETPOINT 8
#define CODEC_PLL_INTEGRATOR_STEP 400.0
#define CODEC_PLL_INTEGRATOR_PERIODS 500
#define CODEC_SRC_PCM_BUFFER_SIZE 4096

#define METER_UPDATE_INTERVAL 50
#define METER_AVERAGING_PERIODS 8

class Codec : public QObject
{
  Q_OBJECT;
 public:
  enum ConnectionState {StateIdle=0,StateConnecting=2,StateConnected=3};
  enum SdesField {SdesName=0,SdesEmail=1,SdesPhone=2,SdesLocation=3,SdesTool=4,
		  SdesLastValue=5};
  Codec(unsigned id,QObject *parent=0,const char *name=0);
  ~Codec();
  QString rtpSdesField(Codec::SdesField field) const;
  void setRtpSdesField(Codec::SdesField field,const QString &str);
  PaDeviceIndex inputDevice() const;
  void setInputDevice(PaDeviceIndex dev);
  PaDeviceIndex outputDevice() const;
  void setOutputDevice(PaDeviceIndex dev);
  unsigned receiveBuffers() const;
  void setReceiveBuffers(unsigned buffers);
  QString transmitCname() const;
  void setTransmitCname(const QString &name);
  unsigned timeout() const;
  void setTimeout(unsigned msecs);
  Codec::ConnectionState connectionState() const;
  bool isLocked() const;
  bool isLoopedBack() const;
  QHostAddress peerAddress() const;
  CodecStats *codecStats() const;
  void connectToHost(Q_UINT16 src_port,const QHostAddress &dest_addr,
		     Q_UINT16 dest_port,AudioSettings *s);

 public slots:
  void disconnect();
  void sendGpio(unsigned line,bool state,unsigned interval);
  void setLoopback(bool state);
  void setCodebook(const QString &start_pkt,const QString &comment_pkt,
		   const QString &codebook_pkt);

 signals:
  void connectionStateChanged(unsigned id,Codec::ConnectionState state);
  void codebookChanged(const QString &start_pkg,const QString &comment_pkt,
		       const QString &cookbook_pkt);  // Base64 encoded
  void locked(unsigned id,bool state);
  void loopedBack(unsigned id,bool state);
  void error(unsigned id,int err);
  void leftInputChanged(unsigned id,int level);
  void rightInputChanged(unsigned id,int level);
  void leftOutputChanged(unsigned id,int level);
  void rightOutputChanged(unsigned id,int level);
  void gpioChanged(unsigned id,unsigned line,bool state);

 private slots:
  void meterData();
  void rtcpTransmitData();
  void rtcpReceiveData();
  void timeoutData();
  void transOneshotData(void *data);

 private:
  void InitEncoder(unsigned chans,unsigned samprate,unsigned bitrate,
		   bool preprocess);
  void FreeEncoder();
  void InitDecoder(unsigned samprate);
  void FreeDecoder();
  int EncodeFrame(int16_t *inbuf,unsigned frames,unsigned char *outbuf);
  int DecodeFrame(unsigned char *inbuf,unsigned bytes,uint32_t seqno,
		  int16_t *outbuf,unsigned maxframes);

  //
  // Layer3
  //
  void InitLayer3Encoder(unsigned chans,unsigned samprate,unsigned bitrate);
  void FreeLayer3Encoder();
  void InitLayer3Decoder();
  void FreeLayer3Decoder();
  int EncodeLayer3Frame(const int16_t *inbuf,unsigned frames,
			unsigned char *outbuf);
  int DecodeLayer3Frame(unsigned char *inbuf,unsigned bytes,int16_t *outbuf);
  int WriteLayer3Buffer(unsigned char *inbuf,size_t len,
			unsigned char *outbuf,size_t *outlen);
#ifdef HAVE_MPEG_L3
  lame_global_flags *codec_lame_gfp;
  unsigned codec_lame_offset;
  int16_t *codec_last_sample_block;
  struct mad_stream codec_mad_stream;
  struct mad_frame codec_mad_frame;
  struct mad_synth codec_mad_synth;
#endif  // HAVE_MPEG_L3

  //
  // Speex
  //
  void InitSpeexEncoder(unsigned samprate,unsigned quality);
  void FreeSpeexEncoder();
  void InitSpeexDecoder(unsigned samprate);
  void FreeSpeexDecoder();
  int EncodeSpeexFrame(const int16_t *inbuf,unsigned frames,
			unsigned char *outbuf);
  int DecodeSpeexFrame(unsigned char *inbuf,unsigned bytes,int16_t *outbuf);
#ifdef HAVE_SPEEX
  SpeexBits codec_speex_enc_bits;
  void *codec_speex_enc_state;
  SpeexBits codec_speex_dec_bits;
  void *codec_speex_dec_state;
  SpeexPreprocessState *codec_preprocess_state;
#endif  // HAVE_SPEEX

  //
  // Vorbis
  //
  void InitVorbisEncoder(unsigned chans,unsigned samprate,unsigned bitrate);
  void FreeVorbisEncoder();
  void InitVorbisDecoder(unsigned samprate);
  void FreeVorbisDecoder();
  int EncodeVorbisFrame(const int16_t *inbuf,unsigned frames,
			unsigned char *outbuf);
  int DecodeVorbisFrame(unsigned char *inbuf,unsigned bytes,uint32_t seqno,
			int16_t *outbuf,unsigned maxframes);
#ifdef VORBIS
  QString DumpOggPacket(const QString &title,ogg_packet *op);
  vorbis_info codec_vorbis_trans_info;
  vorbis_comment codec_vorbis_trans_comment;
  vorbis_dsp_state codec_vorbis_trans_dsp_state;
  vorbis_block codec_vorbis_trans_block;
  vorbis_info codec_vorbis_recv_info;
  vorbis_comment codec_vorbis_recv_comment;
  vorbis_block codec_vorbis_recv_block;
  vorbis_dsp_state codec_vorbis_recv_dsp_state;
  bool codec_vorbis_decoder_ready;
#endif  // VORBIS
  //
  // Common Components
  //
  void Silence(int16_t *buf,unsigned frames,unsigned chans);
  unsigned GenerateRtcpPacket(uint32_t *data,unsigned len);
  uint64_t GetNtpTimestamp(const QDateTime &dt);
  int SampleToDb(double samp);
  void SetConnectionState(Codec::ConnectionState state);
  bool InitSockets(Q_UINT16 port);
  void FreeSockets();
  bool InitAudioInterface();
  void FreeAudioInterface();
  void StartMeters();
  void StopMeters();
  bool SetDestinationAddress(const QHostAddress &addr,Q_UINT16 port);
  void ReadPrivateSourceChunk(uint8_t *data,unsigned len);
  unsigned codec_id;
  QString codec_sdes_fields[Codec::SdesLastValue];
  QString codec_hostname;
  Q_UINT16 codec_port;
  unsigned codec_sample_enc_size;
  unsigned codec_sample_dec_size;
  PaDeviceIndex codec_input_pdev;
  PaDeviceIndex codec_output_pdev;
  PaStream *codec_input_pstream;
  PaStream *codec_output_pstream;
  AudioSettings codec_settings;
//  AudioSettings::Format codec_format;
//  unsigned codec_channels;
//  unsigned codec_sample_rate;
//  unsigned codec_bit_rate;
//  bool codec_preprocess;
  bool codec_listen_preprocess;
  unsigned codec_timeout;
  Codec::ConnectionState codec_connection_state;
  bool codec_locked;
  bool codec_lock_state;
  int codec_frame_size;
  SRC_STATE *codec_src_state;
  MeterAverage *codec_pll_average;
  double codec_pll_offset;
  QSocketDevice *codec_rtp_socket;
  QSocketDevice *codec_rtcp_socket;
  QHostAddress *codec_dest_addr;
  RTPHeader *codec_transmit_header;
  int16_t codec_input_meters[2];
  int16_t codec_output_meters[2];
  QTimer *codec_meter_timer;
  MeterAverage *codec_input_averages[2];
  MeterAverage *codec_output_averages[2];
  QTimer *codec_rtcp_transmit_timer;
  QTimer *codec_rtcp_receive_timer;
  QTimer *codec_timeout_timer;
  QString codec_transmit_cname;
  uint32_t codec_transmit_packets;
  uint32_t codec_transmit_octets;
  uint32_t codec_recv_ssrc;
  uint32_t codec_recv_total_packets_lost;
  uint32_t codec_recv_interval_packets_lost;
  uint32_t codec_recv_start_sequence_number;
  uint32_t codec_recv_start_packet_count;
  uint16_t codec_recv_last_sequence_number;
  uint16_t codec_recv_sequence_lo_number;
  uint16_t codec_recv_sequence_hi_number;
  uint64_t codec_recv_ntp_timestamp;
  uint32_t codec_trans_gpio;
  OneShot *codec_trans_gpio_oneshot;
  float codec_input_gain_ratio;
  uint32_t codec_recv_gpio;
  RingBuffer *codec_ring;
  CodecStats *codec_stats;
  bool codec_loopback;
  unsigned codec_buffer_setpoint;
  unsigned codec_stream_ratio;
  friend int WriteBuffer(Codec *codec,unsigned char *buf,size_t len);
  friend int AudioInputCallback(const void *input, void *output,
				unsigned long frameCount, 
				const PaStreamCallbackTimeInfo *timeInfo, 
				PaStreamCallbackFlags statusFlags, 
				void *userData);
  friend int AudioOutputCallback(const void *input, void *output,
				 unsigned long frameCount, 
				 const PaStreamCallbackTimeInfo *timeInfo, 
				 PaStreamCallbackFlags statusFlags,
				 void *userData);
  friend void Silence(Codec *codec,int16_t *buf,unsigned frames);
};


#endif  // CODEC_H
