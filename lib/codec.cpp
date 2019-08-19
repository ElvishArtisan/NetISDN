// codec.cpp
//
// (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
//
// A realtime MPEG Layer 3 Codec.
//
//  $Id: codec.cpp,v 1.31 2008/11/14 16:35:38 cvs Exp $
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <math.h>
#ifdef WIN32
#include <io.h>
#include <Winsock2.h>
#else
#include <unistd.h>
#endif  // WIN32

#include <qmessagebox.h>
#include <qapplication.h>
#include <qdatetime.h>

#include <ogg/ogg.h>

#include <netconf.h>
#include <rtpcontrolheader.h>
#include <codec.h>

#ifdef HAVE_MPEG_L3
signed int Scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}
#endif  // HAVE_MPEG_L3


int AudioInputCallback(const void *input, void *output, 
		       unsigned long frameCount, 
		       const PaStreamCallbackTimeInfo *timeInfo, 
		       PaStreamCallbackFlags statusFlags, void *userData)
{
  int s;
  Codec *codec=(Codec *)userData;
  int16_t *inbuf=(int16_t *)input;
  unsigned char buf[CODEC_MAX_PACKET_SIZE];
  int16_t meters[2]={0,0};
  AudioSettings *settings=&codec->codec_settings;

  if((input!=NULL)&&(!codec->codec_loopback)) {
#ifdef HAVE_SPEEX
    if(codec->codec_preprocess_state!=NULL) {
      speex_preprocess_run(codec->codec_preprocess_state,inbuf);
    }
#endif  // HAVE_SPEEX
    if((s=codec->EncodeFrame((int16_t *)input,frameCount,buf))>0) {
      for(unsigned i=0;i<codec->codec_stream_ratio;i++) {
	codec->codec_rtp_socket->writeBlock((const char *)buf,s,
			       *codec->codec_dest_addr,codec->codec_port);
      }
    }
  }

  //
  // Process Meters
  //
  switch(settings->channels(AudioSettings::Transmit)) {
    case 1:
      for(unsigned i=0;i<frameCount;i++) {
	if(inbuf[i]>meters[0]) {
	  meters[0]=inbuf[i];
	}
      }
      codec->codec_input_meters[0]=meters[0];
      codec->codec_input_meters[1]=meters[0];
      break;
      
    case 2:
      for(unsigned i=0;i<(frameCount*2);i+=2) {
	if(inbuf[i]>meters[0]) {
	  meters[0]=inbuf[i];
	}
	if(inbuf[i+1]>meters[1]) {
	  meters[1]=inbuf[i+1];
	}
      }
      codec->codec_input_meters[0]=meters[0];
      codec->codec_input_meters[1]=meters[1];
      break;
  }
  return paContinue;
}


