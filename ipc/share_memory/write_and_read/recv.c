/****************************************************************
    Copyright (C) 2014 Sean Guo. All rights reserved.
					      									  
    > File Name:         < recv.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/23 >
    > Last Changed: 
    > Description:
****************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>

int main(){
	int semid;
	int shmid;
	int ret;
	char *p_map;
	key_t semkey,shmkey;
	struct sembuf getsem,setsem;
	semkey=ftok("/etc/fb.modes",0x0031);
	if(semkey==-1){


		perror("ftok error");
		exit(1);
	}
	semid=semget(semkey,0,IPC_CREAT);//当第二个参数为0时，表示不关心信号量的数目
	if(semid==-1){
		perror("semid error");
		exit(1);
	}
	printf("semid=%d/n",semid);

	getsem.sem_num=0;
	getsem.sem_op=-1;
	getsem.sem_flg=SEM_UNDO;
	setsem.sem_num=1;
	setsem.sem_op=1;
	setsem.sem_flg=SEM_UNDO;



	shmkey=ftok("/root/shm3.c",0x0029);
	if(shmkey==-1){
		perror("ftok error");
		exit(1);
	}
	printf("shmkey=%d/n",shmkey);
	shmid=shmget(shmkey,4096,IPC_CREAT);//发送方已经创建了共享内存，所以接收方不需要创建，当为IPC_CREA时，如果已经创建与key关联的共享内存时，此时返回的是共享内存标识符
	printf("shmid=%d/n",shmid);
	if(shmid==-1){
		perror("shm error");
		exit(1);
	}

	p_map=(char*)shmat(shmid,NULL,0);//返回映射文件系统shm中文件到内存的对应的地址
	//第3个参数shmflg,如果shmflg为SHM＿RND，且shmaddr!=0,则共享内存会连接到shmaddr-(shmaddr%SHMLBA)的地址上，即连接到距离shmaddr最近的且是SHMLBA倍数的内存上
	//如果不为SHM_RND,则连接到shmaddr指定的内存上
	//如果SHM_RDONLY则会连接到只读段，否则连接的段是可被读写的

	while(1){
		semop(semid,&getsem,1);
		printf("Lucy:%s",p_map);
		printf("Peter:");
		fgets(p_map,256,stdin);
		semop(semid,&setsem,1);
	}

	return 1;
}
