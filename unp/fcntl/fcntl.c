/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < fcntl.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/14 >
    > Last Changed: 
    > Description:		fcntl函数

	int fcntl(int fd, int cmd); 
	int fcntl(int fd, int cmd, long arg); 
	int fcntl(int fd, int cmd, struct flock *lock); 
	控制文件描述符fd, cmd为命令, arg为可选的命令参数.

cmd:
F_DUPFD, 复制现有的描述符;
F_GETFD / F_SETFD, 获得/设置描述符标记;
F_GETFL / F_SETFL, 获得/设置描述符状态;
F_GETOWN / F_SETOW,获得/设置异步I/O所有权;
F_GETLK / F_SETLN, 获得/设置记录锁.

常用用法:
非阻塞I/O:  	可将cmd 设为F_SETFL,将lock设为O_NONBLOCK;
信号驱动I/O:	可将cmd 设为F_SETFL,将lock设为O_ASYNC.

****************************************************************/


#include <stdio.h>			//perror();
#include <fcntl.h>			//fcntl();
#include <sys/socket.h>		//socket();
#include <stdlib.h>			//exit();
#include <string.h>			//bzero();
#include <netinet/in.h>		//struct sockaddr_in;
#include <unistd.h>			//read(), write();

#define SERVPORT 3333		//server port.
#define LISTENQ 10
#define BUFFER_SIZE 100

//宏开关, 通过注释对比fcntl()的效果.
#define SET_NONBLOCK_FLAG

int main()
{
	struct sockaddr_in server_sockaddr, client_sockaddr;
	int recvbytes, flags;
	socklen_t sin_size;
	int listenfd, client_fd;
	char buf[BUFFER_SIZE];
	/*创建socket*/
	if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		exit(1);
	}

	/*设置sockaddr结构*/
	bzero(&server_sockaddr, sizeof(server_sockaddr));
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(SERVPORT);
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*将本地ip地址绑定端口号*/
	if(bind(listenfd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) < 0)
	{
		perror("bind");
		exit(1);
	}

	/*监听*/
	if(listen(listenfd,LISTENQ) < 0){
		perror("listen");
		exit(1);
	}

	/*fcntl()函数，处理多路复用I/O*/
	if( (flags = fcntl(listenfd, F_SETFL, 0)) < 0)	//清空F_SETFL选项
		perror("fcntl F_SETFL");
#ifdef SET_NONBLOCK_FLAG
	flags |= O_NONBLOCK;				//配置O_NONBLOCK
#endif

	if(fcntl( listenfd, F_SETFL,flags)<0)			//使用fcntl配置F_SETFL选项
		perror("fcntl");

	sin_size = sizeof(struct sockaddr_in);
	if( (client_fd = accept(listenfd, (struct sockaddr*)&client_sockaddr, &sin_size)) < 0)
	{  //服务器接受客户端的请求，返回一个新的文件描述符
		perror("accept");
		exit(1);
	}
	//由于已配置socket为O_NONBLOCK, 因此如果没有可连接的socket, accept将不会阻塞,而是直接返回结果.

	if((recvbytes = recv(client_fd, buf, BUFFER_SIZE, 0)) < 0)
	{
		perror("recv");
		exit(1);
	}
	if(read(client_fd, buf, BUFFER_SIZE) < 0)
	{
		perror("read");
		exit(1);
	}

	printf("received a connection :%s",buf);

	close(client_fd);
	exit(1);
}
