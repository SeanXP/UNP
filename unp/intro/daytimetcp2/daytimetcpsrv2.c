/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < daytimetcpsrv.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2016/05/10 >
  > Description:        与协议无关(IPv4/IPv6)的daytime服务器;

 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>         //exit();
#include <string.h>         //bzero();
#include <sys/socket.h>
#include <netinet/in.h>     //struct sockaddr_in;
#include <time.h>           //time();
#include <arpa/inet.h>      //inet_pton();
#include <unistd.h>         //write();
#include <netdb.h>          // getaddrinfo(), struct addrinfo

#define MAXLINE     4096    /* max text line length */
#define LISTENQ     128     /* 2nd argument to listen()*/

//tcp_listen: 创建TCP Socket, 绑定服务器的特定端口并开始监听外来连接
int tcp_listen(const char *hostname, const char *service, socklen_t *addrlenp);

int main(int argc, char **argv)
{
    int                     listenfd, connfd;
    char                    buff[MAXLINE];
    time_t                  ticks;

    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <service or port#>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if((listenfd = tcp_listen(NULL, argv[1], NULL)) == -1)
    {
        fprintf(stderr, "tcp_listen() error\n");
        exit(EXIT_FAILURE);
    }
    //{{{ daytime service
    for ( ; ; ) {
        // int accept(int s, struct sockaddr * addr, int * addrlen);
        // accept()用来接受描述符s 的socket连接请求.
        // socket 必需先经bind()、listen()函数处理过,
        // 当有连接请求进来时, accept()会返回一个新的socket 处理代码,  往后的数据传送与读取就是经由新的socket处理,
        //  而原来参数s 的socket 能继续使用accept()来接受新的连线要求.
        // 连线成功时, 参数addr 所指的结构会被系统填入远程主机的地址数据, 参数addrlen 为scokaddr 的结构长度.
        // 成功则返回新的socket 处理代码, 失败返回-1, 错误原因存于errno 中.
        if((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) < 0)
        {
            perror("accept() error");
            exit(EXIT_FAILURE);
        }
        //这里不关心客户端的数据, 因此传递NULL值.

        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        write(connfd, buff, strlen(buff));

        //调用close()只会减少对应socket描述符的引用数, 当引用数为0才会清楚对应的socket.
        close(connfd);
    }//}}}
}

//{{{ tcp_listen: 创建TCP Socket, 绑定服务器的特定端口并开始监听外来连接，协议无关(IPv4/IPv6)；
int tcp_listen(const char *hostname, const char *service, socklen_t *addrlenp)
{
    int listenfd;
    int ret;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags      =   AI_PASSIVE;         // 套接字用于被动打开
    hints.ai_family     =   AF_UNSPEC;          // 返回适用于指定主机名和服务名的地址, AAAA记录以sockaddr_in6结构返回, A记录返回sockaddr_in结构;
    hints.ai_socktype   =   SOCK_STREAM;        // TCP Socket

    // getaddrinfo() returns zero on success or one of the error codes listed in gai_strerror(3) if an error occurs.
    if((ret = getaddrinfo(hostname, service, &hints, &res)) != 0)
    {
        fprintf(stderr, "tcp_connect() error for %s, %s: %s", hostname, service, gai_strerror(ret));
        return (-1);
    }

    // 遍历res链表的每一个addrinfo结构体，尝试创建Socket并绑定特定服务端口；
    ressave = res;
    do{
        if((listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        {
            // socket() error, continue the next addrinfo.
            continue;
        }
        else
        {
            // SO_REUSEADDR允许启动一个监听服务器并捆绑其众所周知端口，即使以前建立的将此端口用做他们的本地端口的连接仍存在。
            const int on = 1;
            if((ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
            {
                fprintf(stderr, "setsockopt() error");
                return (-1);
            }
            if(bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
                break;      // success
            else
            {
                // connect error, continue the next addrinfo.
                close(listenfd);
                continue;
            }
        }
    }while((res = res->ai_next) != NULL);

    if(res == NULL)
    {
        // print errno (set from final connect() or socket())
        fprintf(stderr, "tcp_listen() error for %s, %s\n", hostname, service);
        return -1;
    }

    if(listen(listenfd, LISTENQ) < 0)
    {
        perror("listen() error");
        return -1;
    }

    if(addrlenp)
        *addrlenp = res->ai_addrlen;    // return size of protocol address

    freeaddrinfo(ressave);
    return listenfd;
}//}}}
