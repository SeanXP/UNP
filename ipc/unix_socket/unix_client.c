/****************************************************************
    Copyright (C) 2014 Sean Guo. All rights reserved.
					      									  
    > File Name:         < unix_client.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/07/06 >
    > Last Changed: 
    > Description:
****************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/un.h>

#define UNIX_SOCKET "/tmp/unix_socket"

int main(int argc,char **argv)
{
	int sockfd;
	struct sockaddr_un addr;
	char path[104] = UNIX_SOCKET;
	int len;

	//客户端创建UNIX域socket(同服务器)
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		printf("create socket failed.\n");
		return -1;
	}
	//客户端调用connect连接服务器
	bzero(&addr, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy((void*)&addr.sun_path, UNIX_SOCKET);  

	if(connect(sockfd,(struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		printf("connect error.\n");
		return 1;
	}
	printf("write messages to echo server.\n");
	do
	{
		char buf[512];
		int n;
		printf("> ");
		fgets(buf, 512, stdin);	//从终端读取出用户的输入, 发送给服务器.

		if(send(sockfd,buf,strlen(buf),0) == -1)
		{
			printf("send error\n");
			break;
		}
		if((n = recv(sockfd,buf,512,0)) <= 0) //接收服务器的应答
		{
			printf("receive error\n");
			break;
		}
		else
		{
			buf[n] = '\0';
			printf("server> %s", buf);
		}
	} while(1);

	close(sockfd);
	return 0;
}
