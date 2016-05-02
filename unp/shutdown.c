/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < shutdown.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2016/05/02 >
  > Description:        Socket - shutdown()

    #include <sys/socket.h>
    int shutdown(int socket, int how);
    how:    SHUT_RD (0) 关闭sockfd的读功能;
            SHUT_WR (1) 关闭sockfd的写功能;
            SHUT_RDWR (2) 关闭sockfd的读写功能。
    返回: 成功返回0, 失败返回-1, 设置错误码errno;
    errno: EBADF(无效sockfd), ENOTCONN(sockfd未连接), ENOTSOCK(非socket描述符)

    禁止在一个套接口上进行数据的接收与发送, 但不关闭socket, 不释放占用资源.
    累计效果: 先调用SHUT_RD, 再调用SHUT_WR, 实现的效果为SHUT_RDWR.

    -----
    shutdown() & close()
    close():    标记socket为关闭, 本进程中, 关闭的sockfd不可再使用, 协议等待发送队列中的数据后结束通信;
                减少sockfd的引用, 其他进程可能仍在使用;引用为0才释放资源;
                close()同时终止读/写两个方向的数据传送;
    shutdown(): 关闭sockfd的所有的连接(读/写), 其他进程亦会被干扰.
                shutdown()可以关闭一半TCP连接(通过SHUT_WR关闭单方向的写);
    -----
    Usage:
        ./echo_server &
        ./echo_client <server IP address>
        killall echo_server
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>         //exit();
#include <string.h>         //bzero();
#include <sys/socket.h>     //socket(),shutdown();
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
    int sockfd;
    char buffer[BUFFER_MAX + 1];
    struct sockaddr_in    servaddr;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <Server IP address>\neg. $ ./%s 127.0.0.1\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    //int socket(int domain, int type, int protocol);
    // domain:  communications domain; PF_INET,PF_INET6,PF_ROUTE,PF_LOCAL,PF_UNIX;
    // type:    SOCK_STREAM,SOCK_DGRAM,SOCK_RAW;
    // protocol: 0 - default protocol for this address family and type; IPPROTO_TCP,IPPROTO_UDP;
    //建立网际(AF_INET == PF_INET)字节流(SOCK_STREAM)套接字 ==> TCP Socket;
    // <bits/socket.h>, #define AF_INET PF_INET
    //返回一个int型的文件描述符, 表示此套接字接口(一般为3开始, 0,1,2对应stdin,stdout,stderr);
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
    servaddr.sin_port   = htons(LISTEN_PORT);
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
    if (connect(sockfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect() error");
        exit(EXIT_FAILURE);
    }

    while((fgets(buffer, BUFFER_MAX, stdin) != NULL))       //读取用户在终端的输入, 准备发送至服务器.
    {
        write(sockfd, buffer, strlen(buffer));              //发送至socket发送缓冲区
        if(read(sockfd, buffer, BUFFER_MAX) == 0)           //从socket中读取返回数据
        {
            perror("failed to read data from server");
            exit(EXIT_FAILURE);
        }
        if(fputs(buffer, stdout) == EOF)                    //显示返回数据.
        {
            perror("fputs() error");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
