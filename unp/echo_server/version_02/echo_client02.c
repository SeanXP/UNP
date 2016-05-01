/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < echo_client.c >
  > Author:            < Shawn Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2014/06/19 >
  > Description:
    Usage:
        ./echo_server &
        ./echo_client <server IP address>
        killall echo_server
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>         //exit();
#include <string.h>         //bzero();
#include <sys/socket.h>     //socket();
#include <netinet/in.h>     //struct sockaddr_in;
#include <arpa/inet.h>      //inet_pton();
#include <unistd.h>         //read(), write(), close();
#include <errno.h>          //perror(), errno.

#define BUFFER_MAX 4096        // max text line length
#define LISTEN_PORT 9669    //服务器监听端口

//本程序是IPv4协议相关, 故为sockaddr_in & AF_INET.
//IPv4          [ IPv6 ]
//sockaddr_in   [ sockaddr_in6 ]
//AF_INET       [ AF_INET6 ]

int main(int argc, char **argv)
{
    int i, sockfd[5];
    char buffer[BUFFER_MAX + 1];
    struct sockaddr_in    servaddr;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <Server IP address>\neg. $ ./%s 127.0.0.1\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    //创建5个socket连接服务器, 测试
    for( i = 0; i < 5; i++)
    {
        if((sockfd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            fprintf(stderr, "socket() error");
            exit(EXIT_FAILURE);
        }

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port   = htons(LISTEN_PORT);
        if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        {
            fprintf(stderr, "inet_pton(%s) error", argv[1]);
            exit(EXIT_FAILURE);
        }
        if (connect(sockfd[i], (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        {
            perror("connect() error");
            exit(EXIT_FAILURE);
        }
    }

    //连接5个socket, 但是这里只用1个socket.
    while((fgets(buffer, BUFFER_MAX, stdin) != NULL))       //读取用户在终端的输入, 准备发送至服务器.
    {
        write(sockfd[0], buffer, strlen(buffer)); 		//发送至socket发送缓冲区
        if(read(sockfd[0], buffer, BUFFER_MAX) == 0)		//从socket中读取返回数据
        {
            perror("failed to read data from server");
            exit(EXIT_FAILURE);
        }
        if(fputs(buffer, stdout) == EOF)			//显示返回数据.
        {
            fprintf(stderr, "fputs() error.\n");
            exit(1);
        }
    }

    // 同时有5个socket同时结束, 则服务器会同时收到5个子进程的SIGCHLD信号;
    return 0;
}
