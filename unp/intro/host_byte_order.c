/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < host_byte_order.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2014/06/06 >
  > Description:        test the host byte order( Little-Endian / Big-Endian )

 ****************************************************************/

#include <stdio.h>

//使用union的特性来判断Host byte order.
//存储数据, 然后用另一个读取方式读取.
union {
    short  s;
    char   c[sizeof(short)];
} un;

int main(int argc, char **argv)
{

    /*Big-Endian    (数据的大端(MSB)存储在起始位置(低地址位))*/
    //Big-Endian:   (0x01, 0x02)
    /*Little-Endian (小端(LSM)存储在起始位置(低地址位))*/
    //Little-Endian:(0x02, 0x01)
    un.s = 0x0102;

    if (sizeof(short) == 2) {
        if (un.c[0] == 1 && un.c[1] == 2)   // 0x01, 0x02
            printf("big-endian\n");
        else if (un.c[0] == 2 && un.c[1] == 1)
            printf("little-endian\n");      // 0x02, 0x01
        else
            printf("unknown\n");
    } else
        printf("sizeof(short) = %lu\n,this program can't running.", sizeof(short));

    return 0;
}
