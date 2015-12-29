/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < shutdown.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/13 >
    > Last Changed: 
    > Description:		Socket - shutdown
	
	禁止在一个套接口上进行数据的接收与发送, 但不关闭socket, 不释放占用资源.
	int shutdown(int sockfd,int how);
	参数: socket描述符, 标志how. 
	返回: 成功返回0, 失败返回-1, 设置错误码errno;
errno: EBADF(无效sockfd), ENOTCONN(sockfd未连接), ENOTSOCK(非socket描述符)
how:	SHUT_RD (0) 关闭sockfd的读功能;
		SHUT_WR (1) 关闭sockfd的写功能;
		SHUT_RDWR (2) 关闭sockfd的读写功能。

累计效果: 先调用SHUT_RD, 在调用SHUT_WR, 实现的效果为SHUT_RDWR.

>>>>>>>> shutdown() & close()
	close(): 标记socket为关闭, 本进程中, 关闭的sockfd不可再使用,协议等待发送队列中的数据后结束通信.
			 减少sockfd的引用, 其他进程可能仍在使用.引用为0才释放资源.
	shutdown(): 关闭sockfd的所有的连接(读/写), 其他进程亦会被干扰.
****************************************************************/


#include <stdio.h>
#include<sys/socket.h>

int main()
{

    return 0;
}