int AudioOutputCallback(const void *input, void *output, 
			unsigned long frameCount, 
			const PaStreamCallbackTimeInfo *timeInfo, 
			PaStreamCallbackFlags statusFlags, void *userData)
{
  int n;
  int16_t *outbuf=(int16_t *)output;
  Codec *codec=(Codec *)userData;
  unsigned char buf[CODEC_MAX_PACKET_SIZE];
  unsigned char *fbuf=NULL;
  unsigned flen=0;
  int16_t meters[2]={0,0};
  RTPHeader hdr(RTPHeader::PayloadMpa);
  int l;
  int frames=0;
  int16_t inter_buf[CODEC_SRC_PCM_BUFFER_SIZE];
  SRC_DATA src_data;
  float src_in[CODEC_SRC_PCM_BUFFER_SIZE];
  float src_out[CODEC_SRC_PCM_BUFFER_SIZE];
  AudioSettings *settings=&codec->codec_settings;

  if(codec->codec_loopback) {
    while((n=codec->codec_rtp_socket->
	   readBlock((char *)buf,CODEC_MAX_PACKET_SIZE))>0) {
      codec->codec_rtp_socket->
	writeBlock((char *)buf,n,*codec->codec_dest_addr,codec->codec_port);
      codec->Silence(outbuf,frameCount,
		     settings->channels(AudioSettings::Receive));
    }
    return paContinue;
  }

  if(output==NULL) {
    return paContinue;
  }

  //
  // Fill Ring Buffer
  // 
  while((n=codec->codec_rtp_socket->
	 readBlock((char *)buf,CODEC_MAX_PACKET_SIZE))>0) {
    if(n>RTPHEADER_MIN_SIZE) {
      
      //
      // Read RTP Header
      //
	l=4*hdr.readBlock((uint32_t *)buf,n/4);

      //
      // Verify packet order and generate receiver statistics
      //
      // FIXME: We could attempt to deal with out of order packets 
      //        a little more gracefully...
      //
      if(hdr.ssrc()==codec->codec_recv_ssrc) {
	if(hdr.sequenceNumber()!=(codec->codec_recv_sequence_lo_number+1)) {
	  if(hdr.sequenceNumber()!=(codec->codec_recv_sequence_lo_number)) {
	    codec->codec_recv_total_packets_lost++;
	    codec->codec_recv_interval_packets_lost++;
	  }
	  fbuf=NULL;
	  flen=0;
	}
	else {
	  fbuf=l+buf;
	  flen=n-l;
	}
	if(hdr.sequenceNumber()>codec->codec_recv_sequence_lo_number) {
	  codec->codec_recv_sequence_lo_number=hdr.sequenceNumber();
	}
	else {
	  if((codec->codec_recv_sequence_lo_number>0xff00)&&
	     (hdr.sequenceNumber()<
	      (codec->codec_recv_sequence_lo_number-0xff00))) {
	    // Sequence number rolled over
	    codec->codec_recv_sequence_lo_number=hdr.sequenceNumber();
	    codec->codec_recv_sequence_hi_number++;
	    codec->codec_recv_last_sequence_number=0;
	  }
	  else {
	    fbuf=NULL;
	    flen=0;
	  }
	}
      }
      else {  // SSRC changed!
	codec->codec_recv_ssrc=hdr.ssrc();
	codec->codec_recv_total_packets_lost=0;
	codec->codec_recv_interval_packets_lost=0;
	codec->codec_recv_sequence_lo_number=hdr.sequenceNumber();
	codec->codec_recv_last_sequence_number=hdr.sequenceNumber();
	codec->codec_recv_start_sequence_number=hdr.sequenceNumber();
	codec->codec_recv_ssrc=hdr.ssrc();
	codec->codec_recv_ntp_timestamp=0;
	codec->codec_recv_start_packet_count=0;
	fbuf=l+buf;
	flen=n-l;
      }
      
      //
      // Process Output
      //
      if(flen>0) {
	codec->codec_pll_offset=codec->codec_pll_average->average();
	frames=codec->
	  DecodeFrame(fbuf,flen,codec->codec_recv_last_sequence_number,
		      inter_buf,CODEC_SRC_PCM_BUFFER_SIZE/settings->
		      channels(AudioSettings::Receive));
	src_short_to_float_array(inter_buf,src_in,frames*
				 settings->channels(AudioSettings::Receive));
	memset(&src_data,0,sizeof(src_data));
	src_data.data_in=src_in;
	src_data.data_out=src_out;
	src_data.input_frames=frames;
	src_data.output_frames=
	  CODEC_SRC_PCM_BUFFER_SIZE/settings->channels(AudioSettings::Receive);
	src_data.src_ratio=(settings->sampleRate()+codec->codec_pll_offset)/
	  settings->sampleRate();
	src_set_ratio(codec->codec_src_state,src_data.src_ratio);
	src_process(codec->codec_src_state,&src_data);
	src_float_to_short_array(src_out,inter_buf,
				 src_data.output_frames_gen*
				 settings->channels(AudioSettings::Receive));
	codec->codec_ring->
	  write((char *)inter_buf,
		2*src_data.output_frames_gen*
		settings->channels(AudioSettings::Receive));
      }
    }
  }

  //
  // Prefill the ring buffer before unmuting
  //
//  printf("readSpace: %ld  setpt: %lu\n",codec->codec_ring->readSpace(),      
//	 (2*frameCount*codec->codec_channels*CODEC_BUFFER_SETPOINT));


  if((codec->codec_ring->readSpace()<
      (2*frameCount*settings->channels(AudioSettings::Receive)*
       codec->codec_buffer_setpoint))&&
     (!codec->codec_locked)) {
    memset(outbuf,0,settings->channels(AudioSettings::Receive)*
	   frameCount*sizeof(int16_t));
    return paContinue;
  }

  //
  // Phase Detector
  //
  if(codec->codec_ring->readSpace()<
     (2*frameCount*settings->channels(AudioSettings::Receive)*
      codec->codec_buffer_setpoint)) {
    codec->codec_pll_average->addValue(CODEC_PLL_INTEGRATOR_STEP);
  }
  else {
    codec->codec_pll_average->addValue(-CODEC_PLL_INTEGRATOR_STEP);
  }

  //
  // Process Output
  //
  if(codec->codec_ring->readSpace()>=
     (2*frameCount*settings->channels(AudioSettings::Receive))) {
    codec->codec_ring->read((char *)outbuf,
		   2*frameCount*settings->channels(AudioSettings::Receive));

    //
    // Process Meters
    //
    switch(settings->channels(AudioSettings::Receive)) {
      case 1:
	for(unsigned i=0;i<frameCount;i++) {
	  if(outbuf[i]>meters[0]) {
	    meters[0]=outbuf[i];
	  }
	}
	codec->codec_output_meters[0]=meters[0];
	codec->codec_output_meters[1]=meters[0];
	break;
	
      case 2:
	for(unsigned i=0;i<(frameCount*2);i+=2) {
	  if(outbuf[i]>meters[0]) {
	    meters[0]=outbuf[i];
	  }
	  if(outbuf[i+1]>meters[1]) {
	    meters[1]=outbuf[i+1];
	  }
	}
	codec->codec_output_meters[0]=meters[0];
	codec->codec_output_meters[1]=meters[1];
	break;
    }
    codec->codec_locked=true;
  }
  else {
    codec->
      Silence(outbuf,frameCount,settings->channels(AudioSettings::Receive));
  }

  return paContinue;
}
  
  
Codec::Codec(unsigned id,QObject *parent,const char *name)
  : QObject(parent,name)
{
  //
  // Initialize Data Structures
  //
  codec_id=id;
  codec_connection_state=Codec::StateIdle;
  codec_dest_addr=NULL;
  codec_hostname="";
  codec_port=0;
  codec_sample_enc_size=0;
  codec_sample_dec_size=0;
#ifdef HAVE_MPEG_L3
  codec_last_sample_block=NULL;
  codec_lame_gfp=NULL;
  codec_lame_offset=0;
#endif  // HAVE_MPEG_L3
  codec_input_pdev=0;
  codec_output_pdev=0;
  codec_timeout=CODEC_DEFAULT_TIMEOUT;
  codec_locked=false;
  codec_lock_state=false;
  codec_frame_size=576;
  codec_src_state=NULL;
  codec_rtp_socket=NULL;
  codec_rtcp_socket=NULL;
#ifdef HAVE_VORBIS
  codec_vorbis_decoder_ready=false;
#endif  // HAVE_VORBIS
  codec_recv_total_packets_lost=0;
  codec_recv_interval_packets_lost=0;
  codec_recv_sequence_lo_number=0;
  codec_recv_sequence_hi_number=0;
  codec_recv_last_sequence_number=0;
  codec_recv_ssrc=0;
  codec_recv_ntp_timestamp=0;
  codec_recv_start_sequence_number=0;
  codec_recv_start_packet_count=0;
  for(unsigned i=0;i<Codec::SdesLastValue;i++) {
    codec_sdes_fields[i]="";
  }
  codec_sdes_fields[Codec::SdesTool]=
    QString("NetISDN-")+QString(VERSION);
  codec_ring=new RingBuffer(CODEC_RING_SIZE);
  codec_pll_offset=0.0;
  codec_loopback=false;
  codec_trans_gpio=0;
  codec_recv_gpio=0;
  codec_stream_ratio=1;
#ifdef HAVE_SPEEX
  codec_preprocess_state=NULL;
#endif  // HAVE_SPEEX
  codec_input_gain_ratio=1.0;
  codec_buffer_setpoint=CODEC_DEFAULT_BUFFER_SETPOINT;

  //
  // Get default audio devices
  //
  codec_input_pdev=Pa_GetDefaultInputDevice();
  codec_output_pdev=Pa_GetDefaultOutputDevice();

  //
  // Codec Stats
  //
  codec_stats=new CodecStats();

  //
  // Phase Locked Loop
  //
  codec_pll_average=new MeterAverage(CODEC_PLL_INTEGRATOR_PERIODS);

  //
  // OneShots
  //
  codec_trans_gpio_oneshot=new OneShot(this);
  connect(codec_trans_gpio_oneshot,SIGNAL(timeout(unsigned)),
  	  this,SLOT(transOneshotData(unsigned)));

  //
  // Timers
  //
  codec_meter_timer=new QTimer(this,"codec_meter_timer");
  connect(codec_meter_timer,SIGNAL(timeout()),this,SLOT(meterData()));

  codec_rtcp_transmit_timer=new QTimer(this,"codec_rtcp_transmit_timer");
  connect(codec_rtcp_transmit_timer,SIGNAL(timeout()),
	  this,SLOT(rtcpTransmitData()));

  codec_rtcp_receive_timer=new QTimer(this,"codec_rtcp_receive_timer");
  connect(codec_rtcp_receive_timer,SIGNAL(timeout()),
	  this,SLOT(rtcpReceiveData()));

  codec_timeout_timer=new QTimer(this,"codec_timeout_timer");
  connect(codec_timeout_timer,SIGNAL(timeout()),this,SLOT(timeoutData()));

}


Codec::~Codec()
{
  delete codec_timeout_timer;
  delete codec_rtcp_receive_timer;
  delete codec_rtcp_transmit_timer;
  delete codec_meter_timer;
  delete codec_stats;
}


QString Codec::rtpSdesField(Codec::SdesField field) const
{
  if(field>=Codec::SdesLastValue) {
    return QString();
  }
  return codec_sdes_fields[field];
}


void Codec::setRtpSdesField(Codec::SdesField field,const QString &str)
{
  if(field>=Codec::SdesLastValue) {
    return;
  }
  codec_sdes_fields[field]=str;
}


PaDeviceIndex Codec::inputDevice() const
{
  return codec_input_pdev;
}


void Codec::setInputDevice(PaDeviceIndex dev)
{
  codec_input_pdev=dev;
}


PaDeviceIndex Codec::outputDevice() const
{
  return codec_output_pdev;
}


void Codec::setOutputDevice(PaDeviceIndex dev)
{
  codec_output_pdev=dev;
}


unsigned Codec::receiveBuffers() const
{
  return codec_buffer_setpoint;
}


