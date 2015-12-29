/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < raw.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/15 >
    > Last Changed: 
    > Description:
****************************************************************/


#include <stdio.h>
#include <netinet/ip.h> 			//struct ip;
#include <netinet/ip_icmp.h>		//struct icmp;
#include <netinet/in.h> 			//struct sockaddr_in;
#include <stdlib.h>					//exit();
#include <string.h>					//bzero();

#define BUFFSIZE 1024
#define IP_VERSION		4

int main(int argc,char *argv[])
{

	struct icmphdr *icmphdr_ptr;
	struct iphdr *iphdr_ptr;

	unsigned char buff[BUFFSIZE];
	int sockfd;
	struct sockaddr_in mysocket;
	int flag = 1;

	if(argc < 3) 
	{
		fprintf(stderr,"usage: %s source-ip dest-ip\n",argv[0]);
		exit(-1);
	}
	
	//SOCK_RAW, 原始socket; IPPROTO_IP, 接收IP协议的数据包.
	if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_IP)) < 0)
  	{
		perror("socket()");
		exit(-1);
	}

	//设置socket option.
	//flag = 0, 协议自动填充IP首部;
	//flag = 1, 必须手动填写IP首部.
	//设置IP_HDRINCL(header including)选项,就必须手工填充每个要发送的数据包的源IP地址,否则,内核将自动创建IP首部.
	if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, (char *) &flag, sizeof(flag)) < 0)
  	{
		perror("setsockopt()");
		exit(-1);
	}

	iphdr_ptr = (struct iphdr *)buff;
	bzero(&iphdr_prt,sizeof(struct iphdr));
	iphdr->ihl = 5;
	iphdr->version = IPVERSION;
	iphdr->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));
	iphdr->id = htons(getpid());
	iphdr->ttl = 60;
	iphdr->protocol = IPPROTO_ICMP;
	iphdr->saddr = inet_addr(argv[1]);
	iphdr->daddr = inet_addr(argv[2]);
	iphdr->check = in_cksum((unsigned short *)iphdr,sizeof(struct iphdr));
	icmphdr = (struct icmphdr *)(buff +sizeof(struct iphdr));
	bzero((char *)icmphdr,sizeof(struct icmphdr));
	icmphdr->type = ICMP_ECHO;//8
	icmphdr->un.echo.sequence = getpid();
	icmphdr->un.echo.id =  getpid();
	icmphdr->checksum = in_cksum((unsigned short *)icmphdr,
			sizeof(struct icmphdr));

	bzero((char *)&mysocket,sizeof(mysocket));
	mysocket.sin_family = AF_INET;
	mysocket.sin_addr.s_addr = inet_addr(argv[2]);

	if(sendto(sockfd, (char *)buff, sizeof(buff), 0x0,
				(struct sockaddr *)&mysocket,sizeof(mysocket)) < 0)  {
		perror("sendto");
		exit(-1);
	}

	return 0;
}
