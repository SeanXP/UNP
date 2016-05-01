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

### Socket Notes
1. 当fork子进程时，必须捕获SIGCHLD信号;
2. 当捕获信号时，必须处理被中断的系统调用；
3. SIGCHLD信号处理函数必须正确编写，使用waitpid函数以免留下僵死进程。
