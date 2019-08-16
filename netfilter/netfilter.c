/* netfilter.cpp
 *
 * (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <linux/types.h>
#include <linux/netfilter.h>

#include <libnetfilter_queue/libnetfilter_queue.h>

double loss_ratio=0.0;

static int callback(struct nfq_q_handle *queue,struct nfgenmsg *msg,
	     struct nfq_data *nfa,void *data)
{
  u_int32_t id=0;
  struct nfqnl_msg_packet_hdr *header;

  header=nfq_get_msg_packet_hdr(nfa);
  id = ntohl(header->packet_id);

  if(drand48()<loss_ratio) {
    printf("packet drop!\n");
    return nfq_set_verdict(queue,id,NF_DROP,0,NULL);
  }
  return nfq_set_verdict(queue,id,NF_ACCEPT,0,NULL);
}


int main(int argc,char *argv[])
{
  struct nfq_handle *nfqh;
  struct nfq_q_handle *queue;
  struct nfnl_handle *nh;
  int fd;
  int rv;
  char buf[4096] __attribute__ ((aligned));
  double percent;

  //
  // Configure Random Number Generator
  //
  if(argc!=2) {
    fprintf(stderr,"netfilter <%% packet loss>\n");
    exit(256);
  }
  srand48(time(NULL));
  sscanf(argv[1],"%lf",&percent);
  if((percent<0.0)||(percent>100.0)) {
    fprintf(stderr,"netfilter: packet loss must be between 0.0 and 100.0\n");
    exit(256);
  }
  loss_ratio=percent/100.0;

  //
  // Initialize Library
  //
  if((nfqh=nfq_open())==NULL) {
    fprintf(stderr,"netfilter: error opening library handle\n");
    exit(256);
  }

  //
  // Bind to AF_INET
  //
  nfq_unbind_pf(nfqh,AF_INET);
  if(nfq_bind_pf(nfqh,AF_INET)<0) {
    fprintf(stderr,"netfilter: unable to bind to AF_INET\n");
    exit(256);
  }

  //
  // Bind to Queue
  //
  if((queue=nfq_create_queue(nfqh,0,callback,NULL))==NULL) {
    fprintf(stderr,"netfilter: unable to bind to queue\n");
    exit(256);
  }

  //
  // Set COPY_PACKET Mode
  //
  if(nfq_set_mode(queue,NFQNL_COPY_PACKET,0xffff)<0) {
    fprintf(stderr,"netfilter: unable to set COPY_PACKET mode\n");
    exit(256);
  }

  //
  // Main Loop
  //
  nh = nfq_nfnlh(nfqh);
  fd = nfnl_fd(nh);
  
  while ((rv=recv(fd,buf,sizeof(buf),0))&&rv>=0) {
    nfq_handle_packet(nfqh,buf,rv);
  }

  return 0;
}
