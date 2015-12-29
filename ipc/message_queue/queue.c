/****************************************************************
    Copyright (C) 2014 Sean Guo. All rights reserved.
					      									  
    > File Name:         < queue.c >
    > Author:            < Sean Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/07/06 >
    > Last Changed: 
    > Description:		单线程创建一个消息队列, 先写后读.
			
	消息队列是随内核持续的，只有在内核重起或显示删除一个消息队列时,该消息队列才会真正删除;
		系统中记录消息队列的数据结构struct ipc_ids msg_ids位于内核中， 可以访问到每个消息队列头的第一个成员struct kern_ipc_perm;
			而每个struct kern_ipc_perm能够与具体的消息队列对应起来是因为在该结构中，有一个key_t类型成员key，而key则唯一确定一个消息队列。
		系统中所有消息队列都可以在结构msg_ids中找到访问入口;
		消息队列的信息基本上都保存在消息队列头中,可以分配一个类似于消息队列头的结构(struct msqid_ds，),来返回消息队列的属性；同样可以设置该数据结构。
	消息队列其实就是一个消息的链表，每个消息队列有一个队列头，称为struct msg_queue，
		这个队列头描述了消息队列的key值，用户ID，组ID等信息，但它存于内核中.
		而结构体struct msqid_ds能够返回或设置消息队列的信息，这个结构体位于用户空间中,与msg_queue结构相似;

	消息队列允许一个或多个进程向它写入或读取消息，消息队列是消息的链表。
	消息是按消息类型访问，进程必须指定消息类型来读取消息,
	同样，当向消息队列中写入消息时也必须给出消息的类型，如果读队列使用的消息类型为0，则读取队列中的第一条消息。

	内核空间的结构体msg_queue(消息队列的队列头), 描述了对应key值消息队列的情况，
	而对应于用户空间的msqid_ds这个结构体，因此，可以操作msgid_ds这个结构体来操作消息队列。

	当向消息队列发送消息,那么队列会增加一条消息，读消息队列时，消息队列会把此消息删除掉。
	
>>>>>>>>
	
	int msgget(key_t key,int msgflg);
	这个函数返回与key对应的消息队列的标识符。但有两种情况会创建队列:
		(1) 如果没有消息队列与key相对应，且msgflg包含了IPC_CREAT标志 (IPC_CREAT|IPC_EXCL)则创建队列。
			如果消息队列存在那么errno会被设置为EEXIST.
		(2) key参数为IPC_PRIVATE,此时系统为消息队列指定一个key。
	调用成功后返回消息队列标识符，失败返回－1

	int msgrcv(int msqid,struct msgbuf* msgp,int msgsz,long msgtype,int msgflg);
	该系统调用从msgid代表的消息队列中读取一个消息，并把消息存储在msgp指向的msgbuf结构体中
	第一个参数 msqid为消息队列描述字
	第二个参数消息返回后存储的地址msgbuf*
	第三个参数指定msgbuf中的第二个参数mtext的长度
	第四个参数为消息类型
	第五个参数为消息标识符msgflg
		msgflg可以为以下值:
		IPC_NOWAIT 如果没有满足条件的消息则会立即返回，errno=ENOMSG
		IPC_EXCEPT 与msgtyp配合使用，返回队列中第一个类型不为msgtyp的消息 
		IPC_NOERROR 如果队列中满足条件的消息内容大于请求的msgsz,则把消息截断，截断的部分将会丢失
	成功返回读出消息实际字节数，否则返回－1
	msgrcv()解除阻塞的条件有三个：
		消息队列中有了满足条件的消息；
		msqid代表的消息队列被删除；
		调用msgrcv（）的进程被信号中断；

	int msgsnd(int msqid,struct msgbuf* msgp,int msgsz,int msgflg);
	向消息队列中发送一个消息，发送的消息存在msgbuf结构体指针中
	msgsz指定发送消息的第二个参数mtext的大小
	msgflg如下:
		IPC_NOWAIT,如果消息队列没有足够的空间容纳发送的消息 ，则消息队列不会等待，否则会等待。
	成功返回0，否则返回－1
	msgsnd()解除阻塞的条件有三个：
		消息队列中有容纳该消息的空间；
		msqid代表的消息队列被删除；
		调用msgsnd（）的进程被信号中断；

	int msgctl(int msqid,int cmd,struct msqid_ds *buf);
	该函数的功能是对消息队列进行cmd工作，cmd 有三种方式: IPC_STAT,IPC_SET,IPC_RMID
	msqid_ds是用户空间的类似于msg_queue的结构体。
		(1)IPC_STAT 用来获取队列的信息，返回的信息存于buf指向的msqid结构体中
		(2)IPC_SET 用来设置消息队列的属性，设置的属性存于buf指向的msqid结构体
		(3)IPC_RMID 删除由msqid标识的消息队列
	调用成功后返回0，否则返回－1

>>>>>>>>>>>

消息队列与管道以及有名管道相比:
	它提供有格式字节流，有利于减少开发人员的工作量；
	消息具有类型，在实际应用中，可作为优先级使用;
	消息队列可以在几个进程间复用，而不管这几个进程是否具有亲缘关系;
	消息队列是随内核持续的，生命力更强，应用空间更大。

****************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<sys/msg.h>

#define BUFSIZE 128

// 自定义消息结构体
struct msg_buf
{
	long type;				//message type
	char msg[BUFSIZE];		//message buffer
};

int main()
{
	key_t key;
	int msgid;
	struct msg_buf msg_snd,msg_rcv;
	char string[] = "hello world";

	//清空结构体内容, 以便重新赋值.
	memset(&msg_snd,'\0',sizeof(struct msg_buf));
	memset(&msg_rcv,'\0',sizeof(struct msg_buf));
	msg_snd.type = 1;
	msg_rcv.type = 1;
	memcpy(msg_snd.msg, &string, strlen(string));

	// 创建IPC键值key
	key = ftok(".",'A');
	if(key == -1)
	{
		perror("ftok");
		exit(EXIT_FAILURE);
	}

	// 通过key，创建信息队列
	msgid = msgget(key, 0600 | IPC_CREAT);
	if(msgid == -1)
	{
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	// 发送消息
	int res = msgsnd(msgid, (void *)&msg_snd, strlen(msg_snd.msg), 0);
	if(res == -1)
	{
		perror("msgsnd");
		exit(EXIT_FAILURE);
	}
	// 读取消息队列基本信息
	struct msqid_ds msg_info;
	msgctl(msgid, IPC_STAT, &msg_info);
	printf("number of messages in queue:  msg_qnum = %lu\n", msg_info.msg_qnum);

	// 读取消息
	msgrcv(msgid, (void*)&msg_rcv, BUFSIZE, msg_rcv.type, 0);
	printf("rev msg:%s\n",msg_rcv.msg);

	//再次查看消息数量(少1)
	msgctl(msgid, IPC_STAT, &msg_info);
	printf("number of messages in queue:  msg_qnum = %lu\n", msg_info.msg_qnum);

	// 删除队列
	msgctl(msgid,IPC_RMID,0);

	return 0;
}

