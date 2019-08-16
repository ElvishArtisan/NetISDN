/* decoder.c
 *
 * Receive and play an L3 stream from a remote system
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#include <mad.h>

#include <sys/soundcard.h>


volatile int stopping=0;
int audio_fd;
int dest_sock;
unsigned char mpeg_buffer[4096];


void SigHandler(int signo)
{
  pid_t pid;
  int i;

  switch(signo) {
    case SIGTERM:
    case SIGINT:
      exit(256);
      break;

    case SIGCHLD:
      pid=waitpid(-1,NULL,WNOHANG);
      while(pid>0) {
	pid=waitpid(-1,NULL,WNOHANG);
      }
      signal(SIGCHLD,SigHandler);
      break;
  }
}


















static int PrintFrameInfo(FILE *fp, struct mad_header *Header)
{
	const char	*Layer,
				*Mode,
				*Emphasis;

	/* Convert the layer number to it's printed representation. */
	switch(Header->layer)
	{
		case MAD_LAYER_I:
			Layer="I";
			break;
		case MAD_LAYER_II:
			Layer="II";
			break;
		case MAD_LAYER_III:
			Layer="III";
			break;
		default:
			Layer="(unexpected layer value)";
			break;
	}

	/* Convert the audio mode to it's printed representation. */
	switch(Header->mode)
	{
		case MAD_MODE_SINGLE_CHANNEL:
			Mode="single channel";
			break;
		case MAD_MODE_DUAL_CHANNEL:
			Mode="dual channel";
			break;
		case MAD_MODE_JOINT_STEREO:
			Mode="joint (MS/intensity) stereo";
			break;
		case MAD_MODE_STEREO:
			Mode="normal LR stereo";
			break;
		default:
			Mode="(unexpected mode value)";
			break;
	}

	/* Convert the emphasis to it's printed representation. Note that
	 * the MAD_EMPHASIS_RESERVED enumeration value appeared in libmad
	 * version 0.15.0b.
	 */
	switch(Header->emphasis)
	{
		case MAD_EMPHASIS_NONE:
			Emphasis="no";
			break;
		case MAD_EMPHASIS_50_15_US:
			Emphasis="50/15 us";
			break;
		case MAD_EMPHASIS_CCITT_J_17:
			Emphasis="CCITT J.17";
			break;
#if (MAD_VERSION_MAJOR>=1) || \
	((MAD_VERSION_MAJOR==0) && (MAD_VERSION_MINOR>=15))
		case MAD_EMPHASIS_RESERVED:
			Emphasis="reserved(!)";
			break;
#endif
		default:
			Emphasis="(unexpected emphasis value)";
			break;
	}

	fprintf(fp,"%lu kb/s audio MPEG layer %s stream %s CRC, "
			"%s with %s emphasis at %d Hz sample rate\n",
			Header->bitrate,Layer,
			Header->flags&MAD_FLAG_PROTECTION?"with":"without",
			Mode,Emphasis,Header->samplerate);
	return(ferror(fp));
}


signed int scale(mad_fixed_t sample)
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










enum mad_flow MadInput(void *buf,struct mad_stream *s)
{
  int n;
  static int count=0;

  printf("MadInput()\n");
  if((n=recvfrom(dest_sock,buf,4096,0,NULL,NULL))>2) {
    mad_stream_buffer(s,buf,n);
  }

  if(count++>10) {
    return MAD_FLOW_STOP;
  }
  return MAD_FLOW_CONTINUE;
}


enum mad_flow MadHeader(void *buf,struct mad_header const *h)
{
  printf("MadHeader()\n");
  return MAD_FLOW_STOP;
}


enum mad_flow MadFilter(void *buf,struct mad_stream const *s,
			 struct mad_frame *f)
{
  printf("MadFilter()\n");
  return MAD_FLOW_STOP;
}


enum mad_flow MadOutput(void *buf,struct mad_header const *h,
			struct mad_pcm *pcm)
{
  printf("MadOutput()\n");
  return MAD_FLOW_STOP;
}


enum mad_flow MadError(void *buf,struct mad_stream *s,struct mad_frame *f)
{
  printf("MAD Stream Error: %s\n",mad_stream_errorstr(s));
  exit(256);
}