void Codec::setReceiveBuffers(unsigned buffers)
{
  codec_buffer_setpoint=buffers;
}


QString Codec::transmitCname() const
{
  return codec_transmit_cname;
}


void Codec::setTransmitCname(const QString &name)
{
  codec_transmit_cname=name;
  codec_stats->setTransmitCname(name);
}


unsigned Codec::timeout() const
{
  return codec_timeout;
}


void Codec::setTimeout(unsigned msecs)
{
  codec_timeout=msecs;
}


Codec::ConnectionState Codec::connectionState() const
{
  return codec_connection_state;
}


bool Codec::isLocked() const
{
  return codec_locked;
}


bool Codec::isLoopedBack() const
{
  return codec_loopback;
}


QHostAddress Codec::peerAddress() const
{
  return *codec_dest_addr;
}


CodecStats *Codec::codecStats() const
{
  return codec_stats;
}


void Codec::connectToHost(Q_UINT16 src_port,const QHostAddress &dest_addr,
			  Q_UINT16 dest_port,AudioSettings *s)
{
  if(codec_connection_state!=Codec::StateIdle) {
    return;
  }

  SetConnectionState(Codec::StateConnecting);
  codec_settings=*s;
  codec_stream_ratio=s->streamRatio(AudioSettings::Transmit);
  codec_input_gain_ratio=expf(((float)s->inputGain()/2000.0)*logf(10));

  //
  // Initialize Counters
  //
  codec_transmit_packets=0;
  codec_transmit_octets=0;
  codec_recv_start_packet_count=0;
  codec_recv_sequence_hi_number=0;

  //
  // Initialize Codec
  //
  InitEncoder(s->channels(AudioSettings::Transmit),s->sampleRate(),
	      codec_settings.bitRate(AudioSettings::Transmit)/1000,
	      s->enableProcessor(AudioSettings::Transmit));
  InitDecoder(s->sampleRate());
  if(!InitSockets(src_port)) {
    FreeDecoder();
    FreeEncoder();
    SetConnectionState(Codec::StateIdle);
    return;
  }

  //
  // Initialize Destination Address
  //
  if(!SetDestinationAddress(dest_addr,dest_port)) {
    FreeSockets();
    FreeDecoder();
    FreeEncoder();
    SetConnectionState(Codec::StateIdle);
    return;
  }

  //
  // Initialize Audio Interface
  //
  if(!InitAudioInterface()) {
    FreeSockets();
    FreeDecoder();
    FreeEncoder();
    SetConnectionState(Codec::StateIdle);
    return;
  }
  StartMeters();
}


void Codec::disconnect()
{
  uint32_t packet[1500];
  unsigned n;
  
  switch(codec_connection_state) {
    case Codec::StateIdle:
      return;

    case Codec::StateConnecting:
    case Codec::StateConnected:
      break;
  }

  StopMeters();
  FreeAudioInterface();

  //
  // Send BYE Packet
  //
  n=GenerateRtcpPacket(packet,1500);
  RTPBye *bye=new RTPBye();
  bye->addSsrc(codec_transmit_header->ssrc());
  n+=bye->writeBlock(packet+n,1500-n);
  codec_rtcp_socket->writeBlock((const char *)packet,n*sizeof(uint32_t),
				*codec_dest_addr,codec_port+1);
  delete bye;

  if(codec_locked) {
    emit locked(codec_id,false);
    codec_locked=false;
  }

  //
  // Clear Codec Stats
  //
  codec_stats->clearAll();
  codec_stats->setTransmitCname(codec_transmit_cname);
  codec_recv_sequence_hi_number=0;

  //
  // Update State
  //
  FreeSockets();
  FreeEncoder();
  codec_transmit_header=NULL;
  codec_stream_ratio=1;
  SetConnectionState(Codec::StateIdle);
}


void Codec::sendGpio(unsigned line,bool state,unsigned interval)
{
  uint32_t mask=1<<line;

  if(state) {
    if((codec_trans_gpio&mask)==0) {
      codec_trans_gpio|=mask;
      if(interval>0) {
	codec_trans_gpio_oneshot->start(line,interval);
      }
    }
  }
  else {
    if((codec_trans_gpio&mask)!=0) {
      codec_trans_gpio&=~mask;
    }
  }
}


void Codec::setLoopback(bool state)
{
  if(state!=codec_loopback) {
    codec_loopback=state;
    emit loopedBack(codec_id,state);
  }
}


void Codec::setCodebook(const QString &start_pkt,const QString &comment_pkt,
			const QString &codebook_pkt)
{
#ifdef HAVE_VORBIS
  unsigned char data[32768];
  ogg_packet op;

  memset(&op,0,sizeof(op));
  op.packet=data;
  
  //
  // Start Header
  //
  if((op.bytes=NetBase64ToData(start_pkt,data,32767))==0) {
    fprintf(stderr,"Codec::setCodebook: invalid Base64 encoding\n");
    return;
  }
  op.packetno=0;
  op.b_o_s=1;
  if(vorbis_synthesis_headerin(&codec_vorbis_recv_info,
			       &codec_vorbis_recv_comment,&op)<0) {
    fprintf(stderr,"Codec::setCodebook: invalid start header\n");
    return;
  }

  //
  // Comment Header
  //
  if((op.bytes=NetBase64ToData(comment_pkt,data,32767))==0) {
    fprintf(stderr,"Codec::setCodebook: invalid Base64 encoding\n");
    return;
  }
  op.packetno=1;
  if(vorbis_synthesis_headerin(&codec_vorbis_recv_info,
			       &codec_vorbis_recv_comment,&op)<0) {
    fprintf(stderr,"Codec::setCodebook: invalid comment header\n");
    return;
  }

  //
  // Codebook Header
  //
  if((op.bytes=NetBase64ToData(codebook_pkt,data,32767))==0) {
    fprintf(stderr,"Codec::setCodebook: invalid Base64 encoding\n");
    return;
  }
  op.packetno=2;
  if(vorbis_synthesis_headerin(&codec_vorbis_recv_info,
			       &codec_vorbis_recv_comment,&op)<0) {
    fprintf(stderr,"Codec::setCodebook: invalid codebook header\n");
    return;
  }

  //
  // Initialize the decoder
  //
  vorbis_synthesis_init(&codec_vorbis_recv_dsp_state,&codec_vorbis_recv_info);
  vorbis_block_init(&codec_vorbis_recv_dsp_state,&codec_vorbis_recv_block);
  codec_vorbis_decoder_ready=true;
#endif  // HAVE_VORBIS
}


void Codec::meterData()
{
  for(unsigned i=0;i<2;i++) {
    codec_input_averages[i]->addValue(codec_input_meters[i]);
    codec_output_averages[i]->addValue(codec_output_meters[i]);
  }
  emit leftInputChanged(codec_id,
			SampleToDb(codec_input_averages[0]->average()));
  emit rightInputChanged(codec_id,
			 SampleToDb(codec_input_averages[1]->average()));
  emit leftOutputChanged(codec_id,
			 SampleToDb(codec_output_averages[0]->average()));
  emit rightOutputChanged(codec_id,
			  SampleToDb(codec_output_averages[1]->average()));
  if(codec_locked!=codec_lock_state) {
    if(codec_locked&&(codec_connection_state==Codec::StateConnecting)) {
      SetConnectionState(Codec::StateConnected);
    }
    if(codec_locked) {
      codec_timeout_timer->stop();
    }
    else {
      codec_timeout_timer->start(codec_timeout,true);
    }
    codec_lock_state=codec_locked;
    emit locked(codec_id,codec_locked);
  }
}


