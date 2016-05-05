/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < dns_x.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2014/06/13 >
  > Description:        使用 gethostbyaddr函数, 解析IP对应的域名,实现DNS反解析效果.

Usage:    ./dns_x xxx.xxx.xxx.xxx
与linux/unix系统本身的dig -x系统命令对比。
使用dig -x xxx.xxx.xxx.xxx 进行IP反解析, 与./dns_x的测试结果进行比较.

gethostbyaddr在in_addr.arpa域中向一个名字服务器查询PTR记录。
./dns_x 115.239.211.110 会提交一个 110.211.239.115.in-addr.arpa. 查询，但会失败。

nslookup 115.239.211.110
** server can't find 110.211.239.115.in-addr.arpa.: NXDOMAIN
要使用这个函数，本地要有反向解析的服务。

可以dns_x成功的IP, 必须是先nslookup成功的IP;

------

struct hostent *gethostbyaddr(const char *addr,size_t len,int family);
参数: IP地址(网络字节序),地址长度(AF_INET为4), 地址类型, addr参数实际上不是char * 类型，而是一个指向存放IPv4地址的某个in_addr结构的指针
返回值: hostent的结构体指针。如果函数调用失败,将返回NULL。
gethostbyaddr()在in_addr.arpa域中的向一个名字服务器查询PTR记录;

struct hostent
{
char    *h_name;                //主机的规范名(区分主机规范名与主机别名)
char    **h_aliases;            //主机的别名(可以有很多个)
int     h_addrtype;                //主机ip地址的类型(ipv4 -> AF_INET or ipv6 -> AF_INET6)
int     h_length;                //ip地址长度
char    **h_addr_list;             //ip地址(网络字节序存储)
#define h_addr h_addr_list[0]
};

主机规范名(eg. www.l.google.com), 主机别名(eg. www.google.com), 别名是为了便于记忆。
 ****************************************************************/


#include <stdio.h>
#include <stdlib.h>         // exit()
#include <netdb.h>          // gethostbyname(); struct hostent;
#include <arpa/inet.h>      // inet_ntop();

int main(int argc, char *argv[])
{
    //ip_ptr指向主机IP地址的字符串,
    //name_ptr指向返回结构体的主机名称, alias_pptr指向返回结构体中的主机别名列表;
    char *ip_ptr = NULL, *name_str = NULL, **alias_pptr = NULL;
    char **addr_list_pptr = NULL;            //指向返回的结构体中的主机IP地址.
    struct hostent *host_ptr = NULL;
    char ip_str[32];                        //存储解析的IP地址
    struct in_addr ip_addr;

    if(argc != 2)
    {
        fprintf(stderr, "Usage: ./%s host_IP\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    printf("IP:%s\n", argv[1]);
    //将点分十进制转in_addr, int inet_pton(int af, const char *src, void *dst);
    /*inet_pton(AF_INET, (const char *)&argv[1], &ip_addr);*/
    /*printf("inet_pton: 0x%x\n", ip_addr.s_addr); // 注意得到的字节序:网络字节顺序(Big-Endian)*/
    if(inet_aton(argv[1], &ip_addr) == 0)
    {
        perror("inet_aton() error");
        exit(EXIT_FAILURE);
    }

    //addr参数实际上不是char * 类型，而是一个指向存放IPv4地址的某个in_addr结构的指针
    if((host_ptr = gethostbyaddr(&ip_addr, 4, AF_INET)) == NULL) //调用失败返回NULL, 并设置h_errno;
    {
        fprintf(stderr, "gethostbyaddr() error: %s", hstrerror(h_errno));
        exit(EXIT_FAILURE);
    }

    //打印解析结果.
    printf("official hostname(->h_name):%s\n",host_ptr->h_name);
    for(alias_pptr = host_ptr->h_aliases; *alias_pptr != NULL; alias_pptr++)
        printf("alias hostname(->h_aliases):%s\n",*alias_pptr);
    switch(host_ptr->h_addrtype)
    {
        case AF_INET:
        case AF_INET6:
            addr_list_pptr = host_ptr->h_addr_list;
            for(; *addr_list_pptr != NULL; addr_list_pptr++)
                printf("Ip address list:%s\n",
                        inet_ntop(host_ptr->h_addrtype, *addr_list_pptr, ip_str, sizeof(ip_str)));
            //结构体中使用宏定义, 定义了 h_addr, 表示 h_addr_list[0], 解析的第一个IP值.
            printf("first address in addr_list: %s\n",
                    inet_ntop(host_ptr->h_addrtype, host_ptr->h_addr, ip_str, sizeof(ip_str)));
            break;
        default:
            printf("unknown address type(not AF_INET || AF_INET6)\n");
            break;
    }

    return 0;
}
