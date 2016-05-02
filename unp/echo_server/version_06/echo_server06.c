/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < echo_server.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2016/05/02 >
  > Description:       单进程+select() 代替多进程socket;

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

  1. sin_family 即为sa_family
  2. sin_port 为使用的port 编号
  3. sin_addr. s_addr 为IP 地址 sin_zero 未使用.
  参数 addrlen 为sockaddr 的结构长度.

  返回值：成功则返回0, 失败返回-1, 错误原因存于errno 中.

  错误代码：
  1、EBADF 参数sockfd 非合法socket 处理代码.
  2、EACCESS 权限不足
  3、ENOTSOCK 参数sockfd 为一文件描述词, 非socket.
  //}}}

 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>		//exit();
#include <string.h>		//bzero();
#include <sys/socket.h>
#include <netinet/in.h>	//struct sockaddr_in;
#include <time.h>		//time();
#include <arpa/inet.h>  //inet_pton();
#include <unistd.h>     //write();
#include <signal.h>     //signal();
#include <sys/wait.h>   //wait();
#include <errno.h>

#define BUFFER_MAX 	4096	// max text line length
#define LISTENQ     128     // 2nd argument to listen(), max backlog for AF_INET is 128;
#define LISTEN_PORT	9669	// 服务器监听端口

int main(int argc, char **argv)
{
    int i, maxi, maxfd;
    int	listenfd, connfd, sockfd;
    int nready, client_fd[FD_SETSIZE];
    ssize_t num;
    fd_set rset, allset;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    unsigned char buffer[BUFFER_MAX];

    //{{{ socket
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
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
    //}}}
    //{{{ loop for select()
    // maxfd, select 1st parameter;
    maxfd = listenfd;
    // maxi, index into client_fd[] array
    maxi = -1;
    for(i = 0; i < FD_SETSIZE; i++)
        client_fd[i] = -1;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for ( ; ; )
    {
        rset = allset;
        switch((nready = select(maxfd + 1, &rset, NULL, NULL, NULL)))
        {
            case -1:    //select() error
                perror("select() error");
                exit(EXIT_FAILURE);
                break;
            case 0:     //nothing to select
                break;
            default:
                if(FD_ISSET(listenfd, &rset))      // new client connection
                {
                    // 1. accept()
                    addr_len = sizeof(client_addr);
                    if((connfd = accept(listenfd, (struct sockaddr *) &client_addr, (socklen_t *)&addr_len)) < 0)
                    {
                        perror("accept() error");
                        exit(EXIT_FAILURE);
                    }
                    // 2. add connfd to client_fd[];
                    for(i = 0; i < FD_SETSIZE; i++)
                        if(client_fd[i] < 0) // find 1st empty element (-1) to store connfd;
                    {
                        client_fd[i] = connfd;
                        break;
                    }
                    if(i == FD_SETSIZE)
                    {
                        fprintf(stderr, "error: too many clients. ( > %d)\n", FD_SETSIZE);
                        exit(EXIT_FAILURE);
                    }
                    // 3. set new fd_set;
                    FD_SET(connfd, &allset);
                    if(connfd > maxfd)
                        maxfd = connfd;
                    if(i > maxi)
                        maxi = i;
                    if(--nready <= 0)
                        continue;       // no more readable descriptors;
                }
                // check all clients for data
                for(i = 0; i <= maxi; i++)
                {
                    if((sockfd = client_fd[i]) < 0)
                        continue;
                    else if(FD_ISSET(sockfd, &rset))
                    {
                        // sockfd is readable
                        if((num = read(sockfd, buffer, BUFFER_MAX)) < 0)   // error
                        {
                            perror("read() error");
                            exit(EXIT_FAILURE);
                        }
                        else if(num == 0) // EOF
                        {
                            // connection closed by client
                            printf("client fd %d close.\n", sockfd);
                            close(sockfd);
                            FD_CLR(sockfd, &allset);
                            client_fd[i] = -1;
                        }
                        else
                        {
                            if((num = write(sockfd, buffer, num)) < 0)
                            {
                                perror("write() error");
                                exit(EXIT_FAILURE);
                            }
                            if(--nready <= 0) // no more readable descriptors
                                break;
                        }
                    }
                }
                break;
        }
    }//}}}

return 0;
}
