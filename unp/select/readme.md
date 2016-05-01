多路复用 - select()
====

    $ cd unp/select
    $ make test

##select()

select可以实现非阻塞,监视多个文件描述符(或多个socket)的状态(读/写/异常)并进行处理;

在读写socket时, 先使用select检查是否可读写, 使得程序性能更好;

    #include <sys/select.h>
    int select(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout);

* maxfdp, 文件描述符的范围, linux下需设置为[系统文件描述符的最大值+1];
* readfds, 指向fd_set结构的指针, 用于监视文件描述符的读情况;
* writefds, 指向fd_set结构的指针, 用于监视文件表述符的写情况;
* errorfds, 用于监视文件错误异常;
* timeout, 超时时间:
    * NULL, 即不设置超时时间, 将select()置于阻塞状态, 直到某文件描述符发生变化;
    * 0, 纯粹的非阻塞函数, 查询文件描述符的状态，并立即返回;
    * 正数,select()函数在timeout时间内阻塞等待;

返回:

* 成功返回正数, 表示fd_set中对应位为1的fd数量;
* 为0表示没有文件可读写;
* 负值表示select错误.

##struct fd_set

调用select()后, 会检查fd_set中标记的文件描述符, 并将满足条件的位置1, 其他复位为0.

struct fd_set为一个集合(每一位对应一个描述符), 存放文件描述符(file descriptor).

操作宏定义:

    #include <sys/select.h>

    FD_ZERO(fd_set *fdset);               // clear all bits in fdset
    FD_SET(int fd, fd_set *fdset);        // turn on the bit for fd in fdset
    FD_CLR(int fd, fd_set *fdset);        // turn off the bit for fd in fdset
    FD_ISSET(int fd, fd_set *fdset);      // is the bit for fd on in fdset?

unix的`<sys/select.h>`定义常量FD_SETSIZE, 是数据类型fd_set的描述字数量, 通常为1024;

    #include <sys/select.h>
    /* Maximum number of file descriptors in `fd_set'.  */
    #define FD_SETSIZE      __FD_SETSIZE

    #include <bits/typesizes.h>
    #define __FD_SETSIZE        1024

设置maxfdp为max_fd + 1, 以提高效率, 使得select()不必检查1024位.
