/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < daytimetcpsrv.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2014/06/05 >
  > Description:

  //{{{ struct sockaddr
  struct sockaddr
  {
    unsigned short int sa_family;
    char sa_data[14];
  };

  * sa_family 为调用socket()时的domain 参数, 即AF_xxxx 值.
  * sa_data 最多使用14 个字符长度.

  此sockaddr 结构会因使用不同的socket domain 而有不同结构定义,
  例如使用AF_INET domain,其socketaddr 结构定义便为
  struct socketaddr_in
  {
      unsigned short int sin_family;
      uint16_t sin_port;
      struct in_addr sin_addr;
      unsigned char sin_zero[8];
  };

  struct in_addr
  {
      uint32_t s_addr;
  };

  1、sin_family 即为sa_family
  2、sin_port 为使用的port 编号
  3、sin_addr. s_addr 为IP 地址 sin_zero 未使用.
  参数 addrlen 为sockaddr 的结构长度.

  返回值：成功则返回0, 失败返回-1, 错误原因存于errno 中.
  //}}}

 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>         //exit();
#include <string.h>         //bzero();
#include <sys/socket.h>
#include <netinet/in.h>     //struct sockaddr_in;
#include <time.h>           //time();
#include <arpa/inet.h>      //inet_pton();
#include <unistd.h>         //write();

#define MAXLINE     4096    /* max text line length */
#define LISTENQ     128     /* 2nd argument to listen()*/

int main(int argc, char **argv)
{
    int                     listenfd, connfd;
    struct sockaddr_in      servaddr;
    char                    buff[MAXLINE];
    time_t                  ticks;

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);   // 通配地址INADDR_ANY == 0, 表示交由内核选择;
    servaddr.sin_port        = htons(13);           /* daytime server port */

    // int bind(int sockfd, struct sockaddr * my_addr, int addrlen);
    // 为sockfd指定端口(my_addr.sin_port)或IP(my_addr.sin_addr);
    // 此名称由参数my_addr 指向一个sockaddr 结构, 对于不同的socket domain 定义了一个通用的数据结构
    if(bind(listenfd, (const struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind() error");
        exit(EXIT_FAILURE);
    }

    // int listen(int sockfd, int backlog);
    // listen()配置未连接的sockfd为被动socket, 并开始接受该socket的连接;
    // listen()只设置socket 为listen 状态, 真正接收client端连接请求的是accept();
    // 参数backlog 指定同时能处理的最大连接要求, 如果连接数目达此上限则client 端将收到ECONNREFUSED 的错误;
    // 如果socket 为AF_INET 则参数backlog 最大值可设至128;
    // listen()只适用SOCK_STREAM 或SOCK_SEQPACKET 的socket 类型, 不适合UDP(无连接);
    // 成功则返回0, 失败返回-1, 错误原因存于errno.
    if(listen(listenfd, LISTENQ) < 0)
    {
        perror("listen() error");
        exit(EXIT_FAILURE);
    }

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
    }
}
