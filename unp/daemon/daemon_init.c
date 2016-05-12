/****************************************************************
  Copyright (C) 2016 Sean Guo. All rights reserved.

  > File Name:         < daemon_init.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2016/05/09 >
  > Description:        将普通进程转化为守护进程

  守护进程是后台运行且不与任何控制终端关联的进程。
  守护进程可以由系统初始化脚本启动，也可以由用户的SHELL终端启动。
  后者必须亲自脱离与控制终端的关联，从而避免与作业控制、终端会话管理、终端产生信号等发生不期望的交互。
 ****************************************************************/

#include <stdio.h>
#include <sys/types.h>          // pid_t
#include <unistd.h>             // fork(), _exit(), setsid()
#include <signal.h>             // signal()
#include <fcntl.h>              // open()
#include <syslog.h>             // openlog(), closelog(), syslog()

#define MAXFD 64

// 守护进程化当前进程;
int daemon_init(const char *pname, int facility)
{
    int i;
    pid_t pid;

    if((pid = fork()) < 0)
        return (-1);
    else if(pid > 0)
        _exit(0);           // parent process terminates

    // child process [1], 创建新的会话头进程，从而脱离原SHELL所在的控制终端，脱离原来的终端会话管理;
    //setsid()  creates a new session if the calling process is not a process group leader.
    if(setsid() < 0)        // become session leader
        return (-1);

    // 没有控制终端的会话头进程打开一个终端设备时，该终端会自动称为此会话头进程的控制终端。
    // 再次fork的目的是就是确保新新子进程不再是会话头进程，从而不能自动获得控制终端。
    // 当会话头进程终止时，其会话中的所有进程都会收到SIGHUP信号，因此这里要提前忽略SIGHUP信号。
    signal(SIGHUP, SIG_IGN);
    if((pid = fork()) < 0)
        return (-1);
    else if(pid > 0)
        _exit(0);           // child process [1] terminates

    // child process [2]
    chdir("/");             // change working directory

    // 关闭守护进程从执行它的进程（通常是一个shell终端）继承来的所有打开着的描述符。
    // close off file descriptors, 这里直接关闭前64个（方便）
    for(i = 0; i < MAXFD; i++)
        close(i);
    // redirect stdin, stdout, and stderr to /dev/null
    open("/dev/null", O_RDONLY);        // fd[1] -> stdin to /dev/null, 调用read()将返回0(EOF)；
    open("/dev/null", O_RDWR);          // fd[2] -> stdout to /dev/null, 调用write()将由内核丢弃数据；
    open("/dev/null", O_RDWR);          // fd[3] -> stderr
    // 打开这3个描述符，是避免守护进程所调用的库函数因为系统调用而识别所造成的隐患。

    // 现在该进程是一个守护进程了，这里打开系统日志，以便输入日志消息；
    openlog(pname, LOG_PID, facility);
    return 0;
}
