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
#include <sys/soundcard.h>


volatile int stopping=0;


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


int main(int argc,char *argv[])
{
  int snd;
  int fd;
  int arg;
  char buf[4608];
  int offset=0;
  char lame_buf[LAME_MAXMP3BUFFER];
  int n;
  int s;
  lame_global_flags *gfp;
  struct sockaddr_in sa;
  struct sockaddr_in dest_sa;

  //
  // Open the audio interface
  //
  if((snd=open("/dev/dsp",O_RDONLY))<0) {
    perror("encoder");
    exit(256);
  }
  arg=2;
  if(ioctl(snd,SNDCTL_DSP_CHANNELS,&arg)<0) {
    perror("encoder");
    exit(256);
  }
  printf("Using %d channels\n",arg);
  arg=AFMT_S16_LE;
  if(ioctl(snd,SNDCTL_DSP_SETFMT,&arg)<0) {
    perror("encoder");
    exit(256);
  }
  if(arg!=AFMT_S16_LE) {
    printf("Unable to set sign 16 bit little-endian mode!\n");
    exit(256);
  }
  arg=48000;
  if(ioctl(snd,SNDCTL_DSP_SPEED,&arg)<0) {
    perror("encoder");
    exit(256);
  }
  printf("Using %d samples/sec\n",arg);

  //
  // Open Source Socket
  //
  if((fd=socket(PF_INET,SOCK_DGRAM,0))<0) {
    perror("encoder");
    exit(256);
  }
  memset(&sa,0,sizeof(sa));
  sa.sin_family=AF_INET;
  sa.sin_port=htons(9876);
  sa.sin_addr.s_addr=htonl(INADDR_ANY);
  if(bind(fd,(struct sockaddr *)&sa,sizeof(sa))<0) {
    perror("encoder");
    exit(256);
  }

  //
  // Initialize Encoder
  //
  gfp=lame_init();
  lame_set_in_samplerate(gfp,48000);
  lame_set_brate(gfp,192);
  lame_set_bWriteVbrTag(gfp,0);
  lame_set_num_channels(gfp,2);
  lame_set_mode(gfp,STEREO);
  lame_set_strict_ISO(gfp,1);
  lame_set_disable_reservoir(gfp,1);
  lame_init_params(gfp);

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
  dest_sa.sin_addr.s_addr=htonl(0xC0A80A15);  // 192.168.10.21

  while(stopping==0) {
    n=read(snd,buf,4608);
    if((s=lame_encode_buffer_interleaved(gfp,(short int *)buf,n/4,
					 lame_buf+offset,
					 LAME_MAXMP3BUFFER)+offset)>=0) {
      offset=WriteBuffer(fd,&dest_sa,lame_buf,s);
    }
    else {
      printf("LAME ERROR: %d\n",s);
    }
  }
  if((s=lame_encode_flush(gfp,lame_buf,LAME_MAXMP3BUFFER))>0) {
    write(fd,lame_buf,s);
  }
  lame_mp3_tags_fid(gfp,NULL);
  lame_close(gfp);
  close(fd);
  close(snd);

  exit(0);
}
