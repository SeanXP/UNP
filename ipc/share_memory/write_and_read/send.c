/****************************************************************
    Copyright (C) 2014 Sean Guo. All rights reserved.
					      									  
    > File Name:         < send.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/23 >
    > Last Changed: 
    > Description:
****************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <signal.h>

int semid;
int shmid;

//定义用户空间内保存信号量集合信息的联合体
union sem_un {
	int val;		//初始化信号量集合中的信号量
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo*_buf;
};

void myhandler(int signum,siginfo_t *si,void* vcontext);

int main()
{
	int ret;
	struct sembuf getsem, setsem;	//定义信号量操作的结构体
	key_t semkey;					//信号量的IPC对象标识符
	key_t shmkey;					//共享内存的IPC对象标识符
	char* p_map;					//返回共享内存的地址

	//semget来创建一个信号量集合
	semkey = ftok("/tmp",0x11);
	if(semkey == -1)
	{
		perror("sem ftok error");
		exit(1);
	}
	//创建一个信号量集合中有两个信号量
	semid = semget(semkey, 2, IPC_CREAT | IPC_EXCL | 0666); 
	if(semid==-1)
	{
		perror("semget error");
		exit(1);
	}
	//打印出semid的共享内存标示符
	printf("semid = %d\n",semid);
	
	//注册信号处理函数
	struct sigaction act, oldact;
	act.sa_sigaction = myhandler;
	act.sa_flags = SA_SIGINFO | SA_RESETHAND;
	sigaction(SIGINT, &act, &oldact);
	
	//初始化信号量集合中的这两个信号量
	union sem_un seminit;
	seminit.val=0;						//将信号量集合中的信号量初始值设置为0
	semctl(semid,0,SETVAL,seminit);
	semctl(semid,1,SETVAL,seminit);

	//设置信号量的操作
	getsem.sem_num=1;
	getsem.sem_op=-1;
	getsem.sem_flg=SEM_UNDO;

	setsem.sem_num=0;
	setsem.sem_op=1;
	setsem.sem_flg=SEM_UNDO;

	//返回共享内存的IPC对象标识符
	shmkey=ftok("/root/shm3.c",0x0029);
	if(shmkey==-1){
		perror("ftok error");
		exit(1);
	}
	printf("shmkey=%d/n",shmkey);
	shmid=shmget(shmkey,4096,IPC_CREAT|IPC_EXCL|0666);
	printf("shmid%d/n",shmid);
	if(shmid==-1){
		perror("shm error");
		exit(1);
	}


	p_map=(char*)shmat(shmid,NULL,0);//返回映射文件系统shm中文件到内存的对应的地址

	while(1){

		printf("Lucy:");
		fgets(p_map,256,stdin);
		semop(semid,&setsem,1);//释放资源
		semop(semid,&getsem,1);
		printf("Peter:%s",p_map);

	}




}

void myhandler(int signo,siginfo_t *si,void* vcontext){//当我们按下ctrl+c时，应该删除信号量集合与共享内存
	puts("delete operation");
	semctl(semid,0,IPC_RMID);//这个函数可以有三个参数或者四个参数，当有四个参数时，第四个参数必须是sem_un联合体，初始化操作，这里使用的是3个参数
	//第二个参数是信号量的索引，当为IPC_RMID时，第2个参数被忽略，在进程退出时，删除信号量集合
	shmctl(shmid,IPC_RMID,NULL);
	exit(1);

}
