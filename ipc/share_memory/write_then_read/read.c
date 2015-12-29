/****************************************************************
    Copyright (C) 2014 Sean Guo. All rights reserved.
					      									  
    > File Name:         < read.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/23 >
    > Last Changed: 
    > Description:
****************************************************************/
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <fcntl.h>

typedef struct
{
	char name[4];
	int age;
} people;

int main()
{
	int i;
	people *p_map;
	int shmid;
	key_t key = ftok("/tmp", 59); //获得IPC对象标识符

	printf("key_id: %d\t", key);	
	//返回共享内存标识符
	shmid = shmget(key, 4096, IPC_CREAT);
	printf("shmid: %d\n", shmid);

	//将共享内存区域映射到进程的地址空间中去,返回共享内存区域的地址，利用这个指针进行操作
	p_map = (people*)shmat(shmid, NULL, 0);
	
	printf("people name: %s\npeople age: %d\n",p_map->name, p_map->age);
	//断开与共享内存的连接
	shmdt(p_map);
}
