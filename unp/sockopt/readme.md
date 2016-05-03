配置套接字选项 - getsockopt() / setsockopt()
====

    #include <sys/socket.h>
    int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
    int setsockopt(int sockfd, int level, int optname, void *optval, sock_len optlen);

参数:

* level, 选项所在的协议层级别, SOL_SOCKET(通用socket选项), SOL_TCP(TCP), IPPROTO_IP(IPv4);
* optname, 选项名称;
* optval, 选项值;
* optlen, 选项值长度;

返回: 成功返回0, 失败返回-1并配置errno;

-----

##通用套接字选项 SOL_SOCKET

|level|optname|description|
| :---: | :----: | :----: |
|SOL_SOCKET|SO_BROADCAST| 发送广播(只有数据报套接字支持广播), 发送广播前需要设置 |
|SOL_SOCKET|SO_DEBUG| 仅TCP支持, 开启后内核保留分组以供调试 |
|SOL_SOCKET|SO_DONTROUTE| 绕过底层协议的正常路由机制 |
|SOL_SOCKET|SO_ERROR| 获取待处理错误so_error值 |
|SOL_SOCKET|SO_KEEPALIVE| TCP 保活选项(keep-alive), 2小时无数据交换则发送一个keep-alive probe |
|SOL_SOCKET|SO_LINGER| 指定close()如何关闭面向连接的协议TCP/SCTP |
|SOL_SOCKET|SO_OOBINLINE| 开启则带外数据将留在正常的输入队列 |
|SOL_SOCKET|SO_RCVBUF| 接收缓冲区, 可以修改默认大小 |
|SOL_SOCKET|SO_SNDBUF| 发送缓冲区, 可以修改默认大小 |

##IPPROTO_IP

##IPPROTO_TCP