void Codec::rtcpTransmitData()
{
  int n;
  uint32_t packet[1500];

  //
  // Codec Stats
  //
  codec_stats->setTransmitSsrc(codec_transmit_header->ssrc());
  codec_stats->setTransmitPacketsSent(codec_transmit_packets);
  codec_stats->
    setReceptionPacketsReceived(((codec_recv_sequence_hi_number<<16)|
				 codec_recv_last_sequence_number)-
				 (codec_recv_start_sequence_number+
				  codec_recv_total_packets_lost));
  codec_stats->setReceptionPacketsLost(codec_recv_total_packets_lost);
  codec_stats->setReceptionPllOffset(codec_pll_offset);
  double cpuload=0.0;
  if(codec_input_pstream!=NULL) {
    cpuload+=Pa_GetStreamCpuLoad(codec_input_pstream);
  }
  if(codec_output_pstream!=NULL) {
    cpuload+=Pa_GetStreamCpuLoad(codec_output_pstream);
  }
  codec_stats->setTransmitCpuLoad(cpuload);
  codec_stats->setTransmitGpioMask(codec_trans_gpio);
  codec_stats->setReceptionGpioMask(codec_recv_gpio);

  //
  // Transmit Sender Report + CNAME
  //
  n=GenerateRtcpPacket(packet,1500);
  codec_rtcp_socket->writeBlock((const char *)packet,n*sizeof(uint32_t),
				*codec_dest_addr,codec_port+1);
}


void Codec::rtcpReceiveData()
{
  int n;
  uint32_t packet[1500];
  uint8_t sddata[256];
  unsigned sdlen=0;

  //
  // Handle Incoming Packets
  //
  while((n=codec_rtcp_socket->readBlock((char *)packet,1500))>0) {
    RTPControlHeader *hdr=new RTPControlHeader();
    hdr->readBlock(packet,n);
    RTPSourceDescription *sd;
    if((sd=hdr->sourceDescription())!=NULL) {
      for(unsigned i=0;i<sd->chunks();i++) {
	switch(sd->chunkType(i)) {
	  case RTPSourceDescription::ChunkCname:
	    codec_stats->setReceptionCname(sd->chunkString(i));
	    break;

	  case RTPSourceDescription::ChunkName:
	    codec_stats->setName(sd->chunkString(i));
	    break;

	  case RTPSourceDescription::ChunkEmail:
	    codec_stats->setEmail(sd->chunkString(i));
	    break;

	  case RTPSourceDescription::ChunkPhone:
	    codec_stats->setPhone(sd->chunkString(i));
	    break;

	  case RTPSourceDescription::ChunkLoc:
	    codec_stats->setLocation(sd->chunkString(i));
	    break;

	  case RTPSourceDescription::ChunkTool:
	    codec_stats->setTool(sd->chunkString(i));
	    break;

	  case RTPSourceDescription::ChunkNote:
	    codec_stats->setNote(sd->chunkString(i));
	    break;

	  case RTPSourceDescription::ChunkPriv:
	    if((sdlen=sd->chunkData(i,sddata,256))>0) {
	      ReadPrivateSourceChunk(sddata,sdlen);
	    }
	    break;
	}
      }
    }
    RTPSenderReport *sr;
    if((sr=hdr->senderReport())!=NULL) {
      if(codec_recv_start_packet_count==0) {
	codec_recv_start_packet_count=sr->senderPacketCount();
      }
      codec_recv_ntp_timestamp=sr->ntpTimestamp();
      codec_stats->setReceptionPacketsSent(sr->senderPacketCount()-
					   codec_recv_start_packet_count);
      if(sr->receptionBlocks()>0) {  // FIXME: we only handle single senders
	RTPReceptionBlock *rb;
	rb=sr->receptionBlock(0);
	codec_stats->setReceptionSsrc(rb->ssrc());
	codec_stats->setTransmitPacketsLost(rb->cumulativePacketsLost());
	codec_stats->setTransmitFractionLost(rb->fractionPacketsLost());
      }
    }
    if(hdr->bye()!=NULL) {
      disconnect();

      //
      // Reset Receiver Stats
      //
      codec_recv_total_packets_lost=0;
      codec_recv_interval_packets_lost=0;
      codec_recv_sequence_lo_number=0;
      codec_recv_last_sequence_number=0;
      codec_recv_start_sequence_number=0;
      codec_recv_ssrc=0;
      codec_recv_ntp_timestamp=0;
      qApp->processEvents();
      delete hdr;
      return;
    }
    delete hdr;
  }
}


void Codec::timeoutData()
{
  disconnect();
}


void Codec::transOneshotData(unsigned data)
{
  codec_trans_gpio&=~(1<<data);
}


void Codec::InitEncoder(unsigned chans,unsigned samprate,unsigned bitrate,
			bool preprocess)
{
  switch(codec_settings.format(AudioSettings::Transmit)) {
    case AudioSettings::Layer3:
      InitLayer3Encoder(chans,samprate,bitrate);
      codec_transmit_header=new RTPHeader(RTPHeader::PayloadMpa);
      break;

    case AudioSettings::Speex:
      InitSpeexEncoder(samprate,bitrate/1000);
      switch(samprate) {
	case 8000:
	  codec_transmit_header=new RTPHeader(RTPHeader::PayloadSpeexNarrow);
	  break;

	case 16000:
	  codec_transmit_header=new RTPHeader(RTPHeader::PayloadSpeexWide);
	  break;

	case 32000:
	  codec_transmit_header=new RTPHeader(RTPHeader::PayloadSpeexSuperWide);
	  break;
      }
      break;

    case AudioSettings::Vorbis:
      InitVorbisEncoder(chans,samprate,bitrate);
      codec_transmit_header=new RTPHeader(RTPHeader::PayloadVorbis);
      break;

    case AudioSettings::NoFormat:
      break;
  }
  codec_transmit_header->
    setSsrc((uint32_t)((double)rand()/(double)RAND_MAX*4294967295.0));
#if HAVE_SPEEX
  if(preprocess&&(chans==1)) {
    codec_preprocess_state=
      speex_preprocess_state_init(codec_sample_enc_size,
				  codec_settings.sampleRate());
  }
#endif  // HAVE_SPEEX
}


void Codec::FreeEncoder()
{
#ifdef HAVE_SPEEX
  if(codec_preprocess_state!=NULL) {
    speex_preprocess_state_destroy(codec_preprocess_state);
    codec_preprocess_state=NULL;
  }
#endif  // HAVE_SPEEX
  switch(codec_settings.format(AudioSettings::Transmit)) {
    case AudioSettings::Layer3:
      FreeLayer3Encoder();
      break;

    case AudioSettings::Speex:
      FreeSpeexEncoder();
      break;

    case AudioSettings::Vorbis:
      FreeVorbisEncoder();
      break;

    case AudioSettings::NoFormat:
      break;
  }
}



void Codec::InitDecoder(unsigned samprate)
{
  switch(codec_settings.format(AudioSettings::Receive)) {
    case AudioSettings::Layer3:
      InitLayer3Decoder();
      break;

    case AudioSettings::Speex:
      InitSpeexDecoder(samprate);
      break;

    case AudioSettings::Vorbis:
      InitVorbisDecoder(samprate);
      break;

    case AudioSettings::NoFormat:
      break;
  }
}


