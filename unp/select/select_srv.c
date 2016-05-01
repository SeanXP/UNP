/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < select_srv.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2014/06/14 >
  > Description:        Socket - 多路复用 - select()

  //{{{ 多路复用 - select()
  select可以实现非阻塞,监视多个文件描述符(或多个socket)的状态(读/写/异常)并进行处理;
  在读写socket时, 先使用select检查是否可读写, 使得程序性能更好;

  #include <sys/select.h>
  int select(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout);

  maxfdp, 文件描述符的范围, linux下需设置为[系统文件描述符的最大值+1];
  readfds, 指向fd_set结构的指针, 用于监视文件描述符的读情况;
  writefds, 指向fd_set结构的指针, 用于监视文件表述符的写情况;
  errorfds, 用于监视文件错误异常;
  timeout, 超时时间:
    NULL, 即不设置超时时间, 将select()置于阻塞状态, 直到某文件描述符发生变化.
    0, 纯粹的非阻塞函数, 查询文件描述符的状态，并立即返回.
    正数,select()函数在timeout时间内阻塞等待.

  返回: 成功返回正数, 表示fd_set中对应位为1的fd数量;
        为0表示没有文件可读写;
        负值表示select错误.
  调用select()后, 会检查fd_set中标记的文件描述符, 并将满足条件的位置1, 其他复位为0.
  //}}}

  //{{{ struct fd_set
  #include <sys/select.h>
  struct fd_set为一个集合(每一位对应一个描述符), 存放文件描述符(file descriptor).

  操作宏定义:
  FD_ZERO(fd_set *fdset);               // clear all bits in fdset
  FD_SET(int fd, fd_set *fdset);        // turn on the bit for fd in fdset
  FD_CLR(int fd, fd_set *fdset);        // turn off the bit for fd in fdset
  FD_ISSET(int fd, fd_set *fdset);      // is the bit for fd on in fdset?

  unix的<sys/select.h>定义常量FD_SETSIZE, 是数据类型fd_set的描述字数量, 通常为1024, 即可表示fd < 1024的描述符;
  设置maxfdp为max_fd + 1, 以提高效率, 使得select()不必检查1024位.

  //}}}

 ****************************************************************/
#include <stdio.h>
#include <string.h>             //bzero();
#include <sys/select.h>         //select(), struct fd_set;
#include <sys/time.h>           //struct timeval;
#include <netinet/in.h>         //struct sockaddr_in;
#include <stdlib.h>             //exit();
#include <unistd.h>             //read(), write();

#define LISTEN_PORT     9669    //listen port.
#define LISTENQ         10
#define BUFFER_SIZE     256

int main(int argc, char *argv[])
{
    int listenfd, connfd;
    char buffer[BUFFER_SIZE] = {0};         //256字节的接收缓冲区
    int maxfdp = 0;
    struct sockaddr_in server_addr;
    struct fd_set fds;
    struct timeval timeout = {3,0};         //select等待3秒, 3秒轮询;若要select非阻塞就置timeout为0
    /*
       struct timeval
       {
            __time_t tv_sec;        // Seconds.
            __suseconds_t tv_usec;  // Microseconds.
       };
    */
    //{{{ socket connect
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;              // AF_INET, IPv4;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // 通配地址INADDR_ANY == 0, 表示交由内核选择;
    server_addr.sin_port        = htons(LISTEN_PORT);

    if(bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind() error");
        exit(EXIT_FAILURE);
    }

    if(listen(listenfd, LISTENQ) < 0)
    {
        perror("listen() error");
        exit(EXIT_FAILURE);
    }

    if((connfd = accept(listenfd, (struct sockaddr *) NULL, (socklen_t *) NULL)) < 0)
    {
        perror("accept() error");
        exit(EXIT_FAILURE);
    }
    //}}}

    //{{{ select() sockfd
    FD_ZERO(&fds);                  //每次循环都要清空集合，否则不能检测描述符变化
    FD_SET(connfd, &fds);           //添加描述符 (socket)
    maxfdp = connfd + 1;            //设置maxfdp, 描述符最大值加1

    //timeout时间内等待. 检查读情况, 传递NULL至第2,3参数表示不关心写/异常情况;
    switch(select(maxfdp, &fds, NULL, NULL, &timeout))
    {
        case -1: //select错误,返回-1, 此时退出程序
            fprintf(stderr, "select() error");
            exit(EXIT_FAILURE);
            break;
        case 0:    //没有可操作的文件描述符号, 超时, 返回0
            printf("no socket to read.\n");
            break; //再次轮询
        default:
            //select()会更新fds, 将不满足条件(这里是读)的文件描述符去掉.
            if(FD_ISSET(connfd, &fds)) //测试sock是否可读，即是否网络上有数据
            {
                int num = 0;
                printf("select(): fd[%d] can read now!\n", connfd);
                // read()函数读取客户端应答
                // TCP为无记录边界的字节流协议, 由于TCP的分片, 导致服务器应答格式不确定，故放入循环持续读取.
                while((num = read(connfd, buffer, BUFFER_SIZE)) > 0)
                {
                    buffer[num] = 0;                    // null terminate
                    if (fputs(buffer, stdout) == EOF)   //print to stdout
                    {
                        fprintf(stderr, "fputs() error");
                        exit(EXIT_FAILURE);
                    }
                }
                if (num < 0)
                {
                    fprintf(stderr, "read() error");
                    exit(EXIT_FAILURE);
                }
            }
            break;
    }
    //}}}
    return 0;
}
