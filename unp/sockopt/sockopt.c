/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < sockopt.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/14 >
    > Last Changed: 
    > Description:		配置套接字选项 - getsockopt() / setsockopt()

	配置套接字选项
	int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
	int setsockopt(int sockfd, int level, int optname, void *optval, sock_len optlen);
	参数:	level, 选项所在的协议层级别, SOL_SOCKET(通用socket选项), SOL_TCP(TCP);
			optname, 选项名称.
			optval, 选项值.
			optlen, 选项值长度.
		
	返回: 成功返回0, 失败返回-1并配置errno;

常用选项组合:
SOL_SOCKET	  SO_REUSEADDR
SOL_SOCKET    SO_KKEPALIVE
SOL_SOCKET    SO_TYPE
SOL_SOCKET    SO_ERROR

SOL_TCP       SO_NODELAY

****************************************************************/

#include <stdio.h>
#include <sys/socket.h>	//socket();
#include <stdlib.h>		//exit();
#include <string.h>		//strerror();
#include <errno.h>		//errno
#include <unistd.h>		//close();

int main()
{
	int sockfd, return_val;
	int sndbuf_size = 0;        /* Send buffer size */
	int rcvbuf_size = 0;        /* Receive buffer size */
	int so_type = 0;			//socket type
	socklen_t optlen;        /* Option length */


	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Error: failed to create a socket.");
		fputs(strerror(errno), stderr);
		exit(1);
	}
	printf("create a socket, fd = %d\nuse getsockopt() to get something...\n", sockfd);

	//读取socket中的SO_SNDBUF选项, 发送缓冲区大小.
	optlen = sizeof(sndbuf_size);
	if( (return_val = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, &optlen)) < 0)
	{
		fprintf(stderr, "Error: falied to get socket option.");
		fputs(strerror(errno), stderr);
		exit(1);
	}
	printf("get SO_SNDBUF: %d\n", sndbuf_size);

	//读取SO_RCVBUF选项值
	optlen = sizeof(rcvbuf_size);
	if( (return_val = getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, &optlen)) < 0)
	{
		fprintf(stderr, "Error: falied to get socket option.");
		fputs(strerror(errno), stderr);
		exit(1);
	}
	printf("get SO_RCVBUF: %d\n", rcvbuf_size);

	printf("use setsockopt() to set SO_SNDBUF option.\n");
	
	//设置SO_SNDBUF选项值.
	sndbuf_size = 5000;
	optlen = sizeof(sndbuf_size);
	if( (return_val = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, optlen)) < 0)
	{
		fprintf(stderr, "Error: falied to set socket option.");
		fputs(strerror(errno), stderr);
		exit(1);
	}
	printf("set SO_SNDBUF: %d\n", sndbuf_size);

	//检查SO_SNDBUF选项值.
	optlen = sizeof(sndbuf_size);
	if( (return_val = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sndbuf_size, &optlen)) < 0)
	{
		fprintf(stderr, "Error: falied to get socket option.");
		fputs(strerror(errno), stderr);
		exit(1);
	}
	printf("get SO_SNDBUF: %d\n", sndbuf_size);
	//读取SO_RCVBUF选项值
	optlen = sizeof(rcvbuf_size);
	if( (return_val = getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, &optlen)) < 0)
	{
		fprintf(stderr, "Error: falied to get socket option.");
		fputs(strerror(errno), stderr);
		exit(1);
	}
	printf("get SO_RCVBUF: %d\n", rcvbuf_size);

	//读取SO_TYPE选项值
	optlen = sizeof(so_type);
	if( (return_val = getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &so_type, &optlen)) < 0)
	{
		fprintf(stderr, "Error: falied to get socket option.");
		fputs(strerror(errno), stderr);
		exit(1);
	}
	printf("get SO_TYPE: %d\n", so_type);

	close(sockfd);
    return 0;
}