void Codec::FreeDecoder()
{
  switch(codec_settings.format(AudioSettings::Receive)) {
    case AudioSettings::Speex:
      FreeSpeexDecoder();
      break;

    case AudioSettings::Layer3:
      FreeLayer3Decoder();
      break;

    case AudioSettings::Vorbis:
      FreeVorbisDecoder();
      break;

    case AudioSettings::NoFormat:
      break;
  }
}


int Codec::EncodeFrame(int16_t *inbuf,unsigned frames,
		       unsigned char *outbuf)
{
  if(codec_input_gain_ratio!=1.0) {
    for(unsigned i=0;i<frames;i++) {
      inbuf[i]=(int16_t)((float)inbuf[i]*codec_input_gain_ratio);
    }
  }
  switch(codec_settings.format(AudioSettings::Transmit)) {
    case AudioSettings::Layer3:
      return EncodeLayer3Frame(inbuf,frames,outbuf);

    case AudioSettings::Speex:
      return EncodeSpeexFrame(inbuf,frames,outbuf);

    case AudioSettings::Vorbis:
      return EncodeVorbisFrame(inbuf,frames,outbuf);

    case AudioSettings::NoFormat:
      break;
  }
  return 0;
}


int Codec::DecodeFrame(unsigned char *inbuf,unsigned bytes,uint32_t seqno,
		       int16_t *outbuf,unsigned maxframes)
{
  switch(codec_settings.format(AudioSettings::Receive)) {
    case AudioSettings::Layer3:
      return DecodeLayer3Frame(inbuf,bytes,outbuf);

    case AudioSettings::Speex:
      return DecodeSpeexFrame(inbuf,bytes,outbuf);

    case AudioSettings::Vorbis:
      return DecodeVorbisFrame(inbuf,bytes,seqno,outbuf,maxframes);

    case AudioSettings::NoFormat:
      break;
  }
  return 0;
}


void Codec::InitLayer3Encoder(unsigned chans,unsigned samprate,unsigned bitrate)
{
#ifdef HAVE_MPEG_L3
  codec_lame_gfp=lame_init();
  lame_set_in_samplerate(codec_lame_gfp,samprate);
  lame_set_brate(codec_lame_gfp,bitrate);
  lame_set_bWriteVbrTag(codec_lame_gfp,0);
  lame_set_num_channels(codec_lame_gfp,chans);
  switch(chans) {
    case 1:
      lame_set_mode(codec_lame_gfp,MONO);
      break;

    case 2:
      lame_set_mode(codec_lame_gfp,STEREO);
      break;
  }
  lame_set_strict_ISO(codec_lame_gfp,0);
  //  lame_set_padding_type(codec_lame_gfp,PAD_NO);
  lame_set_disable_reservoir(codec_lame_gfp,1);
  lame_init_params(codec_lame_gfp);
  codec_frame_size=144000*bitrate/samprate;
  codec_sample_enc_size=1152;
#endif // HAVE_MPEG_L3
}


void Codec::FreeLayer3Encoder()
{
#ifdef HAVE_MPEG_LE3
  lame_close(codec_lame_gfp);
  codec_lame_gfp=NULL;
#endif  // HAVE_MPEG_L3
}


void Codec::InitLayer3Decoder()
{
#ifdef HAVE_MPEG_L3
  mad_stream_init(&codec_mad_stream);
  mad_frame_init(&codec_mad_frame);
  mad_synth_init(&codec_mad_synth);
  codec_sample_dec_size=1152;
  codec_last_sample_block=
    new int16_t[1152*codec_settings.channels(AudioSettings::Receive)];
#endif  // HAVE_MPEG_L3
}


void Codec::FreeLayer3Decoder()
{
#ifdef HAVE_MPEG_L3
  delete codec_last_sample_block;
  codec_last_sample_block=NULL;
#endif  // HAVE_MPEG_L3
}


int Codec::EncodeLayer3Frame(const int16_t *inbuf,unsigned frames,
			     unsigned char *outbuf)
{
#ifdef HAVE_MPEG_L3
  int s;
  size_t n;
  unsigned char buf[CODEC_MAX_PACKET_SIZE];

  //
  // Process Input
  //
  switch(codec_settings.channels(AudioSettings::Transmit)) {
    case 1:
      if((s=lame_encode_buffer(codec_lame_gfp,inbuf,NULL,frames,
			       buf+codec_lame_offset,
			       CODEC_MAX_PACKET_SIZE)+codec_lame_offset)>=0) {
	codec_lame_offset=WriteLayer3Buffer(buf,s,outbuf,&n);
      }
      else {
	printf("LAME ERROR: %d\n",s);
	return 0;
      }
      break;
      
    case 2:
      if((s=lame_encode_buffer_interleaved(codec_lame_gfp,(short int *)inbuf,
					   frames,buf+codec_lame_offset,
					   CODEC_MAX_PACKET_SIZE)+
	  codec_lame_offset)>=0) {
	codec_lame_offset=WriteLayer3Buffer(buf,s,outbuf,&n);
      }
      else {
	printf("LAME ERROR: %d\n",s);
	return 0;
      }
      break;
  }
  return n;
#else
  return 0;
#endif  // HAVE_MPEG_L3
}


int Codec::DecodeLayer3Frame(unsigned char *inbuf,unsigned bytes,
			     int16_t *outbuf)
{
#ifdef HAVE_MPEG_L3
  if(inbuf==NULL) {  // No data available!
    for(unsigned i=0;
	i<(1152*codec_settings.channels(AudioSettings::Receive));i++) {
      outbuf[i]=codec_last_sample_block[i];
      codec_last_sample_block[i]=0;
    }
  }
  else {
    for(int i=0;i<MAD_BUFFER_GUARD;i++) {
      inbuf[bytes+i]=0;
    }
    mad_stream_buffer(&codec_mad_stream,inbuf,bytes+MAD_BUFFER_GUARD);
    if(mad_frame_decode(&codec_mad_frame,&codec_mad_stream)!=0) {
      return 0;
    }
    mad_synth_frame(&codec_mad_synth,&codec_mad_frame);
    for(int i=0;i<codec_mad_synth.pcm.length;i++) {
      for(int j=0;j<codec_mad_synth.pcm.channels;j++) {
	outbuf[codec_mad_synth.pcm.channels*i+j]=
	  Scale(codec_mad_synth.pcm.samples[j][i]);
	codec_last_sample_block[codec_mad_synth.pcm.channels*i+j]=
	  outbuf[codec_mad_synth.pcm.channels*i+j];
      }
    }
    return codec_mad_synth.pcm.length;
  }
  return 1152;
#else
  return 0;
#endif  // HAVE_MPEG_L3
}


int Codec::WriteLayer3Buffer(unsigned char *inbuf,size_t len,
			     unsigned char *outbuf,size_t *outlen)
{
#ifdef HAVE_MPEG_L3
  int i;
  int blocks=len/codec_frame_size;
  int offset=len%codec_frame_size;
  int l;

  for(i=0;i<blocks;i++) {
    l=4*codec_transmit_header->
      writeBlock((uint32_t *)outbuf,CODEC_MAX_PACKET_SIZE/4);
    (*codec_transmit_header)++;
    for(int j=0;j<codec_frame_size;j++) {
      outbuf[l+j]=inbuf[j+i*codec_frame_size];
    }
    *outlen=codec_frame_size+l;
    codec_transmit_packets++;
    codec_transmit_octets+=(l+codec_frame_size);
  }
  for(i=0;i<offset;i++) {
    inbuf[i]=inbuf[i+codec_frame_size*blocks];
  }
  return offset;
#else
  return 0;
#endif  // HAVE_MPEG_L3
}


