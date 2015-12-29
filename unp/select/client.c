/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < client.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/14 >
    > Last Changed: 
    > Description:
****************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  //inet_pton();
#include <unistd.h>     //read(), write();

#define MAXLINE 4096	/* max text line length */
#define TEST_PORT   9669        //listen port.

int main(int argc, char **argv)
{
	int					sockfd, n;
	char				buffer[MAXLINE + 1];
	struct sockaddr_in	servaddr;

	if (argc != 2)
	{
		fprintf(stderr, "usage: a.out <Server IP address>\neg. $ ./a.out 127.0.0.1");
		exit(1);
	}
	//建立网际(AF_INET)字节流(SOCK_STREAM)套接字
	//返回一个int型的文件描述符, 代替此套接字接口(一般为3开始, 0,1,2对应stdin,stdout,stderr)
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "socket error");
		exit(1);
	}

	//printf("sockfd = %d\n", sockfd); //文件描述符, 从3以后开始。
	bzero(&servaddr, sizeof(servaddr));	//将servaddr结构体内容清零.

	//对于BSD,是AF,对于POSIX是PF。
	//UNIX系统支持AF_INET，AF_UNIX，AF_NS等，而DOS,Windows中仅支持AF_INET，它是网际网区域。
	//理论上建立socket时是指定协议，应该用PF_xxxx，设置地址时应该用AF_xxxx。
	servaddr.sin_family = AF_INET;		//AF 表示ADDRESS FAMILY 地址族，PF 表示PROTOCOL FAMILY 协议族，但这两个宏定义是一样的

	//设定端口号
	//htons(),将主机的无符号短整形数转换成网络字节顺序。简单说就是高低位字节互换.
	//网络字节顺序: (Big-Endian, 低地址存放高位),保证网络数据传输与CPU,OS无关.
	//这里使用著名端口号Port 13 (DAYTIME协议, 服务器通过TCP/IP,以ASCII字符返回当前日期时间)
	servaddr.sin_port   = htons(TEST_PORT);	/* daytime server */
	//htonl(), htons()... 网络字节(h)转(to)网络字节序(n), l(for long), s(for short)...
	//本质上struct sockaddr_in的内容都要写为网络字节序.
	//因为AF_INET一般被宏定义为0, 故可以不用调用htons().

	//inet_pton, 点分十进制->整数
	//将输入参数argv[1]的点分十进制转为in_addr结构体的整数.
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
	{
		fprintf(stderr, "inet_pton error for %s", argv[1]);
		exit(1);
	}
	// #define SA (struct sockaddr) in "udp.h"
	// struct sockaddr, 通用套接字地址结构.
	// connect(),建立TCP连接
	// 成功返回0, 失败返回-1并设置errno.
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
	{
		fprintf(stderr, "connect error");
		exit(1);
	}
	
	//send data to server.
	snprintf(buffer, sizeof(buffer), "hello, this is client.\n");
	write(sockfd, buffer, strlen(buffer));

	close(sockfd);
	
	return 0;
}
