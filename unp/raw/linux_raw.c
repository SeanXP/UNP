/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < linux_raw.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/15 >
    > Last Changed: 
    > Description:		原始socket编程

Usage: ./raw <source IP> <dest IP>
    需要使用root权限执行此命令.

	socket对TCP和UDP协议做了封装，简化了编程接口，但失去了对IP数据包操作的灵活性;
	原始socket直接针对IP数据包编程，具有更强的灵活性，能够访问ICMP和IGMP数据包;
	原始套接字是一个特殊的套接字类型,它的创建方式跟TCP/UDP创建方法几乎是一摸一样.
	raw socket可以编写基于IP协议的高层协议;

功能差异:
	raw套接字却与TCP/UDP套接字的功能有很大的不同.
	TCP/UDP套接字只能访问传输层以及传输层以上的数据,因为当IP层把数据传给传输层时,下层的数据包头已经被丢掉了.
	而原始套接字却可访问传输层以下的数据,所以使用raw套接字你可以实现上至应用层的数据操作,也可以实现下至链路层的数据操作.

访问TCP/UDP数据包:
	对于UDP/TCP产生的IP数据包,内核不将它传递给任何原始套接字, 而只是将这些数据交给对应的UDP/TCP数据处理句柄
	(如果想要通过raw套接字访问TCP/UDP或其它类型的数据, 调用socket函数创建原始套接字第三个参数应该指定为htons(ETH_P_IP),
	也就是通过直接访问数据链路层来实现.(我们后面的密码窃取器就是基于这种类型的).
	socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP)); //可以创建能直接读取链路层数据的raw socket;

IP数据包:
	对于ICMP和EGP等使用IP数据包承载数据但又在传输层之下的协议类型的IP数据包, 
	内核不管是否已经有注册了的句柄来处理这些数据,都会将这些IP数据包复制一份传递给协议类型匹配的原始套接字.

bind():
	如果原始套接字bind绑定了一个地址,核心只将目的地址为本机IP地址的数包传递给原始套接字,
	如果某个原始套接字没有bind地址,核心就会把收到的所有IP数据包发给这个原始套接字.
connect():
	如果原始套接字调用了connect函数,则核心只将源地址为connect连接的IP地址的IP数据包传递给这个原始套接字.
	如果原始套接字没有调用bind和connect函数,则核心会将所有协议匹配的IP数据包传递给这个原始套接字.

	调用bind函数后,发送数据包的源IP地址将是bind函数指定的地址。
	如是不调用bind，则内核将以发送接口的主IP地址填充IP头.
	设置IP_HDRINCL(header including)选项,就必须手工填充每个要发送的数据包的源IP地址,否则,内核将自动创建IP首部.

	调用connect函数后,就可以使用write和send函数来发送数据包,
	而且内核将会用这个绑定的地址填充IP数据包的目的IP地址,
	否则的话,则应使用sendto或sendmsg函数来发送数据包,并且要在函数参数中指定对方的IP地址。
****************************************************************/

#include <stdio.h>
#include <sys/socket.h>     //linux中包含<bits/socket.h>
//linux - <bits/socket.h>中含有PF_PACKET的定义.
//unix(Mac OS X)中, sys/socket.h实现了PF_*, AF_*的定义，但是没有PF_PACKET.

#include <linux/if_ether.h>     //ETH_P_IP 
//#include <netinet/if_ether.h> - unix
#include <linux/in.h>   //IPPROTO_xxx宏定义
#include <stdlib.h>     //exit();

#define BUFFER_MAX 2048
#define FULL_MASK	(0xFF)		//全1掩码.

