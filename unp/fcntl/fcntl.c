/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < fcntl.c >
  > Author:            < Shawn Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2014/06/14 >
  > Description:        fcntl -- file control
 ****************************************************************/


#include <stdio.h>            //perror();
#include <fcntl.h>            //fcntl();
#include <sys/socket.h>        //socket();
#include <stdlib.h>            //exit();
#include <string.h>            //bzero();
#include <netinet/in.h>        //struct sockaddr_in;
#include <unistd.h>            //read(), write();

#define SERVPORT 3333        //server port.
#define LISTENQ 10
#define BUFFER_SIZE 100

//宏开关, 通过注释对比fcntl()的效果.
/*#define SET_NONBLOCK_FLAG*/

int main()
{
    struct sockaddr_in server_sockaddr, client_sockaddr;
    int recvbytes, flags;
    socklen_t sin_size;
    int listenfd, client_fd;
    char buf[BUFFER_SIZE];

    //{{{ socket
    /*创建socket*/
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    /*设置sockaddr结构*/
    bzero(&server_sockaddr, sizeof(server_sockaddr));
    server_sockaddr.sin_family      = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sockaddr.sin_port        = htons(SERVPORT);

    /*将本地ip地址绑定端口号*/
    if(bind(listenfd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) < 0)
    {
        perror("bind() error");
        exit(EXIT_FAILURE);
    }

    /*监听*/
    if(listen(listenfd,LISTENQ) < 0)
    {
        perror("listen() error");
        exit(EXIT_FAILURE);
    }
    //}}}

    //{{{ fcntl(), 处理多路复用I/O
    // 1. 获取文件标志
    if((flags = fcntl(listenfd, F_GETFL, 0)) < 0)
    {
        perror("F_GETFL error");
        exit(EXIT_FAILURE);
    }

    // 2. 配置文件标志
#ifdef SET_NONBLOCK_FLAG
    flags |= O_NONBLOCK;                //配置O_NONBLOCK, 非阻塞式I/O;
    // flags &= ~O_NONBLOCK;            //取消O_NONBLOCK
#endif

    // 3. 设置文件标志到描述符
    if(fcntl(listenfd, F_SETFL,flags) < 0)
    {
        perror("F_SETFL error");
        exit(EXIT_FAILURE);
    }
    //}}}

    //{{{ accept() and test
    sin_size = sizeof(struct sockaddr_in);
    if( (client_fd = accept(listenfd, (struct sockaddr*)&client_sockaddr, &sin_size)) < 0)
    {
        perror("accept() error");
        exit(EXIT_FAILURE);
    }
    //由于已配置socket为O_NONBLOCK, 因此如果没有可连接的socket, accept将不会阻塞,而是直接返回结果.
    //若没有配置O_NONBLOCK, 则accept()将一直阻塞... 不会执行下面的内容;

    if((recvbytes = recv(client_fd, buf, BUFFER_SIZE, 0)) < 0)
    {
        perror("recv() error");
        exit(EXIT_FAILURE);
    }
    if(read(client_fd, buf, BUFFER_SIZE) < 0)
    {
        perror("read() error");
        exit(EXIT_FAILURE);
    }

    printf("received a connection :%s",buf);
    //}}}

    close(client_fd);
    return 0;
}
