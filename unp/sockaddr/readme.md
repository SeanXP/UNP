套接字地址结构
====

结构体定义所在的头文件：

    <netinet/in.h>

## 通用套接字地址 sockaddr

    struct sockaddr
    {
        unsigned short int sa_family;
        char sa_data[14];
    };

**sizeof(struct sockaddr) => 16字节**

为了套接字接口函数的通用性，设计通用套接字接口，不同协议的套接字地址以指针形式传参。
要求对这些套接字函数调用时必须要将执行特定协议的套接字地址结构（例如IPv4的sockaddr_in）的指针进行强制转换。

    int connect(int socket, const struct sockaddr *address, socklen_t address_len);

    struct sockaddr_in server_addr;
    // ...
    connect(sockfd, (const struct sockaddr*) &server_addr, sizeof(server_addr));

## IPv4 套接字地址结构 sockaddr_in

    struct in_addr
    {
        uint32_t s_addr;        // 32-bit IPv4 address
                                // network byte ordered
    };

    struct sockaddr_in
    {
        unsigned short int  sin_family;         // AF_INET
        uint16_t            sin_port;           // 16-bit TCP or UDP port number
                                                // network byte ordered
        struct in_addr      sin_addr;           // 32-bit IPv4 address
                                                // network byte ordered
        unsigned char       sin_zero[8];        // unused
    };

**sizeof(struct sockaddr_in) => 16字节**

之所以使用`struct in_addr`是由于历史原因，早期UNIX将in_addr结构定义为多种结构的联合(union)，如今大多系统已废除该联合，而是直接定义为in_addr_t字段（32位无符号整形）的结构。

## IPv6 套接字地址结构 sockaddr_in6

    struct in6_addr
    {
        uint8_t s6_addr[16];    // 128-bit IPv6 address
                                // network byte ordered
    };

    struct sockaddr_in6
    {
        unsigned short int  sin6_family;        // AF_INET6
        uint16_t            sin6_port;          // 16-bit TCP or UDP port number
                                                // network byte ordered
        uint32_t            sin6_flowinfo;      // flow information, undefined
        struct in6_addr     sin6_addr;          // 128-bit IPv6 address
                                                // network byte ordered
        uint32_t            sin6_scope_id;      // set of interfaces for a scope
    };

* sin6_flowinfo字段分为两个字段，低序20位是游标（flow label），高序12位保留。
* 对于具备范围的地址（scoped address），sin6_scope_id 字段标识其范围（scope），最常见的是链路局部地址（link-local address）的接口索引（interface index）；

**sizeof(struct sockaddr_in6) => 28字节**

## 新的通用套接字地址结构 sockaddr_storage
sockaddr_storage结构足以容纳系统所支持的任何套接字地址结构。

    struct sockaddr_storage
    {
        uint8_t             ss_len;             // length of this struct (implementation dependent)
        unsigned short int  ss_family;          // address family: AF_xxxx value

        // implementation-dependent elements to provide:
        //      a) 要满足任何套接字地址结构的对齐要求
        //      b) struct sockaddr_storage足够大，能够容纳系统之处的任何套接字地址结构。
    };

**sizeof(struct sockaddr_storage) => 足够大**

除了ss_len和ss_len以外，其他字段对用户来说是透明的。struct sockaddr_storage结构必须强制类型转换成适合与ss_family字段对应地址类型的套接字地址结构，才能访问其他字段。

    struct sockaddr_storage cliaddr;
    // ...

    if(cliaddr.ss_family == AF_INET)
    {
        struct sockaddr_in *sockp = (struct sockaddr_in *) &cliaddr;
        // ...
    }
    else if(cliaddr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *sockp = (struct sockaddr_in6 *) &cliaddr;
        // ...
    }
