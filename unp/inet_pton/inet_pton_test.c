/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < inet_pton_test.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2014/06/05 >
  > Description:       inet_pton()函数用法, Linux下IP地址转换函数, 将IP地址在"点分十进制"和"整数"之间转换

  int inet_pton(int af, const char *src, void *dst);
    af: 地址族
    src:来源地址
    dst:接收转换后的数据

  函数将*src地址转换为in_addr的结构体，并复制在*dst中。
  af = AF_INET,     (IPv4), *src(ddd.ddd.ddd.ddd)
  af = AF_INET6,    (IPv6)

  函数出错返回负数, errno设置为EAFNOSUPPORT.
  如果参数af指定的地址族和src格式不对，函数将返回0。

eg:
    Please input IPv4 address: 127.0.0.1
    inet_pton: 0x100007f (1111111 00000000 00000000 00000001)
    inet_ntop: 127.0.0.1

  ---------------
  inet_ntop, 整数->点分十进制.
  const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt);

  转换网络二进制结构(*src)到ASCII类型的地址(点分十进制, char *dst)，参数的作用和inet_pton相同;
  socklen_t cnt是所指向缓存区dst的大小,避免溢出
  如果缓存区太小无法存储地址的值,则返回一个空指针,并将errno置为ENOSPC
 ****************************************************************/
#include <stdio.h>
#include <arpa/inet.h>      //inet_pton();

int main(int argc, char* argv[])
{
    char IPdotdec[20];  //存放点分十进制IP地址
    struct in_addr s;   // IPv4地址结构体

    // 输入IP地址(点分十进制)
    printf("Please input IPv4 address: ");
    scanf("%s", IPdotdec);

    // 点分十进制->整数IP地址.
    inet_pton(AF_INET, IPdotdec, (void *)&s);
    printf("inet_pton: 0x%x\n", s.s_addr); // 注意得到的字节序:网络字节顺序(Big-Endian)

    // 反转换(整数IP地址->点分十进制)
    inet_ntop(AF_INET, (void *)&s, IPdotdec, 16);
    printf("inet_ntop: %s\n", IPdotdec);

    return 0;
}
