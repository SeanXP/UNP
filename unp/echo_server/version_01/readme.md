echo server by TCP Socket API
====

实现最简单的echo server.

    $ cd unp/echo_server/version_01/
    $ make
    $ make test

----

###issue: 子进程僵死问题

其中在echo_server中没有处理子进程, 因此会导致僵死状态的子进程：

客户端连接时，服务器会创建子进程负责处理客户端

当客户端关闭后，子进程结束(向父进程发送SIGCHLD信号, 等待父进程处理; 子进程进入僵死状态,维护部分子进程的信息, 以便父进程以后某个时间获取)

然而此版本的echo server, 会导致很多的僵死子进程，这样占用了过多内核的空间。

在version_02的echo server中将解决这个问题(信号处理函数).
