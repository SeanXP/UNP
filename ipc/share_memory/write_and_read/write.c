/****************************************************************
    Copyright (C) 2014 Sean Guo. All rights reserved.
					      									  
    > File Name:         < write.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/23 >
    > Last Changed: 
    > Description:		信号量+共享内存

	系统调用 semget 和 semctl 负责分配、释放信号量。
	semctl函数对一个信号量执行各种控制操作。
	#include <sys/sem.h>
	int semctl(int semid, int semnum, int cmd, ... );
	semctl() 在semid 标识的信号量集上，或者该集合的第semnum个信号量上执行cmd指定的控制命令。(信号量集合索引起始于零。)
	根据 cmd 不同，这个函数有三个或四个参数。当有四个参数时，第四个参数的类型是 union semun。
	调用程序必须按照下面方式定义这个联合体:(该联合体没有定义在任何系统头文件中, 因此得用户自己声明)
	(Mac os X下(UNIX)内, <sys/sem.h>中实现了union semun.
	
	union semun {
		int val;               // SETVAL使用的值   
		struct semid_ds *buf;  // IPC_STAT、IPC_SET 使用缓存区
		unsigned short *array; // GETALL,、SETALL 使用的数组 
		struct seminfo *__buf; // IPC_INFO(Linux特有) 使用缓存区 
	}; 

	semid_ds 数据结构在头文件 <sys/sem.h> 有如下定义：
	struct semid_ds { 
		struct ipc_perm sem_perm;   // 所有者和权限
		time_t sem_otime;           // 上次执行 semop 的时间  
		time_t sem_ctime;           // 上次更新时间 
		unsigned short sem_nsems;   // 在信号量集合里的索引
	};

	cmd 的有效值是：IPC_STAT,IPC_SET,IPC_RMID,IPC_INFO,
	失败时 semctl() 返回 -1 并设置 errno 指明错误。 
	否则该系统调用返回一个依赖于 cmd 的非负值

****************************************************************/
/*向共享内存中写入People*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>	//semxx();信号量操作
#include <sys/shm.h>	//shmxx();共享内存操作

struct People
{
	char name[10];
	int age;
};

/*设置信号量的初始值，就是资源个数*/
union sem_un	//Mac ox X中, <sys/sem.h>实现了同样的union semun.
{
	int val;
	struct semid_ds *buf;
	ushort *array;
} sem_u;

//信号量集ID，共享内存标识符id。
//信号量集ID设为global, 便于P(),V()函数使用。也可以传递参数.
int semid;
int shmid;

void p();
void v();

int main()
{
	key_t semkey;
	key_t shmkey;

	//创建IPC键.
	semkey = ftok("write.c",0);
	printf("semkey_id:%d\t", semkey);
	shmkey = ftok("read.c",0);
	printf("shmkey_id:%d\n", shmkey);

	/*创建共享内存和信号量的IPC*/
	//根据IPC键值semkey, 决定是创建新的信号量还是返回对应的信号量标识符.
	semid = semget(semkey, 1, 0666 | IPC_CREAT);
	if(semid == -1)
		printf("creat sem is fail\n");
	printf("semid:%d\t", semid);
	//同理, 创建共享内存空间.
	shmid = shmget(shmkey, 1024, 0666 | IPC_CREAT);
	if(shmid == -1)
		printf("creat shm is fail\n");
	printf("shmid:%d\n", shmid);

	//全局变量sem_u(union sem_un) 
	sem_u.val = 1;		//信号量数量
	semctl(semid, 0, SETVAL, sem_u); //使用union sem_un变量, 对信号semid进行初始化.
	//union sem_un中的array数组中的每一个值, 用于初始化信号组的一个信号量;


	/*将共享内存映射到当前进程的地址中，之后直接对进程中的地址addr操作就是对共享内存操作*/
	struct People *addr;
	addr = (struct People*)shmat(shmid, 0, 0);	//映射共享内存空间.
	if(addr==(struct People*)-1)
		printf("shm shmat is fail\n");

	/*向共享内存写入数据*/
	//1. P操作
	p();
	//2. 获得临界区(共享内存区)资源, 可以进行读写了.
	strcpy((*addr).name,"sean guo");
	(*addr).age = 20;
	//3. 执行V操作, 释放临界区(共享内存区)资源，其他进程可以访问了。
	v();

	/*将共享内存与当前进程断开*/
	if(shmdt(addr) == -1)
		printf("shmdt is fail\n");    

}
/*信号量的P操作*/
void p()
{
	/*
	    //存储信号操作结构体.
		struct sembuf
		{
			unsigned short sem_num; // semaphore number,操作信号在信号集中的编号
			short sem_op; 			//semaphore operation,为0表示释放所控资源的使用权, 为负表示用于获取资源的使用权
			short sem_flg; 			//operation flags, 信号操作标志，可能的选择有两种,  
									// IPC_NOWAIT 对信号的操作不能满足时，semop()不会阻塞，并立即返回，同时设定错误信息。
									// SEM_UNDO 程序结束时(不论正常或不正常), 保证信号值会被重设为semop()调用前的值。
		};



	   */
	struct sembuf sem_p;
	//对0号信号进行'-1'操作.
	sem_p.sem_num = 0;
	sem_p.sem_op = -1;

	//int semop(int semid, struct sembuf *sops, unsigned nsops);
	//参数: 信号集的识别码, 可使用semget获取.
	//		指向存储信号操作结构的数组指针sops.
	//		信号操作结构的数量nsops,恒大于或等于1.
	//返回: 成功返回0，失败返回-1并设置errno.
	if(semop(semid, &sem_p, 1) == -1)
		printf("p operation is fail\n");             
}

/*信号量的V操作*/
void v()
{
	struct sembuf sem_v;
	sem_v.sem_num = 0;
	sem_v.sem_op = 1;
	if(semop(semid, &sem_v, 1) == -1)
		printf("v operation is fail\n");
}