enum mad_flow MadMessage(void *buf1,void *buf2,unsigned int *ptr)
{
  printf("MadMessage()\n");
  return MAD_FLOW_STOP;
}


int main(int argc,char *argv[])
{
  int i;
  int j;
  int arg;
  unsigned char buf[8192];
  int16_t outbuf[4096];
  int n;
  int s;
  //struct mad_decoder mad;
  struct mad_stream stream;
  struct mad_frame frame;
  struct mad_synth synth;
  mad_timer_t timer;
  struct sockaddr_in sa;

  //
  // Open Destination Socket
  //
  if((dest_sock=socket(PF_INET,SOCK_DGRAM,0))<0) {
    perror("decoder");
    exit(256);
  }
  memset(&sa,0,sizeof(sa));
  sa.sin_family=AF_INET;
  sa.sin_port=htons(1234);
  sa.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(dest_sock,(struct sockaddr *)&sa,sizeof(sa))<0) {
    perror("decoder");
    exit(256);
  }

  //
  // Open the audio interface
  //
//  audio_fd=open("/home/fredg/wav/output.wav",O_WRONLY|O_CREAT|O_TRUNC);
  if((audio_fd=open("/dev/dsp",O_WRONLY))<0) {
    perror("decoder");
    exit(256);
  }
  arg=2;
  if(ioctl(audio_fd,SNDCTL_DSP_CHANNELS,&arg)<0) {
    perror("decoder");
    exit(256);
  }
  printf("Using %d channels\n",arg);
  arg=AFMT_S16_LE;
  if(ioctl(audio_fd,SNDCTL_DSP_SETFMT,&arg)<0) {
    perror("decoder");
    exit(256);
  }
  if(arg!=AFMT_S16_LE) {
    printf("Unable to set sign 16 bit little-endian mode!\n");
    exit(256);
  }
  arg=48000;
  if(ioctl(audio_fd,SNDCTL_DSP_SPEED,&arg)<0) {
    perror("decoder");
    exit(256);
  }
  printf("Using %d samples/sec\n",arg);

  //
  // Initialize Signals
  //
  signal(SIGTERM,SigHandler);
  signal(SIGINT,SigHandler);
  signal(SIGCHLD,SigHandler);

/*
  //
  // Initialize Decoder
  //
  mad_decoder_init(&mad,mpeg_buffer,MadInput,MadHeader,MadFilter,MadOutput,
		   MadError,MadMessage);
*/
  mad_stream_init(&stream);
  mad_frame_init(&frame);
  mad_synth_init(&synth);
  mad_timer_reset(&timer);

/*
  //
  // Main Loop
  //
  mad_decoder_run(&mad,MAD_DECODER_MODE_SYNC);
*/


  while(stopping==0) {
    if((n=recvfrom(dest_sock,buf,4096,0,NULL,NULL))>2) {
      for(i=0;i<MAD_BUFFER_GUARD;i++) {
	buf[n+i]=0;
      }
      mad_stream_buffer(&stream,buf,n+MAD_BUFFER_GUARD);
      if(mad_frame_decode(&frame,&stream)!=0) {
	if(MAD_RECOVERABLE(stream.error)) {
	  if(stream.error!=MAD_ERROR_LOSTSYNC ||
	     stream.this_frame!=(buf+n)) {
	    printf("THIS: %p  GUARD: %p\n",stream.this_frame,buf+n);
	    fprintf(stderr,"recoverable frame level error (%s)\n",
		    mad_stream_errorstr(&stream));
	    fflush(stderr);
	    continue;
	  }
	}
      }
      mad_synth_frame(&synth,&frame);
      for(i=0;i<synth.pcm.length;i++) {
	for(j=0;j<synth.pcm.channels;j++) {
	  outbuf[2*i+j]=scale(synth.pcm.samples[j][i]);
	}
      }
      write(audio_fd,outbuf,synth.pcm.length*2*synth.pcm.channels);
    }
  }







//  mad_decoder_finish(&mad);

  close(dest_sock);
  close(audio_fd);

  exit(0);
}
