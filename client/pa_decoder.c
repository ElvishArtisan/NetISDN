/* pa_decoder.c
 *
 * Receive and play an L3 stream from a remote system using PortAudio
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
#include <portaudio.h>

#include <sys/soundcard.h>


volatile int stopping=0;
int audio_fd;
int dest_sock;
unsigned char mpeg_buffer[4096];
struct mad_stream mad_stream;
struct mad_frame mad_frame;
struct mad_synth mad_synth;
PaStream *pstream;


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


int AudioCallback(const void *input, void *output, unsigned long frameCount, 
		  const PaStreamCallbackTimeInfo *timeInfo, 
		  PaStreamCallbackFlags statusFlags, void *userData)
{
  int i;
  int j;
  int n;
  int16_t *outbuf=(int16_t *)output;
  unsigned char buf[4096];

  if((n=recvfrom(dest_sock,buf,4096,O_NONBLOCK,NULL,NULL))>2) {
    for(i=0;i<MAD_BUFFER_GUARD;i++) {
      buf[n+i]=0;
    }
    mad_stream_buffer(&mad_stream,buf,n+MAD_BUFFER_GUARD);
    if(mad_frame_decode(&mad_frame,&mad_stream)!=0) {
    }
    mad_synth_frame(&mad_synth,&mad_frame);
    for(i=0;i<mad_synth.pcm.length;i++) {
      for(j=0;j<mad_synth.pcm.channels;j++) {
        outbuf[2*i+j]=scale(mad_synth.pcm.samples[j][i]);
      }
    }
  }
  else {
    for(i=0;i<frameCount;i++) {  // Mute if no audio available
      outbuf[i]=0;
    }
  }

  return paContinue;
}


int main(int argc,char *argv[])
{
  int i;
  int j;
  int arg;
  int n;
  int s;
  struct sockaddr_in dest_sa;
  PaError perr;
  PaDeviceInfo *pdev;
  struct PaStreamParameters psparam;

  //
  // Open Destination Socket
  //
  if((dest_sock=socket(PF_INET,SOCK_DGRAM,0))<0) {
    perror("decoder");
    exit(256);
  }
  fcntl(dest_sock,F_SETFL,O_NONBLOCK);
  memset(&dest_sa,0,sizeof(dest_sa));
  dest_sa.sin_family=AF_INET;
  dest_sa.sin_port=htons(1234);
  dest_sa.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(dest_sock,(struct sockaddr *)&dest_sa,sizeof(dest_sa))<0) {
    perror("decoder");
    exit(256);
  }

  //
  // Initialize MAD Decoder
  //
  mad_stream_init(&mad_stream);
  mad_frame_init(&mad_frame);
  mad_synth_init(&mad_synth);

  //
  // Open the audio interface
  //
//  audio_fd=open("/home/fredg/wav/output.wav",O_WRONLY|O_CREAT|O_TRUNC);
  if((perr=Pa_Initialize())!=paNoError) {
    fprintf(stderr,"pa_decoder: %s\n",Pa_GetErrorText(perr));
    exit(256);
  }
  memset(&psparam,0,sizeof(psparam));
  psparam.device=0;
  psparam.channelCount=2;
  psparam.sampleFormat=paInt16;
  if((perr=Pa_OpenStream(&pstream,NULL,&psparam,48000,1152,paNoFlag,
			 AudioCallback,NULL))!=paNoError) {
    fprintf(stderr,"pa_decoder: %s\n",Pa_GetErrorText(perr));
    Pa_Terminate();
    exit(256);
  }
  Pa_StartStream(pstream);

  //
  // Initialize Signals
  //
  signal(SIGTERM,SigHandler);
  signal(SIGINT,SigHandler);
  signal(SIGCHLD,SigHandler);

  //
  // Main Loop
  //
  while(stopping==0) {
  }

  close(dest_sock);
  Pa_Terminate();

  exit(0);
}
