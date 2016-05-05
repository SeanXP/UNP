/****************************************************************
  Copyright (C) 2016 Sean Guo. All rights reserved.

  > File Name:         < getaddrinfo.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2016/05/05 >
  > Description:        getaddrinfo, freeaddrinfo -- socket address structure to host and service name

//{{{ getaddrinfo()
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    int getaddrinfo(const char *hostname, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
    hostname, 主机名或IP地址(IPv4的点分十进制/IPv6的十六进制串);
    servname, 服务名;
    hints, 可以为NULL, 也可以为addrinfo指针并填入关于期望返回的信息类型的暗示;
    res, 调用成功则返回的addrinfo结构链表, 返回的数据是动态分配的;

    getaddrinfo()解决了把hostname & servname 转换为套接字地址结构(res->ai_addr)的问题;
//}}}

//{{{ freeaddrinfo()
    getaddrinfo()返回的res中都是动态分配空间，调用freeaddrinfo()释放;
    void freeaddrinfo(struct addrinfo *ai);
//}}}

    //{{{ struct addrinfo {
        int ai_flags;           // input flags
        int ai_family;          // protocol family for socket
        int ai_socktype;        // socket type
        int ai_protocol;        // protocol for socket
        socklen_t ai_addrlen;   // length of socket-address
        struct sockaddr *ai_addr; // socket-address for socket
        char *ai_canonname;     // canonical name for service location
        struct addrinfo *ai_next; // pointer to next in list
    };
    //}}}

 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>             // free()
#include <string.h>             // bzero()
#include <arpa/inet.h>          // inet_ntop()
#include <netdb.h>              // getaddrinfo(), struct addrinfo

void print_addrinfo(struct addrinfo *info);

int main(int argc, char* argv[])
{
    struct addrinfo hints, *res;

    bzero(&hints, sizeof(hints));
    hints.ai_flags  =   AI_CANONNAME;       // 返回第一个addrinfo结构的ai_canonname为主机的规范名称CNAME;
    hints.ai_family =   AF_INET;            // 返回AF_INET类型(IPv4);

    getaddrinfo("localhost", "domain", &hints, &res);
    print_addrinfo(res);

    freeaddrinfo(res);
    return 0;
}

void print_addrinfo(struct addrinfo *info)
{
    if(info != NULL)
    {
        printf("\nai_family  : %3d \t(AF_INET:%d, AF_INET6:%d)\n", info->ai_family, AF_INET, AF_INET6);
        printf("ai_socktype: %3d \t(SOCK_DGRAM:%d, SOCK_STREAM:%d)\n", info->ai_socktype, SOCK_DGRAM, SOCK_STREAM);
        printf("ai_protocol: %3d \t(IPPROTO_UDP:%d, IPPROTO_TCP:%d)\n", info->ai_protocol, IPPROTO_UDP, IPPROTO_TCP);
        printf("ai_addrlen: %d\n", info->ai_addrlen);
        if(info->ai_canonname != NULL)
            printf("ai_canonname: %s\n", info->ai_canonname);
        printf("ai_addr(struct sockaddr_in):\n");
        struct sockaddr_in * saddr = (struct sockaddr_in *)info->ai_addr;
        char ipv4[16];
        printf("\tsin_family: %d, sin_port:%d, sin_addr:%s\n",
                saddr->sin_family, saddr->sin_port, inet_ntop(saddr->sin_family, &saddr->sin_addr, ipv4, 16));
        print_addrinfo(info->ai_next);
    }
}
