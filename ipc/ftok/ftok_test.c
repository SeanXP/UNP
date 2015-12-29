/****************************************************************
    Copyright (C) 2014 Sean Guo. All rights reserved.
					      									  
    > File Name:         < ftok_test.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/23 >
    > Last Changed: 
    > Description:

	系统建立IPC通讯 （消息队列、信号量和共享内存） 时必须指定一个ID值。通常情况下，该id值通过ftok函数得到。
	//参数:fname为指定的文件名(该文件必须是存在而且可以访问的,一般使用当前目录),序号id(只有8个bit被使用).
	//返回：成功返回key_t值, 失败返回-1;
	key_t ftok( char * fname, int id );
		在一般的UNIX实现中，是将文件的索引节点号取出，前面加上子序号得到key_t的返回值。
		如指定文件的索引节点号为65538, 换算成16进制为0x010002, 
		而你指定的ID值为38, 换算成16进制为0x26,则最后的key_t返回值为0x26010002。
	两进程如果在pathname和ftok_id上达成一致(或约定好), 双方就都能够通过调用ftok函数得到同一个IPC键.
	ftok调用返回的整数IPC键由id的低序8位(0x57)，st_dev成员的低序8位，st_info的低序16位组合而成;
	fname必须事先已存在, 否则不能保证生成同一IPC键.
****************************************************************/

#include <stdio.h>
#include <stdlib.h>		//exit();
#include <sys/stat.h>	//struct stat;
#include <sys/types.h>
#include <sys/ipc.h>

#define IPCKEY 0x11

int main(int argc, char *argv[])
{
	//struct stat结构体是用来描述一个linux系统文件系统中的文件属性的结构。
	struct stat file_state;

	if (argc != 2)
	{
		fprintf(stderr, "Usage: ./ftok filename\n");
		exit(1);
	}
	//int stat(const char *path, struct stat *struct_stat);
	//返回0表示成功执行;
	//stat()没有处理字符链接(软链接）的能力, 如果一个文件是符号链接，stat会直接返回它所指向的文件的属性；
	//lstat()返回的就是这个符号链接的内容
	stat(argv[1], &file_state);	//取得对应文件的信息.

	//ftok调用返回的整数IPC键由id的低序8位(0x57)，st_dev成员的低序8位，st_info的低序16位组合而成;
	printf("st_dev: %lx, st_ino: %lx\nid: 0x57, key: %x\n", 
			(u_long) file_state.st_dev, (u_long) file_state.st_ino, 
			ftok(argv[1], 0x57));

	return 0;
}

void test1()	//id的范围只有8位, 这里打印出对应一个fname,所有的key值.
{
	int i = 0;
	for ( i = 1; i < 256; ++ i )
	{
		printf("id=%x\t",i);
		printf("key=%x\n",ftok("/tmp", i));
	}
}
