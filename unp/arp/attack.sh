#!/bin/bash
#################################################################
#   Copyright (C) 2015 Sean Guo. All rights reserved.
#														  
#	> File Name:        < send_arp.sh >
#	> Author:           < Sean Guo >		
#	> Mail:             < iseanxp@gmail.com >		
#	> Created Time:     < 2015/05/13 >
#	> Last Changed: 
#	> Description:
#################################################################

# 10.42.1.224 at 00-E0-66-BE-52-80 (yyy-PC)

COUNTER=0
ip="10.42.1.224"
gateway="10.42.0.1"

while [ $COUNTER -lt 5 ]
do
	sudo ./send_arp $gateway aa:bb:cc:dd:ee:ff $ip 00:E0:66:BE:52:80 1
	sleep 1
	echo 'send arp to ', $ip
done
