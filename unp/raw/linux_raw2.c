/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < linux_raw2.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/15 >
    > Last Changed: 
    > Description:		raw socket学习	

	平常用到的网络编程都是在应用层收发数据, 每个程序只能收到发给自己的数据.
	即每个程序只能收到来自该程序绑定的端口的数据。收到的数据往往只包括应用层数据。
	某些情况下我们需要执行更底层的操作，比如监听所有本机收发的数据、修改报头等。

	通过原始套接字, 我们可以抓取所有发送到本机的IP包（包括IP头和TCP/UDP/ICMP包头），
	也可以抓取所有本机收到的帧（包括数据链路层协议头）。
	普通的套接字无法处理ICMP、IGMP等网络报文，而SOCK_RAW可以。利用原始套接字，我们可以自己构造IP头.

	有两种原始套接字, 一种是处理IP层即其上的数据，通过指定socket第一个参数为AF_INET来创建这种套接字。
	另一种是处理数据链路层即其上的数据，通过指定socket第一个参数为AF_PACKET来创建这种套接字。
socket(AF_INET, SOCK_RAW, …:
	当接收包时，表示用户获得完整的包含IP报头的数据包，即数据从IP报头开始算起。
	当发送包时，用户只能发送包含TCP报头或UDP报头或包含其他传输协议的报文.
	IP报头以及以太网帧头则由内核自动加封,除非是设置了IP_HDRINCL的socket选项。
	如果第二个参数为SOCK_STREAM, SOCK_DGRAM，表示接收的数据直接为应用层数据。
PF_PACKET:
	表示获取的数据是从数据链路层开始的数据
	socket(PF_PACKET, SOCK_RAW, htos(ETH_P_IP)):
		表示获得IPV4的数据链路层帧，即数据包含以太网帧头。14+20+(8:udp 或 20:tcp)
		ETH_P_IP: 在<linux/if_ether.h>中定义，可以查看该文件了解支持的其它协议。
	SOCK_RAW, SOCK_DGRAM两个参数都可以使用, 区别在于使用SOCK_DGRAM收到的数据不包括数据链路层协议头.

>>>>>>>>>>>> summary <<<<<<<<<

socket(AF_INET, SOCK_RAW, IPPROTO_TCP | IPPROTO_UDP | IPPROTO_ICMP) , 发送/接收ip数据包:
	能:		该套接字可以接收协议类型为(tcp udp icmp等)发往本机的ip数据包;
	不能:	收到非发往本地ip的数据包(ip软过滤会丢弃非发往本机ip的数据包)
	不能:	收到从本机发送出去的数据包
	发送的话需要自己组织tcp udp icmp等头部.可以setsockopt来自己包装ip头部
	这种套接字用来写个ping程序比较适合

socket(PF_PACKET, SOCK_RAW | SOCK_DGRAM, htons(ETH_P_IP | ETH_P_ARP | ETH_P_ALL)),发送/接收以太网数据帧
这种套接字比较强大，可以监听网卡上的所有数据帧
	能: 接收发往本地mac的数据帧
	能: 接收从本机发送出去的数据帧(第3个参数需要设置为ETH_P_ALL)
	能: 接收非发往本地mac的数据帧(网卡需要设置为promisc混杂模式)

协议类型一共有四个:
ETH_P_IP 	0x800 	只接收发往本机mac的ip类型的数据帧
ETH_P_ARP 	0x806 	只接受发往本机mac的arp类型的数据帧
ETH_P_RARP 	0x8035 	只接受发往本机mac的rarp类型的数据帧
ETH_P_ALL 	0x3 	接收发往本机mac的所有类型ip arp rarp的数据帧,接收从本机发出的所有类型的数据帧.(混杂模式打开的情况下,会接收到非发往本地mac的数据帧)

****************************************************************/

#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/if_packet.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>

//IP首部固定格式, 保证每个元素的顺序与长度.
typedef struct _iphdr //自定义IP首部, 与系统定义相同, 便用分析.
{ 
	unsigned char h_verlen; 	//4位IP版本号+4位首部长度(单位:4字节) 
	unsigned char tos; 			//8位服务类型TOS 
	unsigned short total_len; 	//16位总长度（字节） 
	unsigned short ident; 		//16位标识 
	unsigned short frag_and_flags; //3位标志位 
	unsigned char ttl; 			//8位生存时间 TTL 
	unsigned char proto; 		//8位协议 (TCP, UDP 或其他) 
	unsigned short checksum; 	//16位IP首部校验和 
	unsigned int sourceIP; 		//32位源IP地址 
	unsigned int destIP; 		//32位目的IP地址 
	//不考虑后面的Options + Padding...
} IP_HEADER; 

typedef struct _udphdr //定义UDP首部
{
	unsigned short uh_sport;    //16位源端口
	unsigned short uh_dport;    //16位目的端口
	unsigned int uh_len;		//16位UDP包长度
	unsigned int uh_sum;		//16位校验和
} UDP_HEADER;

typedef struct _tcphdr //定义TCP首部 
{ 
	unsigned short th_sport; 	//16位源端口 
	unsigned short th_dport; 	//16位目的端口 
	unsigned int th_seq; 		//32位序列号 
	unsigned int th_ack; 		//32位确认号 
	unsigned char th_lenres;	//4位首部长度/6位保留字 
	unsigned char th_flag; 		//6位标志位
	unsigned short th_win; 		//16位窗口大小
	unsigned short th_sum; 		//16位校验和
	unsigned short th_urp; 		//16位紧急数据偏移量
}TCP_HEADER; 

typedef struct _icmphdr {  
	unsigned char  icmp_type;  
	unsigned char icmp_code; 	/* type sub code */  
	unsigned short icmp_cksum;  
	unsigned short icmp_id;  
	unsigned short icmp_seq;  
	/* This is not the std header, but we reserve space for time */  
	unsigned short icmp_timestamp;  
}ICMP_HEADER;

void analyseIP(IP_HEADER *ip);
void analyseTCP(TCP_HEADER *tcp);
void analyseUDP(UDP_HEADER *udp);
void analyseICMP(ICMP_HEADER *icmp);


int main(void)
{
	int sockfd;
	IP_HEADER *ip;
	char buf[10240];	//很大的缓冲区, 足以容纳大部分IP数据包.
	ssize_t len;

	/* capture ip datagram without ethernet header */
	if( (sockfd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0)
	{    
		fprintf(stderr, "socket error!\n");
		exit(1);
	}

	//不断读取符合协议(ETH_P_IP)的数据报(SOCK_DGRAM)。
	while (1)
	{
		if( (len = recv(sockfd, buf, sizeof(buf), 0)) < 0)	//阻塞等待接收内容.
		{
			fprintf(stderr, "recv error!\n");
			exit(1);
		}
		else if (n==0)
			continue;

		//接收数据不包括数据链路帧头, 因为为SOCK_DGRAM;
		//若想访问链路帧头信息, 应改为SOCK_RAW.
		ip = (IP_HEADER *)(buf);
		analyseIP(ip);

		//根据IP首部的协议分析决定接下来的分析流程.
		//ip->h_verlen的后4bits为首部长度, 单位为32bits(4字节).
		size_t iplen =  (ip->h_verlen&0x0f)*4;	//计算出IP首部长度.
		if (ip->proto == IPPROTO_TCP)
		{
			TCP_HEADER *tcp = (TCP_HEADER *)(buf +iplen);
			analyseTCP(tcp);
		}
		else if (ip->proto == IPPROTO_UDP)
		{
			UDP_HEADER *udp = (UDP_HEADER *)(buf + iplen);
			analyseUDP(udp);
		}
		else if (ip->proto == IPPROTO_ICMP)
		{
			ICMP_HEADER *icmp = (ICMP_HEADER *)(buf + iplen);
			analyseICMP(icmp);
		}
		else if (ip->proto == IPPROTO_IGMP)
		{
			printf("IGMP----\n");
		}
		else
		{
			printf("other protocol!\n");
		}        
		printf("\n\n");
	}

	close(sockfd);
	return 0;
}

void analyseIP(IP_HEADER *ip)
{
	unsigned char* p = (unsigned char*)&ip->sourceIP;
	printf("Source IP\t: %u.%u.%u.%u\n",p[0],p[1],p[2],p[3]);
	p = (unsigned char*)&ip->destIP;
	printf("Destination IP\t: %u.%u.%u.%u\n",p[0],p[1],p[2],p[3]);

}

void analyseTCP(TCP_HEADER *tcp)
{
	printf("TCP -----\n");
	printf("Source port: %u\n", ntohs(tcp->th_sport));
	printf("Dest port: %u\n", ntohs(tcp->th_dport));
}

void analyseUDP(UDP_HEADER *udp)
{
	printf("UDP -----\n");
	printf("Source port: %u\n", ntohs(udp->uh_sport));
	printf("Dest port: %u\n", ntohs(udp->uh_dport));
}

void analyseICMP(ICMP_HEADER *icmp)
{
	printf("ICMP -----\n");
	printf("type: %u\n", icmp->icmp_type);
	printf("sub code: %u\n", icmp->icmp_code);
}

