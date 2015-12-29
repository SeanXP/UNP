/****************************************************************
    Copyright (C) 2014 Sean Guo. All rights reserved.
					      									  
    > File Name:         < unix_server.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/07/06 >
    > Last Changed: 
    > Description:

	UNIX域套接字分为字节流套接字和数据报套接字.

	struct socketaddr_un{

		short int sun_family;	//AF_UNIX
		char sun_path[104];		//文件名的绝对路径
	}； 

	unix域套接字使用的地址通常是文件系统中一个文件路径, 
	服务器调用函数bind绑定一个UNIX域socket时以该路径名创建一个文件, 
	这些文件不是普通的文件,只能作为域套接字通信，不能读写. 
	bind时建立套接口文件，其默认的权限值是0777，并被当前的umask修改，umask是0022 时即文件权限是0755

	非命名UNIX域socket（socketpair）: (类比匿名管道pipe & 命名管道fifo)
	int socketpair(int family, int type, intprotocol, int sockfd[2]);
	这个函数创建两个互相连接的套接字（socketfd[2]）
	family 是AF_LOCAL, type可以是SOCK_STREAM （字节流）或者SOCK_DGRAM（数据报），协议是0(默认选择)
	以SOCK_STREAM调用socketpair函数得到的套接字叫做流管道（stream pipe）, 是全双工的,就是这两个套接字是可读可写的

****************************************************************/


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h> 
#include <unistd.h>

#define UNIX_SOCKET "/tmp/unix_socket"	//unix socket使用一个文件路径名来表示.

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_un addr;

	//指定AF_UNIX协议.
	//UNIX域协议不是真正的网络协议, 提供同一台机器的进程间通信, 双向通道, 有命名与非命名两种.
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		printf("create socket failed.\n");
		return -1;
	}

	//服务器调用bind绑定UNIX域socket和指定的地址
	bzero(&addr, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy((void*)&addr.sun_path, UNIX_SOCKET);  
	unlink(UNIX_SOCKET);	//删除将要创建的文件，否则绑定失败
	bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

	//开启监听
	listen(sockfd,5);

	//服务器调用accept接收客户端连接
	while(1)
	{
		int n;
		int client_sockfd;

		client_sockfd = accept(sockfd, NULL, NULL);
		if(client_sockfd == -1)
		{
			fprintf(stderr, "accept error.\n");
			break;
		}
		do	//得到客户端连接, 开始接收数据.
		{
			char buf[512];

			n = recv(client_sockfd,buf,512,0);
			if(n > 0)
			{
				buf[n] = '\0';
				printf("recv:\n%s\n", buf);
				n = send(client_sockfd,buf,n,0);	//返回同样的数据, echo server.
			}
		} while(n > 0);
		close(client_sockfd);
	}

	close(sockfd);
	return 0;
}
