/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < select_srv.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/14 >
    > Last Changed: 
    > Description:		Socket - 多路复用 - select

	select可以实现非阻塞, 可监视多个文件描述符的状态(读/写/异常)并进行处理.
	在读写socket时, 先使用select检查是否可读写, 使得程序性能更好.

int select(int maxfdp,fd_set *readfds,fd_set *writefds,fd_set *errorfds,struct timeval *timeout);
参数: maxfdp, 文件描述符的范围, linux下需设置为[系统文件描述符的最大值+1]
readfds, 指向fd_set结构的指针, 用于监视文件描述符的读情况;
writefds, 指向fd_set结构的指针, 用于监视文件表述符的写情况;
errorfds, 用于监视文件错误异常;
timeout, 超时时间:
	NULL, 即不设置超时时间, 将select()置于阻塞状态, 直到某文件描述符发生变化.
	0, 纯粹的非阻塞函数, 立即返回.
	正数,select()函数在timeout时间内阻塞等待.
返回: 成功返回正数, 表示fd_set中对应位为1的fd数量; 为0表示没有文件可读写, 负值表示select错误.
调用select()后, 会检查fd_set中标记的文件描述符, 并将满足条件的位置1, 其他复位为0.


struct fd_set为一个集合, 存放文件描述符(file descriptor).
操作宏定义: FD_ZERO(fd_set *), 清空集合; FD_SET(int ,fd_set *), 添加至集合; 
			FD_CLR(int ,fd_set*), 删除; FD_ISSET(int ,fd_set* ), 检查是否在集合中;
unix的<sys/select.h>定义常量FD_SETSIZE, 是数据类型fd_set的描述字数量, 通常为1024, 即可表示fd < 1024的描述符;
其中, fd_set常用位矢量表示, 设置某位为1表示对应的fd被添加.
设置maxfdp为max_fd + 1, 以提高效率, 使得select不必检查1024位.

struct timeval为常用的结构体:<time.h>
struct timeval
{
	__time_t tv_sec;        // Seconds. 
	__suseconds_t tv_usec;  // Microseconds.
};


****************************************************************/


#include <stdio.h>
#include <string.h>			//bzero();
#include <sys/select.h> 	//select()
#include <sys/time.h> 		//struct timeval;
#include <netinet/in.h> 	//struct sockaddr_in;
#include <stdlib.h>			//exit();
#include <unistd.h>     	//read(), write();

#define TEST_PORT	9669		//listen port.
#define LISTENQ		10			//可监听处理的最大连接数量
#define BUFFER_SIZE	256

int main() 
{ 
	int listenfd, connfd;
	struct sockaddr_in servaddr;
	struct fd_set fds; 
	struct timeval timeout = {3,0}; 		//select等待3秒, 3秒轮询;若要select非阻塞就置timeout为0
	char buffer[BUFFER_SIZE] = {0}; 				//256字节的接收缓冲区 
	int maxfdp = 0;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(TEST_PORT);
	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	listen(listenfd, LISTENQ);
	connfd = accept(listenfd, (struct sockaddr *) NULL, NULL); //等待客户端连接.

	FD_ZERO(&fds); 				//每次循环都要清空集合，否则不能检测描述符变化 
	FD_SET(connfd, &fds); 		//添加描述符 (socket)
	maxfdp = connfd + 1; 		//设置maxfdp, 描述符最大值加1 

	//timeout时间内等待. 检查读/写情况, 传递NULL至第3参数表示不关心异常情况.
	switch(select(maxfdp, &fds, &fds, NULL, &timeout))
	{ 
		case -1: //select错误,返回-1, 此时退出程序 
			exit(-1);
			break; 
		case 0:	//没有可操作的文件描述符号, 超时, 返回0
			break; //再次轮询 
		default: 
			//select()会更新fds, 将不满足条件(这里是读/写)的文件描述符去掉.
			if(FD_ISSET(connfd, &fds)) //测试sock是否可读，即是否网络上有数据 
			{ 
				int n = 0;
				printf("select(): fd[%d] can RW now!\n", connfd);
				// read()函数读取客户端应答
				// TCP为无记录边界的字节流协议
				// 由于TCP的分片，导致服务器应答格式不确定，故放入循环持续读取.
				while ( (n = read(connfd, buffer, BUFFER_SIZE)) > 0) 
				{
					buffer[n] = 0;    /* null terminate */
					if (fputs(buffer, stdout) == EOF)	//print to stdout.
					{   
						fprintf(stderr, "fputs error");
						exit(1);
					}   
				}   
				if (n < 0)
				{   
					fprintf(stderr, "read error");
					exit(1);
				}  
			}// end if break; 
	}// end switch 

	return 0;
}//end main
