/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < daytimetcpcli2.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2016/05/10 >
  > Description:        与协议无关(IPv4/IPv6)的客户端程序
    Usage:
        ./daytimetcpsrv &
        ./daytimetcpcli <server IP address>
        killall daytimetcpsrv
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>         //exit();
#include <string.h>         //bzero();
#include <sys/socket.h>     //socket();
#include <netinet/in.h>     //struct sockaddr_in;
#include <arpa/inet.h>      //inet_pton();
#include <unistd.h>         //read(), write(), close();
#include <netdb.h>          // getaddrinfo(), struct addrinfo

#define MAXLINE 4096        /* max text line length */

// 与（主机:hostname, TCP服务:service）建立TCP连接（协议无关），返回sockfd;
int tcp_connect(const char *hostname, const char *service);

int main(int argc, char **argv)
{
    int sockfd, n;
    char recvline[MAXLINE + 1];

    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <hostname/IP> <service/port#>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if((sockfd = tcp_connect(argv[1], argv[2])) == -1)
    {
        fprintf(stderr, "tcp_connect() error");
        exit(EXIT_FAILURE);
    }

    //{{{
    {
        // The getpeername() function returns the address of the peer connected to the specified socket.
        struct sockaddr_storage cliaddr;
        socklen_t len = sizeof(struct sockaddr_storage);
        // int getpeername(int socket, struct sockaddr *address, socklen_t *restrict address_len);
        if(getpeername(sockfd, (struct sockaddr *)&cliaddr, &len) < 0)
        {
            perror("getpeername() error");
            exit(EXIT_FAILURE);
        }
        if(cliaddr.ss_family == AF_INET)
        {
            // sockaddr_storage剩余的空间为用户通明，必须强制类型转换后才可访问其他字段。
            struct sockaddr_in *sockp = (struct sockaddr_in *)&cliaddr;
            char IPstr[INET_ADDRSTRLEN];
            // int inet_ntop(int af, const void * src, char * dst, socklen_t size);
            printf("tcp_connect() connected to %s:%d\n", inet_ntop(sockp->sin_family, (const void *)&sockp->sin_addr, IPstr, INET_ADDRSTRLEN), ntohs(sockp->sin_port));
        }
        else if(cliaddr.ss_family == AF_INET6)
        {
            struct sockaddr_in6 *sockp = (struct sockaddr_in6 *)&cliaddr;
            /*char IPstr[INET_ADDRSTRLEN6];*/       // Mac OS X的<netinet/in.h>中未定义INET_ADDRSTRLEN6...
            char IPstr[46];
            printf("connected to %s:%d\n", inet_ntop(sockp->sin6_family, (const void *)&sockp->sin6_addr, IPstr, 46), ntohs(sockp->sin6_port));
        }
        else
        {
            fprintf(stderr, "getpeername() error, AF_INET:%d, AF_INET6:%d, sockp->sin_family:%d\n", AF_INET6, AF_INET6, cliaddr.ss_family);
        }
    }
    //}}}
    //{{{ read()函数读取服务器应答
    // TCP为无记录边界的字节流协议, 故服务器应答类似如下:"Mon May 26 20:58:40 2003\r\n"
    // 由于TCP的分片，导致服务器应答格式不确定，故放入循环持续读取.
    while((n = read(sockfd, recvline, MAXLINE)) > 0)
    {
        recvline[n] = 0;    /* null terminate */
        if (fputs(recvline, stdout) == EOF)
        {
            fprintf(stderr, "fputs() error");
            exit(EXIT_FAILURE);
        }
    }
    if (n < 0)
    {
        fprintf(stderr, "read() error");
        exit(EXIT_FAILURE);
    }//}}}
    return 0;
}
//{{{ 与（主机:hostname, TCP服务:service）建立TCP连接（协议无关），返回sockfd;
int tcp_connect(const char *hostname, const char *service)
{
    int sockfd;
    int ret;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family     =   AF_UNSPEC;          // 返回适用于指定主机名和服务名的地址, AAAA记录以sockaddr_in6结构返回, A记录返回sockaddr_in结构;
    hints.ai_socktype   =   SOCK_STREAM;        // TCP Socket

    // getaddrinfo() returns zero on success or one of the error codes listed in gai_strerror(3) if an error occurs.
    if((ret = getaddrinfo(hostname, service, &hints, &res)) != 0)
    {
        fprintf(stderr, "tcp_connect() error for %s, %s: %s", hostname, service, gai_strerror(ret));
        return (-1);
    }

    // 遍历res链表的每一个addrinfo结构体，尝试建立TCP连接；
    ressave = res;
    do{
        if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        {
            // socket() error, continue the next addrinfo.
            continue;
        }
        else
        {
            if(connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
                break;      // success
            else
            {
                // connect error, continue the next addrinfo.
                close(sockfd);
                continue;
            }
        }
    }while((res = res->ai_next) != NULL);

    if(res == NULL)
    {
        // print errno (set from final connect() or socket())
        fprintf(stderr, "tcp_connect() error for %s, %s\n", hostname, service);
        return -1;
    }

    freeaddrinfo(ressave);
    return sockfd;
}//}}}
