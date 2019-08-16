/* encoder.c
 *
 * Encode a live L3 stream and send it to a remote system
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

#include <lame/lame.h>
#include <portaudio.h>


volatile int stopping=0;
lame_global_flags *lame_gfp;
int lame_offset=0;
int src_fd;
struct sockaddr_in dest_sa;

void SigHandler(int signo)
{
  pid_t pid;
  int i;

  switch(signo) {
    case SIGTERM:
    case SIGINT:
      stopping=1;
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


int WriteBuffer(int fd,struct sockaddr_in *sa,char *buf,size_t len)
{
  int i;
  int blocks=len/576;
  int offset=len%576;

  for(i=0;i<blocks;i++) {
    if(sendto(fd,buf+i*576,576,0,(struct sockaddr *)sa,
	      sizeof(*sa))<0) {
      perror("send error");
    }
  }
  for(i=0;i<offset;i++) {
    buf[i]=buf[i+576*blocks];
  }
  return offset;
}


int AudioCallback(const void *input, void *output, unsigned long frameCount, 
		  const PaStreamCallbackTimeInfo *timeInfo, 
		  PaStreamCallbackFlags statusFlags, void *userData)
{
  int i;
  int j;
  int n;
  int s;
  int16_t *inbuf=(int16_t *)input;
  char lame_buf[LAME_MAXMP3BUFFER];

  if((s=lame_encode_buffer_interleaved(lame_gfp,inbuf,frameCount,
				       lame_buf+lame_offset,
				       LAME_MAXMP3BUFFER)+lame_offset)>=0) {
    lame_offset=WriteBuffer(src_fd,&dest_sa,lame_buf,s);
  }
  else {
    printf("LAME ERROR: %d\n",s);
  }

  return paContinue;
}


int main(int argc,char *argv[])
{
  int snd;
  int arg;
  char buf[4608];
  char lame_buf[LAME_MAXMP3BUFFER];
  int n;
  int s;
  struct sockaddr_in sa;
  PaError perr;
  PaDeviceInfo *pdev;
  struct PaStreamParameters psparam;
  PaStream *pstream;

  //
  // Initialize Encoder
  //
  lame_gfp=lame_init();
  lame_set_in_samplerate(lame_gfp,48000);
  lame_set_brate(lame_gfp,192);
  lame_set_bWriteVbrTag(lame_gfp,0);
  lame_set_num_channels(lame_gfp,2);
  lame_set_mode(lame_gfp,STEREO);
  lame_set_strict_ISO(lame_gfp,1);
  lame_set_disable_reservoir(lame_gfp,1);
  lame_init_params(lame_gfp);

  //
  // Open Source Socket
  //
  if((src_fd=socket(PF_INET,SOCK_DGRAM,0))<0) {
    perror("encoder");
    exit(256);
  }
  memset(&sa,0,sizeof(sa));
  sa.sin_family=AF_INET;
  sa.sin_port=htons(9876);
  sa.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(src_fd,(struct sockaddr *)&sa,sizeof(sa))<0) {
    perror("encoder");
    exit(256);
  }

  //
  // Open the audio interface
  //
  if((perr=Pa_Initialize())!=paNoError) {
    fprintf(stderr,"pa_encoder: %s\n",Pa_GetErrorText(perr));
    exit(256);
  }
  memset(&psparam,0,sizeof(psparam));
  psparam.device=0;
  psparam.channelCount=2;
  psparam.sampleFormat=paInt16;
  if((perr=Pa_OpenStream(&pstream,&psparam,NULL,48000,1152,paNoFlag,
			 AudioCallback,NULL))!=paNoError) {
    fprintf(stderr,"pa_encoder: %s\n",Pa_GetErrorText(perr));
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
  // Initialize Destination Address
  //
  memset(&dest_sa,0,sizeof(dest_sa));
  dest_sa.sin_family=AF_INET;
  dest_sa.sin_port=htons(1234);
  dest_sa.sin_addr.s_addr=htonl(0xC0A80A16);  // 192.168.10.22

  while(stopping==0) {
  }
  if((s=lame_encode_flush(lame_gfp,lame_buf,LAME_MAXMP3BUFFER))>0) {
    write(src_fd,lame_buf,s);
  }
  lame_mp3_tags_fid(lame_gfp,NULL);
  lame_close(lame_gfp);
  close(src_fd);
  close(snd);

  exit(0);
}