void Codec::InitSpeexEncoder(unsigned samprate,unsigned quality)
{
#ifdef HAVE_SPEEX
  speex_bits_init(&codec_speex_enc_bits);
  switch(samprate) {
    case 8000:
	codec_speex_enc_state=
	    speex_encoder_init(speex_lib_get_mode(SPEEX_MODEID_NB));
      break;

    case 16000:
	codec_speex_enc_state=
	    speex_encoder_init(speex_lib_get_mode(SPEEX_MODEID_WB));
      break;

    case 32000:
	codec_speex_enc_state=
	    speex_encoder_init(speex_lib_get_mode(SPEEX_MODEID_UWB));
      break;
  }
  speex_encoder_ctl(codec_speex_enc_state,SPEEX_GET_FRAME_SIZE,
		    &codec_sample_enc_size);
  speex_encoder_ctl(codec_speex_enc_state,SPEEX_SET_QUALITY,&quality);
  int vbr=0;
  speex_encoder_ctl(codec_speex_enc_state,SPEEX_SET_VBR,&vbr);
#endif  // HAVE_SPEEX
}


void Codec::FreeSpeexEncoder()
{
#ifdef HAVE_SPEEX
  speex_bits_destroy(&codec_speex_enc_bits);
  speex_encoder_destroy(codec_speex_enc_state);
#endif  // HAVE_SPEEX
}


void Codec::InitSpeexDecoder(unsigned samprate)
{
#ifdef HAVE_SPEEX
  speex_bits_init(&codec_speex_dec_bits);
  switch(samprate) {
    case 8000:
      codec_speex_dec_state=
	  speex_decoder_init(speex_lib_get_mode(SPEEX_MODEID_NB));
      break;

    case 16000:
      codec_speex_dec_state=
	  speex_decoder_init(speex_lib_get_mode(SPEEX_MODEID_WB));
      break;

    case 32000:
      codec_speex_dec_state=
	  speex_decoder_init(speex_lib_get_mode(SPEEX_MODEID_UWB));
      break;
  }
  speex_decoder_ctl((void *)codec_speex_dec_state,SPEEX_GET_FRAME_SIZE,
		    &codec_sample_dec_size);
#endif  // HAVE_SPEEX
}


void Codec::FreeSpeexDecoder()
{
#ifdef HAVE_SPEEX
  speex_bits_destroy(&codec_speex_dec_bits);
  speex_encoder_destroy(codec_speex_dec_state);
#endif  // HAVE_SPEEX
}


int Codec::EncodeSpeexFrame(const int16_t *inbuf,unsigned frames,
			    unsigned char *outbuf)
{
#ifdef HAVE_SPEEX
  int l=4*codec_transmit_header->
    writeBlock((uint32_t *)outbuf,CODEC_MAX_PACKET_SIZE/4);
  (*codec_transmit_header)++;
  speex_bits_reset(&codec_speex_enc_bits);
  speex_encode_int(codec_speex_enc_state,(spx_int16_t *)inbuf,
		   &codec_speex_enc_bits);
  int bytes=speex_bits_write(&codec_speex_enc_bits,(char *)(outbuf+l),
			     CODEC_MAX_PACKET_SIZE)+l;
  codec_transmit_packets++;
  codec_transmit_octets+=(bytes);
  return bytes;
#else
  return 0;
#endif  // HAVE_SPEEX
}


int Codec::DecodeSpeexFrame(unsigned char *inbuf,unsigned bytes,
			    int16_t *outbuf)
{
#ifdef HAVE_SPEEX
  speex_bits_read_from(&codec_speex_dec_bits,(char *)inbuf,bytes);
  speex_decode_int(codec_speex_dec_state,&codec_speex_dec_bits,
		   (spx_int16_t *)outbuf);
  return codec_sample_dec_size;
#else
  return 0;
#endif  // HAVE_SPEEX
}


void Codec::InitVorbisEncoder(unsigned chans,unsigned samprate,unsigned bitrate)
{
#ifdef HAVE_VORBIS
  ogg_packet header;
  ogg_packet comment;
  ogg_packet codebook;

  vorbis_info_init(&codec_vorbis_trans_info);
  vorbis_encode_init(&codec_vorbis_trans_info,chans,samprate,bitrate*1000,
		     -1,-1);
  vorbis_comment_init(&codec_vorbis_trans_comment);
  vorbis_comment_add_tag(&codec_vorbis_trans_comment,"NetISDNVersion",VERSION);
  vorbis_analysis_init(&codec_vorbis_trans_dsp_state,&codec_vorbis_trans_info);
  vorbis_block_init(&codec_vorbis_trans_dsp_state,&codec_vorbis_trans_block);
  vorbis_analysis_headerout(&codec_vorbis_trans_dsp_state,
			    &codec_vorbis_trans_comment,
			    &header,&comment,&codebook);
  QString start_pkt=NetDataToBase64(header.packet,header.bytes);
  QString comment_pkt=NetDataToBase64(comment.packet,comment.bytes);
  QString codebook_pkt=NetDataToBase64(codebook.packet,codebook.bytes);
  emit codebookChanged(start_pkt,comment_pkt,codebook_pkt);

  //
  // FIXME:  We guess at a value and define it statically.  Is there a way
  //         to get/set the actual stream value in libvorbis?
  //
  codec_sample_enc_size=CODEC_VORBIS_SAMPLE_BLOCK_SIZE;
#endif  // HAVE_VORBIS
}


void Codec::FreeVorbisEncoder()
{
#ifdef HAVE_VORBIS
  vorbis_block_clear(&codec_vorbis_trans_block);
  vorbis_dsp_clear(&codec_vorbis_trans_dsp_state);
  vorbis_info_clear(&codec_vorbis_trans_info);
#endif  // HAVE_VORBIS
}


void Codec::InitVorbisDecoder(unsigned samprate)
{
#ifdef HAVE_VORBIS
  //
  // Just do some basic setup here, as we can't fully initialize things
  // until we get the codebook from the far end.
  //
  vorbis_info_init(&codec_vorbis_recv_info);
  vorbis_comment_init(&codec_vorbis_recv_comment);

  //
  // FIXME:  We guess at a value and define it statically.  Is there a way
  //         to get/set the actual stream value in libvorbis?
  //
  codec_sample_dec_size=CODEC_VORBIS_SAMPLE_BLOCK_SIZE;
#endif  // HAVE_VORBIS
}


void Codec::FreeVorbisDecoder()
{
#ifdef HAVE_VORBIS
  if(codec_vorbis_decoder_ready) {
    vorbis_block_clear(&codec_vorbis_recv_block);
    vorbis_dsp_clear(&codec_vorbis_recv_dsp_state);
  }
  vorbis_info_clear(&codec_vorbis_recv_info);
  codec_vorbis_decoder_ready=false;
#endif  // HAVE_VORBIS
}


