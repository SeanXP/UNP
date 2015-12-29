/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < recv_send.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/13 >
    > Last Changed: 
    > Description:			
	
	ssize_t recv(int sockfd, void *buff, size_t nbytes, int flags);
	ssize_t send(int sockfd, const void *buff, size_t nbytes, int flags);
	通过参数flags控制读写数据
	参数: sockfd, socket描述符; buff, 发送/接收数据缓冲区; nbytes, 缓冲区数据大小; flags, 控制参数;

	recv() & send()都是将数据从本地缓冲区buff拷贝至sockfd套接字的缓冲区, 等待发送/接收.
	调用函数成功不代表已经发送/接收成功.

>>>>>>>>>>
	send(); 向已连接的sockfd发送数据, 发送成功, 返回发送数据的总数;发送失败, 返回SOCKET_ERROR;
	成功调用send()不代表数据成功达到, 此端成功发送不表示彼端成功接收.
	1. 检查[发送缓冲区buff]长度的nbytes与[套接字sockfd的发送缓冲区]的长度, 
		若nbytes大, 则不能发送, 返回SOCKET_ERROR;
	2. nbytes < sockfd的缓冲区长度(可以发送), 则检查是否通信协议正在发送sockfd缓冲区的数据, 
		是则等待协议发送完毕(暂时无法修改sockfd发送区的数据);
	3. 现在没有协议发送数据或sockfd的发送缓冲区没有数据(表示可以修改sockfd发送缓冲区数据), 
		则开始比较sockfd发送缓冲区当前剩余的长度和nbytes.
	4.1 nbytes > sockfd发送缓冲区剩余长度, 则等待协议把数据发送完毕(此时不能使用冗余的空间) 
	4.2 nbytes < sockfd发送缓冲区剩余长度, send()将数据拷贝至sockfd发送缓冲区;
	5. send()拷贝完数据即返回. 拷贝成功返回字节数, 失败返回SOCKET_ERROR.


	recv(): 从sockfd中接收数据.
	1. 等待sockfd的[发送缓冲区]数据传输完毕, 若传输出现错误, 则返回SOCKET_ERROR;
	2. 发送缓冲区没有数据以后, recv()会检查sockfd的[接收缓冲区], 等待数据接收完毕;
	3. 数据接收完毕以后, 从sockfd的接收缓冲区拷贝至buff中. 如果buff的长度nbytes较小, 
		则需要拷贝多次. 调用recv()仅表示从sockfd的接收缓冲区拷贝数据, 实际的接收数据由通信协议完成.
	4. recv()返回拷贝的数据数量, 若在拷贝时出错, 返回SOCKET_ERROR;
	5. recv()在等待协议接收数据至sockfd的接收缓冲区时, 
		若被中断, 返回0(没有拷贝到数据), 断网返回SIGPIPE信号。
****************************************************************/


#include <stdio.h>
#include <sys/socket.h> //recv(), send()


int main()
{

    return 0;
}

