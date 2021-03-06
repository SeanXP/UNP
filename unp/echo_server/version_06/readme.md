echo server by TCP Socket API
====

实现最简单的echo server.

    $ cd unp/echo_server/version_01/
    $ make
    $ make test

----

###version 01 - issue: 子进程僵死问题

其中在echo_server中没有处理子进程, 因此会导致僵死状态的子进程：

客户端连接时，服务器会创建子进程负责处理客户端

当客户端关闭后，子进程结束(向父进程发送SIGCHLD信号, 等待父进程处理; 子进程进入僵死状态,维护部分子进程的信息, 以便父进程以后某个时间获取)

然而此版本的echo server, 会导致很多的僵死子进程，这样占用了过多内核的空间。

在version_02的echo server中将解决这个问题(信号处理函数).

###version 02 - issue: 多个子进程同时结束

version-02使用signal()配置了SIGCHLD的信号处理函数, 并在函数中, 使用wait()等待刚才发送SIGCHLD信号的子进程。
经过测试, 版本2解决了僵死子进程的问题。

但是，由于wait()只执行一次，如果同时有多个子进程结束，同时想服务器发送SIGCHLD信号，则信号处理函数只执行一次(且UNIX信号一般是不排队的), 这样又将导致多个僵死子进程的产生。

解决方案(version-03)：使用while循环内嵌套waitpid()函数。其中waitpid()函数使用选项WNOHANG实现非阻塞。

###version 03

    //信号处理函数, 处理所有子进程的结束信号SIGCHLD;
    void sig_child03(int signo)
    {
        pid_t pid;
        int state;

        while((pid = waitpid(-1, &state, WNOHANG)) > 0)	//使用非阻塞的waitpid等待可结束的所有子进程
            printf("child pid[%d] terminated.\n", pid);
    }

issue: I/O Multiplexing (I/O复用)

当客户端进程阻塞于(标准输入)的fget()调用期间，如果服务器进程被杀死，则:

1. 服务器进程(主动关闭-FIN_WAIT_1)发送FIN至客户端，客户端(被动关闭-CLOSE_WAIT)回应ACK，服务器端完成TCP半关闭(FIN_WAIT_2);
2. 由于客户端进程阻塞于fget()系统调用中，正等待stdin的用户输入，则客户端暂时无法处理当前Socket的关闭请求, 则服务器将长期处于(FIN_WAIT_2), 客户端处于(CLOSE_WAIT)状态;
3. 客户端从fget()中返回，读写socket时才会发现socket已半关闭;
4. 客户端进程关闭连接, 发送FIN至服务器端, 服务器端(TIME_WAIT)发送ACK至客户端，客户端完成TCP关闭(CLOSED);

想要进程立刻得知socket已关闭(即使在该进程阻塞于其他系统调用时), 则要用到I/O复用功能。

###version 04 - I/O multiplexing - select()

使用select()持续检查stdin & socket的状态:

* 用户在stdin输入数据/EOF, 则stdin变得可读，由select()跳入stdin分支，读取stdin的输入发送至socket / EOF结束stdin输入;
* 服务器端发送回显数据, socket变为可读，由select()跳入socket分支, 读取Socket的数据(read返回一个大于0的值)并显示到stdout;
* 服务器端退出，发送FIN，socket仍变为可读，由select()跳入socket分支，读取Socket的数据(read返回0/EOF), 结束客户端进程;
* 服务器端意外退出，客户端发送数据后返回RST，则socket仍变为可读，由select()跳入socket分支，读取Socket的数据(read返回-1), 结束客户端进程;

select()巧妙解决了多个文件描述符(这里是标准输入stdin与网络socket)的读/写/异常检测问题.

issue:

* 用户在stdin输入数据EOF后，stdin变得可读，由select()跳入stdin分支，检测为EOF则直接结束函数，返回main()，而main()随后也直接终止，即socket会被close()掉。
* 问题在于，EOF仅仅代表完成了套接字的输出结束，不代表输入也结束，可能仍有数据在去往服务器的路上，或者仍有应答在返回客户端的路上。
* 因此不能在stdin EOF后直接结束socket; 这里需要一种半关闭TCP连接的方法，即shutdown();

###version 05 - shutdown()

    #include <sys/socket.h>
    int shutdown(int socket, int how);

how:
* SHUT_RD (0) 关闭sockfd的读功能, 不再接受新数据，且丢弃套接字接收缓冲区的现有数据;
* SHUT_WR (1) 关闭sockfd的写功能(TCP half-close), 发送完缓冲区的现有数据，后发送TCP的FIN终止序列;
* SHUT_RDWR (2) 关闭sockfd的读写功能。

返回: 成功返回0, 失败返回-1, 设置错误码errno;

errno: EBADF(无效sockfd), ENOTCONN(sockfd未连接), ENOTSOCK(非socket描述符)

禁止在一个套接口上进行数据的接收与发送, 但不关闭socket, 不释放占用资源;

累计效果: 先调用SHUT_RD, 再调用SHUT_WR, 实现的效果为SHUT_RDWR.


**shutdown() & close()**

close():
* 标记socket为关闭, 本进程中, 关闭的sockfd不可再使用, 协议等待发送队列中的数据后结束通信;
* 减少sockfd的引用, 其他进程可能仍在使用;引用为0才释放资源;
* close()同时终止读/写两个方向的数据传送;

shutdown():
* 关闭sockfd的所有的连接(读/写), 其他进程亦会被干扰.
* shutdown()可以关闭一半TCP连接(通过SHUT_WR关闭单方向的写);

###version 06 - 使用select()处理多个socket, 而非为每个socket都fork子进程;

使用数组clientfd[] 存储所以连接的sockfd; clientfd[]起初被初始化为-1, 表示无连接; 使用select()监测所以的客户端sockfd;

issue:

当客户端发送一个字节的数据（非换行符，即未终结stdin的输入）, 则select()跳转到stdin分支等待stdin结束输入，若客户端进入睡眠，则服务器会阻塞在read()调用中，
无法为其他客户端提供服务。

**当一个服务器在处理多个客户时，决定不能阻塞于只于单个客户端相关的系统调用中。否则可能导致服务器被挂起，无法为所以其他客户提供服务(拒绝服务攻击: denial of service);**

### Socket Notes
1. 当fork子进程时，必须捕获SIGCHLD信号;
2. 当捕获信号时，必须处理被中断的系统调用；
3. SIGCHLD信号处理函数必须正确编写，使用waitpid函数以免留下僵死进程。