int Codec::EncodeVorbisFrame(const int16_t *inbuf,unsigned frame,
			     unsigned char *outbuf)
{
#ifdef VORBIS
  //
  // Generate the RTP header
  //
  int l=4*codec_transmit_header->
    writeBlock((uint32_t *)outbuf,CODEC_MAX_PACKET_SIZE/4);
  outbuf[l++]=0;  // Vorbis payload header, Ident=0x000
  outbuf[l++]=0;
  outbuf[l++]=0;
  outbuf[l++]=1;  // 1 vorbis packet per rtp packet
  outbuf[l++]=0;  // Packet length -- we'll fill this out after we
  outbuf[l++]=0;  // generate the data


  ogg_packet op;
  float **buffer=vorbis_analysis_buffer(&codec_vorbis_trans_dsp_state,frame);

  // 
  // PCM in
  //
  for(unsigned i=0;i<frame;i++) {
    for(unsigned j=0;j<codec_settings.channels(AudioSettings::Transmit);j++) {
      buffer[j][i]=
	(float)inbuf[codec_settings.channels(AudioSettings::Transmit)*i+j]/
	32768.0;
    }
  }
  vorbis_analysis_wrote(&codec_vorbis_trans_dsp_state,frame);

  //
  // Vorbis packet out
  //
  if(vorbis_analysis_blockout(&codec_vorbis_trans_dsp_state,
			      &codec_vorbis_trans_block)!=1) {
    return 0;
  }
  vorbis_analysis(&codec_vorbis_trans_block,NULL);
  vorbis_bitrate_addblock(&codec_vorbis_trans_block);
  if(!vorbis_bitrate_flushpacket(&codec_vorbis_trans_dsp_state,&op)) {
    return 0;
  }

  for(int i=0;i<op.bytes;i++) {
    outbuf[l+i]=op.packet[i];
  }

  //
  // Update the packet length
  //
  outbuf[l-2]=0xff&((op.bytes)/256);
  outbuf[l-1]=0xff&((op.bytes)%256);

  codec_transmit_packets++;
  codec_transmit_octets+=(l+op.bytes);
  // printf("%s\n",(const char *)DumpOggPacket(QString().sprintf("encode: %d",codec_transmit_header->sequenceNumber()),&op));
  (*codec_transmit_header)++;

  if((l+op.bytes)>1500) {
    fprintf(stderr,
    "Codec::EncodeVorbisFrame(): giant packet generated, size=%ld",l+op.bytes);
    return 1500;
  }

  return l+op.bytes;
#else
  return 0;
#endif  // HAVE_VORBIS
}


int Codec::DecodeVorbisFrame(unsigned char *inbuf,unsigned bytes,
			     uint32_t seqno,int16_t *outbuf,unsigned maxframes)
{
#ifdef HAVE_VORBIS
  if(!codec_vorbis_decoder_ready) {
    return 0;
  }

  ogg_packet op;
  int frames;
  int total_frames=0;
  float **pcm;

  if(bytes<6) {  // Skip across the payload header
    fprintf(stderr,"**** RUNT PACKET! ****\n");
    return 0;
  }
  if(inbuf[3]!=1) {  // Ignore everything other than raw Vorbis data
    return 0;
  }
  memset(&op,0,sizeof(op));
  op.packet=inbuf+6;
  op.bytes=bytes-6;
  // printf("%s\n",(const char *)DumpOggPacket(QString().sprintf("decode: %d",seqno),&op));

  if(vorbis_synthesis(&codec_vorbis_recv_block,&op)==0) {
    vorbis_synthesis_blockin(&codec_vorbis_recv_dsp_state,
			     &codec_vorbis_recv_block);
  }
  while((frames=vorbis_synthesis_pcmout(&codec_vorbis_recv_dsp_state,&pcm))>0) {
    if((frames+total_frames)>(int)maxframes) {
      fprintf(stderr,"Codec::DecodeVorbisFrames: exceeded maxframes\n");
      return total_frames;
    }
    for(int i=0;i<frames;i++) {
      for(unsigned j=0;j<codec_settings.channels(AudioSettings::Receive);j++) {
	outbuf[codec_settings.channels(AudioSettings::Receive)*
	       (total_frames+i)+j]=(int16_t)(32768.0*pcm[j][i]);
      }
    }
    total_frames+=frames;
    // printf("Frames read: %d\n",frames);
    vorbis_synthesis_read(&codec_vorbis_recv_dsp_state,frames);
  }

  return total_frames;
#else
  return 0;
#endif  // HAVE_VORBIS
}


#ifdef HAVE_VORBIS
QString Codec::DumpOggPacket(const QString &title,ogg_packet *op)
{
  QString ret;
  ret+="**** Ogg Packet Dump ****\n";
  ret+=QString().sprintf("%s.bytes: %ld\n",(const char *)title,op->bytes);
  ret+=QString().sprintf("%s.b_o_s: %ld\n",(const char *)title,op->b_o_s);
  ret+=QString().sprintf("%s.e_o_s: %ld\n",(const char *)title,op->e_o_s);
  ret+=QString().sprintf("%s.granulepos: %ld\n",
			 (const char *)title,op->granulepos);
  ret+=QString().sprintf("%s.packetno: %ld\n",
			 (const char *)title,op->packetno);
  ret+="**************************\n";

  return ret;
}
#endif  // HAVE_VORBIS


void Codec::Silence(int16_t *buf,unsigned frames,unsigned chans)
{
  for(unsigned i=0;
      i<(frames*chans);i++) {  
    buf[i]=0;
  }
  codec_output_meters[0]=0;
  codec_output_meters[1]=0;
  codec_locked=false;
}


unsigned Codec::GenerateRtcpPacket(uint32_t *packet,unsigned len)
{
  int n;
  QDateTime dt=QDateTime(QDate::currentDate(),QTime::currentTime());

  RTPSourceDescription *sd=new RTPSourceDescription();
  RTPSenderReport *sr=new RTPSenderReport(codec_transmit_header->ssrc());
  sr->setNtpTimestamp(GetNtpTimestamp(dt));
  sr->setRtpTimestamp(90000*codec_transmit_packets);
  sr->setSenderPacketCount(codec_transmit_packets);
  sr->setSenderOctetCount(codec_transmit_octets);
  if(codec_recv_sequence_lo_number>codec_recv_last_sequence_number) {
    //
    // FIXME: Jitter and Report Delay stats still need to be implemented!
    //
    RTPReceptionBlock *rb=new RTPReceptionBlock(codec_transmit_header->ssrc());
    rb->setFractionPacketsLost((double)codec_recv_interval_packets_lost/
			       (double)(codec_recv_sequence_lo_number-
					codec_recv_last_sequence_number));
    rb->setCumulativePacketsLost(codec_recv_total_packets_lost);
    rb->setHighestSequenceNumber((codec_recv_sequence_hi_number<<16)|
				 (0xffff&codec_recv_sequence_lo_number));
    rb->setLastSendReportTimestamp(0xffffffff&(codec_recv_ntp_timestamp>>16));
    rb->setSendReportDelay(0);
    sr->addReceptionBlock(rb);
    codec_recv_interval_packets_lost=0;
    codec_recv_last_sequence_number=codec_recv_sequence_lo_number;
  }
  n=sr->writeBlock(packet,len);
  sd->addChunk(RTPSourceDescription::ChunkCname,codec_transmit_header->ssrc(),
	       codec_transmit_cname);
  //
  // Insert GPIO Data
  //
  unsigned char gpio[9];
  gpio[0]=4;
  gpio[1]='G';
  gpio[2]='P';
  gpio[3]='I';
  gpio[4]='O';
  gpio[5]=(codec_trans_gpio>>24)&0xff;
  gpio[6]=(codec_trans_gpio>>16)&0xff;
  gpio[7]=(codec_trans_gpio>>8)&0xff;
  gpio[8]=codec_trans_gpio&0xff;
  sd->addChunk(RTPSourceDescription::ChunkPriv,codec_transmit_header->ssrc(),
	       gpio,9);
  n+=sd->writeBlock(packet+n,len-n);

  delete sr;
  delete sd;
  // Reception block memory management is handled by RTPSenderReport
  // Don't delete them here!

  return n;
}


