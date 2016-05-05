UDP Socket API
====

UDP协议是一种**非连接的、不可靠的数据报文协议**，完全不同于提供面向连接的、可靠的字节流的TCP协议。

* UDP Client程序不和Server程序建立连接，而是直接使用sendto()来发送数据。
* UDP Server程序不需要允许Client程序的连接,而是直接使用recvfrom()来等待, 直到接收到Client程序发送来的数据。

##recvfrom

    #include < sys/socket.h >

    int recvfrom(int sockfd, void *buffer, int length, unsigned int flags, struct sockaddr *from, int *fromlen);

* recvfrom()用来接收远程主机经指定的socket传来的数据,并把数据存到由参数buffer指向的内存空间.
* 参数len 为可接收数据的最大长度。参数flags 一般设0，其他数值定义请参考recv()。
* 参数from用来指定发送数据端的网络地址, 其信息存储与struct sockaddr结构体中。
* 参数fromlen为sockaddr的结构长度。

UDP协议给每个UDP SOCKET设置一个接收缓冲区, 每一个收到的数据报根据其端口放在不同缓冲区。
recvfrom函数每次从接收缓冲区队列取回一个数据报, 没有数据报时将阻塞,
返回值为0表示收到长度为0的空数据报, 不表示对方已结束发送.

    int sendto(int sockfd, const void *buffer, int length, unsigned int flags, const struct sockaddr *to , int tolen);

sendto()用来将数据由指定的socket传给对方主机。

* 参数sockfd为已建好连线的socket描述符, 如果利用UDP协议则不需经过连线操作。
* 参数buffer指向将发送的数据内容, 参数flags 一般设0, 详细描述请参考send()。
* 参数to用来指定欲传送的sockaddr指针地址, 通过bind()与socket描述符绑定。
* 参数tolen为sockaddr的结果长度。

返回：成功返回传送的字符数, 失败返回-1并设置errno.

每次调用sendto都必须指明接收方socket地址, UDP协议没有设置发送缓冲区,
sendto将数据报拷贝到系统缓冲区后返回，通常不会阻塞.
允许发送空数据报，此时sendto返回值为0.

* udp client可以使用read,write代替recvfrom(),sendto();
* udp server必须使用recvfrom(),sendto()函数, 因为udp没有连接, 需要指定对应的struct sockaddr.

##UDP Server特点

1. 服务器不接受客户端连接，只需监听端口
2. 循环服务器，可以交替处理各个客户端数据包，不会被一个客户端独占.

##UDP client - connect()

UDP connect()与TCP connect()不同，没有三次握手，内核仅检查是否存在立即可知的错误，记录对端的IP与端口，然后立刻返回到调用进程。

* 无连接UDP Socket: socket() -> sendto() / recvfrom(); 每次都有指定Socket, 可以不是同一个Socket;
* 已连接UDP Socket: socket() -> connect() -> send() / recv(), 不能在输出操作指定Socket(不使用sendto()或sendto()第五参数为NULL), 默认Socket由connect()函数指定。

sendto和recvfrom在收发时指定地址, 而send和recv则没有, 因此需要提前使用connect()函数.

* connect()在UDP中, 用来检测udp端口的是否开放的、没有被使用的。
* connect()没有向TCP的三次握手, 内核只是检查是否存在立即可知的错误(如目的地不可达等),
记录对端的IP地址和端口号（取自传递给connect的套接口地址结构）,然后立即返回到调用进程。
* 一旦在UDP中调用connect(),则不必再使用recvfrom以获得数据报的发送者，而可使用read，recv或recvmsg, 内核会自动填写IP地址与端口给已connect的socket数据报;
* 已连接的UDP套接口所引发的异步错误将返回给其所在的进程, 未连接的UDP Socket所在进程无法获取异步错误;
* 若在无connect()情况下, 使用非指定地址的函数, 如write()函数发送数据, 将无法发送成功.


未连接(connect())的UDP Socket调用sendto(), 内核暂时连接该Socket, 发送数据报, 然后断开连接;
因此，当应用进程知道自己要给同一Socket发送多个数据报时，**显示连接UDP Socket效率更高**;

## UDP 多次调用connect()

已经调用connect()的UDP Socket由于以下原因再次调用connect():

* 指定新的IP/Port;
* 断开Socket, 通过再次调用connect(), 设置地址族为AF_UNSPEC;
