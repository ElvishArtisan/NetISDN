# iprules.sh
#
# (C) Copyright 2008 Fred Gleason <fredg@paravelsystems.com>
#
# This adds/removes NetISDN UDP packet processing from NetFilter queues.
# For use with testing using the 'netfilter' packet loss simulator.
#

IPTABLES=/usr/sbin/iptables
RTP_PORT=5004
QUEUE_NUMBER=0

if [ -z $1 ] ; then
  echo "usage: iprules.sh set|clear"
  exit 256
fi

#
# Initialize the filter rules
# We allow all forwarding by default
#
$IPTABLES -t filter -F INPUT
$IPTABLES -t filter -P INPUT ACCEPT
$IPTABLES -t filter -F OUTPUT
$IPTABLES -t filter -P OUTPUT ACCEPT
$IPTABLES -t filter -F FORWARD 
$IPTABLES -t filter -P FORWARD ACCEPT
$IPTABLES -t nat -F PREROUTING
$IPTABLES -t nat -P PREROUTING ACCEPT
$IPTABLES -t nat -F OUTPUT
$IPTABLES -t nat -P OUTPUT ACCEPT
$IPTABLES -t nat -F POSTROUTING
$IPTABLES -t nat -P POSTROUTING ACCEPT

if [ $1 = "set" ] ; then
  $IPTABLES -t filter -A INPUT -p udp --dport $RTP_PORT -j NFQUEUE --queue-num $QUEUE_NUMBER
fi


# End of iprules.sh