uint64_t Codec::GetNtpTimestamp(const QDateTime &dt)
{
  double stamp=QDateTime(QDate(1900,1,1),QTime(0,0,0,1)).
    secsTo(QDateTime(dt.date(),QTime(0,0,0,1)));
  stamp+=(double)(QTime(0,0,0,1).msecsTo(dt.time())/1000.0);
  return (uint64_t)(4294967296.0*stamp);
}


int Codec::SampleToDb(double samp)
{
  int ret;

  if(samp==0.0) {
    return -10000;
  }
  ret=(int)(2000.0*log10(samp/32767.0));
  if(ret<-10000) {
    return -10000;
  }
  return ret;
}


void Codec::SetConnectionState(Codec::ConnectionState state)
{
  codec_connection_state=state;
  emit connectionStateChanged(codec_id,state);
}


bool Codec::InitSockets(Q_UINT16 port)
{
  codec_rtp_socket=new QSocketDevice(QSocketDevice::Datagram);
  codec_rtp_socket->setBlocking(false);
  if(!codec_rtp_socket->bind(QHostAddress(),port)) {
    delete codec_rtp_socket;
    codec_rtp_socket=NULL;
    return false;
  }
  codec_rtcp_socket=new QSocketDevice(QSocketDevice::Datagram);
  codec_rtcp_socket->setBlocking(false);
  if(!codec_rtcp_socket->bind(QHostAddress(),port+1)) {
    delete codec_rtp_socket;
    codec_rtp_socket=NULL;
    delete codec_rtcp_socket;
    codec_rtcp_socket=NULL;
    return false;
  }
  codec_rtcp_transmit_timer->start(100);
  codec_rtcp_receive_timer->start(10);
  return true;
}


void Codec::FreeSockets()
{
  codec_rtcp_transmit_timer->stop();
  codec_rtcp_receive_timer->stop();
  delete codec_rtcp_socket;
  codec_rtcp_socket=NULL;
  delete codec_rtp_socket;
  codec_rtp_socket=NULL;
}


bool Codec::InitAudioInterface()
{
  PaError perr;
  int src_err;
  struct PaStreamParameters in_params;
  struct PaStreamParameters out_params;

  //
  // Phase Locked Loop
  //
  codec_recv_total_packets_lost=0;
  codec_recv_interval_packets_lost=0;
  codec_recv_sequence_lo_number=0;
  codec_recv_sequence_hi_number=0;
  codec_recv_last_sequence_number=0;
  codec_recv_ssrc=0;
  codec_recv_ntp_timestamp=0;
  codec_recv_start_sequence_number=0;
  codec_recv_start_packet_count=0;
  codec_ring->reset();
  codec_pll_offset=0.0;
  if((codec_src_state=
      src_new(SRC_SINC_MEDIUM_QUALITY,
	 codec_settings.channels(AudioSettings::Receive),&src_err))==NULL) {
    fprintf(stderr,"src error: %s\n",src_strerror(src_err));
    return false;
  }
  codec_pll_average->preset(0.0);

  //
  // Output Interface
  //
  memset(&out_params,0,sizeof(out_params));
  out_params.device=codec_output_pdev;
  out_params.channelCount=codec_settings.channels(AudioSettings::Receive);
  out_params.sampleFormat=paInt16;
  if((perr=Pa_OpenStream(&codec_output_pstream,NULL,&out_params,
			 codec_settings.sampleRate(),codec_sample_dec_size,
			 paNoFlag,AudioOutputCallback,this))!=
     paNoError) {
    printf("Pa Error: %s\n",Pa_GetErrorText(perr));
    Pa_CloseStream(codec_input_pstream);
    return false;
  }
  if((perr=Pa_StartStream(codec_output_pstream))!=paNoError) {
    printf("Pa Error: %s\n",Pa_GetErrorText(perr));
    Pa_CloseStream(codec_output_pstream);
    Pa_CloseStream(codec_input_pstream);
    return false;
  }

  //
  // Input Interface
  //
  memset(&in_params,0,sizeof(in_params));
  in_params.device=codec_input_pdev;
  in_params.channelCount=codec_settings.channels(AudioSettings::Transmit);
  in_params.sampleFormat=paInt16;
  if((perr=Pa_OpenStream(&codec_input_pstream,&in_params,NULL,
			 codec_settings.sampleRate(),codec_sample_enc_size,
			 paNoFlag,AudioInputCallback,this))!=
     paNoError) {
    printf("Pa Error: %s\n",Pa_GetErrorText(perr));
    return false;
  }
  if((perr=Pa_StartStream(codec_input_pstream))!=paNoError) {
    printf("Pa Error: %s\n",Pa_GetErrorText(perr));
    Pa_CloseStream(codec_input_pstream);
    return false;
  }

  return true;
}


void Codec::FreeAudioInterface()
{
  Pa_StopStream(codec_output_pstream);
  Pa_CloseStream(codec_output_pstream);
  Pa_StopStream(codec_input_pstream);
  Pa_CloseStream(codec_input_pstream);
  codec_src_state=src_delete(codec_src_state);
}


void Codec::StartMeters()
{
  codec_input_averages[0]=new MeterAverage(METER_AVERAGING_PERIODS);
  codec_input_averages[1]=new MeterAverage(METER_AVERAGING_PERIODS);
  codec_output_averages[0]=new MeterAverage(METER_AVERAGING_PERIODS);
  codec_output_averages[1]=new MeterAverage(METER_AVERAGING_PERIODS);
  codec_meter_timer->start(METER_UPDATE_INTERVAL);
}


void Codec::StopMeters()
{
  codec_meter_timer->stop();
  codec_locked=false;
  codec_lock_state=false;
  emit locked(codec_id,false);
  emit leftInputChanged(codec_id,-10000);
  emit rightInputChanged(codec_id,-10000);
  emit leftOutputChanged(codec_id,-10000);
  emit rightOutputChanged(codec_id,-10000);
  delete codec_input_averages[0];
  delete codec_input_averages[1];
  delete codec_output_averages[0];
  delete codec_output_averages[1];
}


bool Codec::SetDestinationAddress(const QHostAddress &addr,Q_UINT16 port)
{
  codec_dest_addr=new QHostAddress(addr);
  if(codec_dest_addr->isNull()) {
    delete codec_dest_addr;
    codec_dest_addr=NULL;
    return false;
  }
  codec_port=port;
  return true;
}


void Codec::ReadPrivateSourceChunk(uint8_t *data,unsigned len)
{
  uint32_t gpio=0;

  //
  // Look for the GPIO chunk, ignore all others
  //
  if((len!=9)||(data[0]!=4)) {
    return;
  }
  if((data[1]!='G')||(data[2]!='P')||(data[3]!='I')||(data[4]!='O')) {
    return;
  }
  gpio=(((uint32_t)data[5])<<24)|(((uint32_t)data[6])<<16)|
    (((uint32_t)data[7])<<8)|((uint32_t)data[8]);

  for(unsigned i=0;i<32;i++) {
    uint32_t mask=1<<i;
    if((gpio&mask)!=(codec_recv_gpio&mask)) {
      emit gpioChanged(codec_id,i,(gpio&mask)!=0);
    }
  }
  codec_recv_gpio=gpio;
}