//直接读取链路层数据包的原始套接字,并从中分析出源MAC地址和目的MAC地址,源IP和目的IP,以及对应的传输层协议
//如果是TCP/UDP协议的话,打印其目的和源端口.
//为了方便阅读,程序中避免了使用任何与协议有关的数据结构,如struct ether_header,struct iphdr等
int main(int argc,char *argv[])
{
	int sockfd, read_len, proto;        
	char buffer[BUFFER_MAX];
	char  *ethhead, *iphead, *tcphead, *udphead, *icmphead, *p; //指向各种数据帧头部的指针.

	//linux中提供了PF_PACKET接口可以操作链路层的数据。
	//利用函数sendto和recefrom来读取和发送链路层的数据包。
	//PF_PACKET可接收SOCK_RAW(发送/接收的数据必须包含链路层的协议头)和SOCK_DGRAM,SOCK_PACKET(已废弃不用)
	//Unix下没有找到PF_PACKET的头文件定义.
    //<linux/if_ehter.h> #define ETH_P_IP    0x0800      /* Internet Protocol packet */
	if( (sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0)
	{
		fprintf(stdout, "create socket error/n");
		exit(0);
	}

	while(1) 
	{
        printf("\n\n");
		//没有bind & conncet, 将读取所有符合协议的IP数据包.
		read_len = recvfrom(sockfd, buffer, BUFFER_MAX, 0, NULL, NULL);
		/************		数据帧格式 		**************
		   14   6(dest)+6(source)+2(type or length)			[Ethernet Header, IEEE802.3]
		   +
		   20   IP Header 
		   +
		   8   icmp,tcp or udp header
		   = 42 bytes
		 */
		if(read_len < 42) //不是一个完整的数据帧, 连报头都不全.
		{
			fprintf(stdout, "Incomplete header, packet corrupt.\n");
			continue;
		}

		//打印以太网头部信息, MAC地址;
		//以太网头部是最外面的包装.
		ethhead = buffer;
		p = ethhead;
		//int n = 0xFF;	//掩码, 所有位全为1.
		printf("MAC: %.2X:%02X:%02X:%02X:%02X:%02X==>"
				"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
				p[6]&FULL_MASK, p[7]&FULL_MASK, p[8]&FULL_MASK, p[9]&FULL_MASK, p[10]&FULL_MASK, p[11]&FULL_MASK,
				p[0]&FULL_MASK, p[1]&FULL_MASK, p[2]&FULL_MASK, p[3]&FULL_MASK, p[4]&FULL_MASK, p[5]&FULL_MASK);

		//打印IP头部信息, source IP & dest IP.
		iphead = ethhead + 14; //跳过14字节的Ethernet Header. 
		p = iphead + 12;		//跳过IP首部前12字节, 接下来为4字节Source IP & 4字节dest IP.
		printf("IP: %d.%d.%d.%d => %d.%d.%d.%d\n",
				p[0]&FULL_MASK, p[1]&FULL_MASK, p[2]&FULL_MASK, p[3]&FULL_MASK,
				p[4]&FULL_MASK, p[5]&FULL_MASK, p[6]&FULL_MASK, p[7]&FULL_MASK);

		proto = (iphead + 9)[0];	//IP首部的第10字节处的协议Protocol
		p = iphead + 20;			//跳过IP头部, 指向更高层的数据帧.
		printf("Protocol: ");
		switch(proto)
		{
			case IPPROTO_ICMP: printf("ICMP\n");break;
			case IPPROTO_IGMP: printf("IGMP\n");break;
			case IPPROTO_IPIP: printf("IPIP\n");break;

							   //若为TCP/UDP, 还可继续分析出更高层数据帧的信息, 端口信息.
			case IPPROTO_TCP :
			case IPPROTO_UDP :
							   printf("%s,", proto == IPPROTO_TCP ? "TCP": "UDP"); 
							   //端口号有两个字节, 需要移位操作.
							   printf("source port: %u,",(((int)(p[0])<<8)&0XFF00)|(p[1]&FULL_MASK));
							   printf("dest port: %u\n", (((int)(p[2])<<8)&0XFF00)|(p[3]&FULL_MASK));
							   break;
			case IPPROTO_RAW : printf("RAW\n");break;
			default:printf("Unkown, please query in include/linux/in.h\n");
		}
	}
}
