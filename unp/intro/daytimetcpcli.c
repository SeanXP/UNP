/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < daytimetcpcli.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2014/06/05 >
  > Description:
    Usage:
        ./daytimetcpsrv &
        ./daytimetcpcli <server IP address>
        killall daytimetcpsrv
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>         //exit();
#include <string.h>         //bzero();
#include <sys/socket.h>
#include <netinet/in.h>     //struct sockaddr_in;
#include <arpa/inet.h>      //inet_pton();
#include <unistd.h>         //read(), write(), close();

#define MAXLINE 4096        /* max text line length */

//本程序是IPv4协议相关, 故为sockaddr_in & AF_INET.
//IPv4          [ IPv6 ]
//sockaddr_in   [ sockaddr_in6 ]
//AF_INET       [ AF_INET6 ]

int main(int argc, char **argv)
{
    int                 sockfd, n;
    char                recvline[MAXLINE + 1];
    struct sockaddr_in  servaddr;
    /*
    sockaddr_in 结构会因使用不同的socket domain 而有不同结构定义
    例如使用AF_INET domain, 其socketaddr 结构定义便为struct socketaddr_in
       struct socketaddr_in
       {
           unsigned short int sin_family;
           uint16_t sin_port;
           struct in_addr sin_addr;
           unsigned char sin_zero[8];
       };
    */

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <Server IP address>\neg. $ ./%s 127.0.0.1\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }
    //建立网际(AF_INET)字节流(SOCK_STREAM)套接字 ==> TCP Socket
    //返回一个int型的文件描述符, 表示此套接字接口(一般为3开始, 0,1,2对应stdin,stdout,stderr)
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "socket() error");
        exit(EXIT_FAILURE);
    }

    //printf("sockfd = %d\n", sockfd); //文件描述符, 从3以后开始。
    bzero(&servaddr, sizeof(servaddr));    //将servaddr结构体内容清零.

    //对于BSD是AF, 对于POSIX是PF。
    //UNIX系统支持AF_INET，AF_UNIX，AF_NS等，而DOS,Windows中仅支持AF_INET，它是网际网区域。
    //理论上建立socket时是指定协议，应该用PF_xxxx，设置地址时应该用AF_xxxx。
    servaddr.sin_family = AF_INET;        //AF 表示ADDRESS FAMILY 地址族，PF 表示PROTOCOL FAMILY 协议族，但这两个宏定义是一样的

    //htons()将主机的无符号短整形数转换成网络字节顺序,简单说就是高低位字节互换.
    //网络字节顺序: (Big-Endian, 低地址存放高位),保证网络数据传输与CPU,OS无关.
    //这里使用著名端口号Port 13 (DAYTIME协议, 服务器通过TCP/IP, 以ASCII字符返回当前日期时间)
    servaddr.sin_port   = htons(13);    /* daytime server port */
    //htonl(), htons()... 网络字节(h)转(to)网络字节序(n), l(for long), s(for short)...
    //本质上struct sockaddr_in的内容都要写为网络字节序.
    //因为AF_INET一般被宏定义为0, 故可以不用调用htons().

    //inet_pton, 点分十进制IP("127.0.0.1") -> 整数IP
    //将输入参数argv[1]的点分十进制转为in_addr结构体的整数IP.
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        fprintf(stderr, "inet_pton(%s) error", argv[1]);
        exit(EXIT_FAILURE);
    }

    // struct sockaddr, 通用套接字地址结构.
    // connect(),建立TCP连接
    // 成功返回0, 失败返回-1并设置errno.
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        fprintf(stderr, "connect() error");
        exit(EXIT_FAILURE);
    }

    // read()函数读取服务器应答
    // TCP为无记录边界的字节流协议, 故服务器应答类似如下:"Mon May 26 20:58:40 2003\r\n"
    // 由于TCP的分片，导致服务器应答格式不确定，故放入循环持续读取.
    while ( (n = read(sockfd, recvline, MAXLINE)) > 0)
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
    }

    exit(0);
}
