/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < udp_server.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/15 >
    > Last Changed: 
    > Description:		A simple UDP echo server.

	Usage:	./udp_server <port number>

	UDP协议是一种非连接的、不可靠的数据报文协议，完全不同于提供面向连接的、可靠的字节流的TCP协议。
	通常，UDP Client程序不和Server程序建立连接，而是直接使用sendto()来发送数据。
	同样，UDP Server程序不需要允许Client程序的连接,而是直接使用recvfrom()来等待, 直到接收到Client程序发送来的数据。

#include < sys/socket.h >

int recvfrom(int sockfd, void *buffer, int length, unsigned int flags, 
			struct sockaddr *from, int *fromlen);
recvfrom()用来接收远程主机经指定的socket传来的数据,并把数据存到由参数buf 指向的内存空间.
参数len 为可接收数据的最大长度。参数flags 一般设0，其他数值定义请参考recv()。
参数from用来指定发送数据端的网络地址, 其信息存储与struct sockaddr结构体中。
参数fromlen为sockaddr的结构长度。

UDP协议给每个UDP SOCKET设置一个接收缓冲区, 每一个收到的数据报根据其端口放在不同缓冲区。
recvfrom函数每次从接收缓冲区队列取回一个数据报, 没有数据报时将阻塞, 
返回值为0表示收到长度为0的空数据报, 不表示对方已结束发送.

>>>>>>>>>>>>>>>>>>>>>>>

int sendto(int sockfd, const void *buffer, int length, unsigned int flags,
			const struct sockaddr *to , int tolen);
sendto()用来将数据由指定的socket传给对方主机。
参数sockfd为已建好连线的socket描述符, 如果利用UDP协议则不需经过连线操作。
参数buffer指向将发送的数据内容, 参数flags 一般设0, 详细描述请参考send()。
参数to用来指定欲传送的sockaddr指针地址, 通过bind()与socket描述符绑定。
参数tolen为sockaddr的结果长度。

返回：成功返回传送的字符数, 失败返回-1并设置errno.

每次调用sendto都必须指明接收方socket地址, UDP协议没有设置发送缓冲区, 
sendto将数据报拷贝到系统缓冲区后返回，通常不会阻塞.
允许发送空数据报，此时sendto返回值为0.


udp client可以使用read,write代替recvfrom(),sendto();
udp server必须使用recvfrom(),sendto()函数, 因为udp没有连接, 需要指定对应的struct sockaddr.

>>>>>>>>>>>>>>>>>>>>>>
UDP Server特点:
1. 服务器不接受客户端连接，只需监听端口
2. 循环服务器，可以交替处理各个客户端数据包，不会被一个客户端独占.


****************************************************************/

#include <stdio.h>
#include <stdlib.h>		//exit();
#include <sys/socket.h>	//socket();
#include <netinet/in.h>	//struct sockaddr_in;
#include <netdb.h>      // gethostbyname(); struct hostent;
#include <unistd.h>
#include <string.h>		//bzero();
#include <sys/types.h> 
#include <arpa/inet.h>

#define BUFSIZE 1024
#define PORT	9669

//error() - wrapper for perror
void error(char *msg) 
{
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) 
{
	int sockfd; 
	int len;
	struct sockaddr_in serveraddr, clientaddr;
	socklen_t addr_len;
	char buffer[BUFSIZE];

	//create socket
	if( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		error("ERROR creating socket");

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);

	if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
		error("ERROR on binding");

	while(1)
	{
		bzero(buffer, sizeof(buffer));
		len = recvfrom(sockfd, buffer, sizeof(buffer), 0, 
					(struct sockaddr *) &clientaddr, &addr_len);
		//终端debug: 显示client端的网络地址
		//char * inet_ntoa( struct in_addr in);
		printf("server log: receive from %s\n", inet_ntoa(clientaddr.sin_addr));

		//const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt);
		//printf("server log: receive from %s\n", inet_ntop(AF_INET, (void *) &(clientaddr.sin_addr), buffer, BUFSIZE));

		//将数据返回给client端
		sendto(sockfd, buffer, len , 0, (struct sockaddr *) &clientaddr, addr_len);
	}

}
