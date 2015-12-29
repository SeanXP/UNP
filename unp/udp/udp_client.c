/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < udp_client.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/15 >
    > Last Changed: 
    > Description:		A simle UDP client.

	Usage:	./udp_client <port number>

	method 1: socket() -> sendto() / recvfrom(); 无连接的UDP
	method 2: socket() -> connect() -> send() / recv() / sendto() / recvfrom()

	sendto和recvfrom在收发时指定地址, 而send和recv则没有, 因此需要提前使用connect()函数.
	connect()在UDP中, 用来检测udp端口的是否开放的、没有被使用的。
	connect()没有向TCP的三次握手, 内核只是检查是否存在立即可知的错误(如目的地不可达等), 
	记录对端的IP地址和端口号（取自传递给connect的套接口地址结构）,然后立即返回到调用进程。

	一旦在UDP中调用connect(),则不必再使用recvfrom以获得数据报的发送者，而可使用read，recv或recvmsg.
	内核会自动填写IP地址与端口给已connect的socket数据报;
	另外, 已连接的UDP套接口所引发的异步错误将返回给其所在的进程。

	若在无connect()情况下, 使用非指定地址的函数, 如write()函数发送数据, 将无法发送成功.
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

//宏开关: 切换UDP为连接/非连接.
//#define CONNECT_UDP	//使用连接式的UDP

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
	struct sockaddr_in serveraddr;
	socklen_t addr_len;
	char buffer[BUFSIZE];

	if(argc != 2)
	{
		fprintf(stderr, "usage: %s <Server IP address>\neg. $./a.out 127.0.0.1\n", argv[0]);
		exit(1);
	}

	//create socket
	if( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		error("ERROR creating socket");

	//make a struct sockaddr_in serveraddr.
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(PORT);
	if(inet_pton(AF_INET, argv[1], &serveraddr.sin_addr) <= 0)
		error("ERROR in inet_pton().");

#ifdef CONNECT_UDP
	//conncet to server
	//与TCP协议不同，UDP的connect()并没有与服务程序三次握手。
	if(connect(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
		error("CONNECT ERROR");
#endif

	//write 
	while(fgets(buffer, BUFSIZE, stdin) != NULL) //读取用户输入
	{
		/* read a line and send to server */
#ifdef CONNECT_UDP
		//conncet UDP.
		//使用无需指定地址的函数发送数据报。由内核填写IP/Port.
		write(sockfd, buffer, strlen(buffer));		//发送至服务器
#else
		//Unconnected UDP.
		//必须使用指定地址的函数发送数据报, 否则无法发送成功.
		sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));  
#endif
		//接收服务器应答
		if( (len = read(sockfd, buffer, BUFSIZE)) < 0)
			error("read error");

		buffer[len] = 0; /* terminate string */
		fputs(buffer, stdout);	//将服务器应答信息显示给用户.
	}

}
