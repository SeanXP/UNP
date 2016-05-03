fcntl -- file control
====

    #include <fcntl.h>

    int fcntl(int fildes, int cmd, ...);

fcntl()执行描述符控制操作;

cmd:

* F_DUPFD, 复制现有的描述符;
* F_GETFD / F_SETFD, 获得/设置描述符标记;
* F_GETFL / F_SETFL, 获得/设置描述符状态;
* F_GETOWN / F_SETOW,获得/设置异步I/O所有权;
* F_GETLK / F_SETLN, 获得/设置记录锁.

常用用法:

* 非阻塞式I/O:  	`fcntl(fd, F_SETFL, O_NONBLOCK);`
* 信号驱动I/O:	`fcntl(fd, F_SETFL, O_ASYNC);`
* 设置套接字属主: `fcntl(fd, F_SETOWN, pid);`, 指定用于接收SIGIN & SIGURG信号的套接字属主,
进程组id通过提供负值的arg来说明(arg绝对值的一个进程组ID)，否则arg将被认为是进程id;
* 获取套接字属主: `fcntl(fd, F_GETOWN);`, 获取套接字的当前属主, 返回值为PID;
