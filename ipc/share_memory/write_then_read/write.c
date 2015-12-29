/****************************************************************
    Copyright (C) 2014 Sean Guo. All rights reserved.
					      									  
    > File Name:         < write.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/23 >
    > Last Changed: 
    > Description:		写进程-将数据写入到共享内存

	共享内存是指把共享数据放到共享内存区域，
	任何需要访问共享内存区域数据的进程都在自己的进程地址空间中开辟一个新的内存区域，
	用来映射共享内存数据的物理页面(所有需要访问共享内存区域的进程都要把该共享区域映射到本进程的地址空间中去), 
	系统用shmget获得或创建一个IPC的共享内存区域，并返回相应的标识符，
	通过shmat将共享内存区域映射到进程的地址空间中去,每一个共享内存区域都对应shm文件系统上的一个文件，
	相当于映射shm文件系统上的同名文件到共享内存区域，
	shmdt是解除对共享内存区的映射，shmctl是对共享内存区的控制操作

	共享内存作用是加快进程间的通信,共享内存的修改对进程是可见的, 将共享内存区域映射到进程地址空间中去, 
	加快进程访问文件/设备的速度.

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


	//返回共享内存标识符shmid;
	//如果共享内存区域不存在，创建一个共享内存区域
	//参数: IPC对象标识符key, 共享内存的大小size(页的整数倍), 标志shmflg;
	int shmget(key_t key, size_t size, int shmflg);
	
	//将共享内存映射到进程的地址空间中，返回的是共享内存的虚拟地址。
	//参数: 共享内存标识符shmid, 映射的共享内存的地址shmaddr(为NULL时, 表示让内核自己去选择), 
	//		本进程对改共享内存的操作模式shmflg(为0表示不设置任何权限限制, 即具有读写权限)
	//返回: 成功映射将返回映射后的地址, 失败返回-1;
	void *shmat(int shmid, const void* shmaddr, int shmflg);	
		shmflg:	SHM＿RDONLY(只读), SHM＿RND, SHM＿REMAP;
		shmaddr: 为空, 则内存选取一片空闲区域; 
				 若非空, 返回地址取决于调用者是否给shmflg参数指定SHM_RND值;
					SHM_RND没有指定: 共享内存区附加到由 shmaddr 指定的地址;
					SHM_RND指定: 附加地址为shmaddr向下舍入一个共享内存低端边界地址后的地址 (SHMLBA ，一个常址)。
			     通常将参数 shmaddr 设置为 NULL;

	//参数: 共享内存标识符shmid, 控制命令cmd, 结构体指针buf;
	int shmctl(int shmid, int cmd, struct shmid_ds* buf);	
		cmd: IPC＿STAT (取得共享内存状态),IPC＿SET(改变共享内存状态), IPC＿RMID(删除共享内存);
		buf: IPC＿STAT，IPC＿SET可以取得结构体内的状态或者是设置状态。 

****************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/shm.h>		//shmxxx();

//设计结构体, 将存储于共享内存.
typedef struct 
{
	char name[4];
	int age;
} people;

int main(int argc,char* argv[])
{
	key_t key = ftok("/tmp", 59); //创建一个IPC对象标识符
	int i;
	int shmid;
	people *p_map;

	printf("key_id: %x\t", key);
	//根据IPC键key, 返回共享内存标识符;
	//如果共享内存区域不存在,则创建一个与key相对应的共享内存区域; 
	//通过IPC键, 实现共享内存区域的区分;
	shmid = shmget(key, 4096, IPC_CREAT | IPC_EXCL | 0666);
	//IPC_CREAT|IPC_EXCL:  如果内核中不存在键值与key相等的共享内存,则新建一个共享内存；如果存在这样的共享内存则报错

	printf("shmid: %x\n", shmid);

	//将共享内存区域映射到本进程的地址空间中去
	//让内核去选择一个地址, 对操作权限没有限制, 返回一个共享内存地址
	//这里将共享内存用于结构题people的存储.
	p_map = (people*)shmat(shmid,NULL,0);  
	
	//对共享内存区域进行读写. 并再另外一个程序的进程中读取. 
	memcpy(p_map->name, "sean", 4);
	p_map->age = 20;

	//断开与共享内存区域的边接
	shmdt(p_map);
}
