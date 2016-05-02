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
#include <sys/select.h>     //select(), struct fd_set;
#include <sys/time.h>       //struct timeval;

#define BUFFER_MAX 4096        // max text line length
#define LISTEN_PORT 9669    //服务器监听端口

// echo client, 读取文本发送至server, 并读回服务器的返回数据，打印出来;
void str_cli(FILE *fp, int sockfd);

//本程序是IPv4协议相关, 故为sockaddr_in & AF_INET.
//IPv4          [ IPv6 ]
//sockaddr_in   [ sockaddr_in6 ]
//AF_INET       [ AF_INET6 ]

int main(int argc, char **argv)
{
    int i, sockfd[5];
    struct sockaddr_in    servaddr;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <Server IP address>\neg. $ ./%s 127.0.0.1\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    //{{{ 创建5个socket连接服务器
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
    //}}}

    //连接5个socket, 但是这里只用1个socket.
    str_cli(stdin, sockfd[0]);

    // 同时有5个socket同时结束, 则服务器会同时收到5个子进程的SIGCHLD信号;
    return 0;
}

//{{{ echo client, 读取文本发送至server, 并读回服务器的返回数据，打印出来;
void str_cli(FILE *fp, int sockfd)
{
    int maxfdp1 = 0;
    fd_set fds;
    char buffer[BUFFER_MAX + 1];

    if(fileno(fp) > sockfd)
        maxfdp1 = fileno(fp) + 1;
    else
        maxfdp1 = sockfd + 1;

    for( ; ; )
    {
        FD_ZERO(&fds);
        FD_SET(fileno(fp), &fds);
        FD_SET(sockfd, &fds);

        switch(select(maxfdp1, &fds, NULL, NULL, NULL))
        {
            case -1:    //select() error
                perror("select() error");
                exit(EXIT_FAILURE);
                break;
            case 0:     //nothing to select
                break;
            default:
                if(FD_ISSET(sockfd, &fds))      // socket is readable
                {
                    //从socket中读取返回数据
                    if(read(sockfd, buffer, BUFFER_MAX) <= 0)
                    {
                        perror("failed to read data from server");
                        exit(EXIT_FAILURE);
                    }
                    //打印服务器回显的数据
                    if(fputs(buffer, stdout) == EOF)
                    {
                        fprintf(stderr, "fputs() error.\n");
                        exit(1);
                    }
                }
                if(FD_ISSET(fileno(fp), &fds))  // input is readable
                {
                    //读取用户在终端的输入, 准备发送至服务器.
                    if((fgets(buffer, BUFFER_MAX, fp) != NULL))
                    {
                        write(sockfd, buffer, strlen(buffer)); //发送至socket发送缓冲区
                    }
                    else    // EOF, input end.
                        return;
                }
                break;
        }
    }
}//}}}
