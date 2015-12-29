/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < host_byte_order.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/06 >
    > Last Changed: 
    > Description:		test the host byte order( Little-Endian / Big-Endian )

	Big-Endian(数据的大端(MSB)存储在起始位置(低地址位))
	Little-Endian(小端(LSM)存储在起始位置(低地址位))

****************************************************************/


//使用union的特性来判断Host byte order.
//存储数据, 然后用另一个读取方式读取.
union {
	short  s;
	char   c[sizeof(short)];
} un;

int main(int argc, char **argv)
{
	//test data.
	//Big-Endian: (0x01, 0x02)
	//Little-Endian:(0x02,0x01)
	un.s = 0x0102;

	//printf("%s: ", CPU_VENDOR_OS);	//UNPv1中, 使用./configure中的命令, 配置了宏定义CPU_VENDOR_OS, 这里不使用.
	if (sizeof(short) == 2) {
		if (un.c[0] == 1 && un.c[1] == 2)
			printf("big-endian\n");
		else if (un.c[0] == 2 && un.c[1] == 1)
			printf("little-endian\n");
		else
			printf("unknown\n");
	} else
		printf("sizeof(short) = %d\n,this program can't running.", sizeof(short));

	return 0;
}
