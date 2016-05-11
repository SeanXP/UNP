IP地址转换
====

## 过时API（只能处理IPv4地址，参数简单）

    #include <arpa/inet.h>

    int inet_aton(const char *cp, struct in_addr *pin);
    in_addr_t inet_addr(const char *cp);

    char * inet_ntoa(struct in_addr in);

* `inet_aton()`，将点分十进制数串`cp`（例如"192.168.1.100"）转换为32-bit网络字节序的二进制值`pin`。成功返回1，否则返回0；
* `inet_addr()`，与`inet_aton()`功能相同，返回二进制值。不过该函数存在一个问题：函数出错返回INADDR_NONE常值（通常为32位全1的值），因此该函数不能处理地址`255.255.255.255`；「该函数如今已被废弃」
* `inet_ntoa`，将32-bit的网络字节序的二进制值`in`转换为点分十进制数串并返回；返回的字符串驻留在静态内存中，因此该函数不可重入。

## 通用API（可以处理IPv4/IPv6地址，参数复杂）

p，表达（presentation），代表ASCII字符地址串（例如"192.168.1.100"，"2001:0db8:85a3:08d3:1319:8a2e:0370:7344"）；
n，数值（numeric），网络字节序的二进制值（32-bit IPv4 or 128-bit IPv6）；

    int inet_pton(int af, const char * src, void * dst);

    const char * inet_ntop(int af, const void * src, char * dst, socklen_t size);

* `af`，地址族协议，`AF_INET` or `AF_INET6`；其他值返回错误，置errno为EAFNOSUPPORT；
* `inet_pton()`尝试转换ASCII字符地址串`src`，并通过`dst`存放二进制结果；成功返回1，失败返回0；
* `inet_ntop()`尝试转换网络字节序的二进制值`src`，并通过`dst`存放ASCII字符地址串，`len`为`dst`缓冲区大小，避免溢出。`len`太小，则返回空指针并置errno为ENOSPC。
