/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < server.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/22 >
    > Last Changed: 
    > Description:		实现一个建议的聊天软件.

 	int bind(int sockfd, struct sockaddr * my_addr, int addrlen);
 	bind()用来设置给参数sockfd 的socket 一个名称. 
 	此名称由参数my_addr 指向一个sockaddr 结构, 对于不同的socket domain 定义了一个通用的数据结构

 	struct sockaddr
 	{
    	unsigned short int sa_family;
	   	char sa_data[14];
	};
	1、sa_family 为调用socket()时的domain 参数, 即AF_xxxx 值.
	2、sa_data 最多使用14 个字符长度.

	此sockaddr 结构会因使用不同的socket domain 而有不同结构定义, 
	例如使用AF_INET domain,其socketaddr 结构定义便为
	struct socketaddr_in
	{
		unsigned short int sin_family;
	    uint16_t sin_port;
		struct in_addr sin_addr;
		unsigned char sin_zero[8];
	};

	struct in_addr
	{
		uint32_t s_addr;
	};
	1、sin_family 即为sa_family
    2、sin_port 为使用的port 编号
    3、sin_addr. s_addr 为IP 地址 sin_zero 未使用.
    参数 addrlen 为sockaddr 的结构长度.

    返回值：成功则返回0, 失败返回-1, 错误原因存于errno 中.

	错误代码：
	1、EBADF 参数sockfd 非合法socket 处理代码.
	2、EACCESS 权限不足
	3、ENOTSOCK 参数sockfd 为一文件描述词, 非socket.

****************************************************************/
#include <stdio.h>
#include <stdlib.h>		//exit();
#include <string.h>		//bzero();
#include <sys/socket.h>
#include <netinet/in.h>	//struct sockaddr_in;
#include <arpa/inet.h>  //inet_pton();
#include <unistd.h>     //write();
#include <errno.h>
#include <pthread.h>    //pthread_create();
#include <sys/sem.h>    //semxx();信号量操作
#include <sys/shm.h>    //shmxx();共享内存操作
//#include <sys/ipc.h>
#include "user.h"		//struct user;

#define MAXLINE 	1024	/* max text line length */
#define LISTENQ     1024    /* 2nd argument to listen() , 排队的最大连接数*/
#define SERVER_IP	"0.0.0.0"
#define LISTEN_PORT	1359	//服务器监听端口

#define IPC_FTOK_FNAME	"/tmp"
#define IPC_FTOK_ID		59		//用于共享内存区域
#define IPC_FTOK_ID2	60		//用于同步信号量

//信号处理函数, 处理SIGCHLD信号, 将等待一个子进程的结束, 防止子程序僵死。
//一个函数可能会处理多个信号, 因此必须要有一个信号参数signo.
void sig_child(int signo);

//验证用户
//参数: 用户名, 用户密码.
int authentication(const char* user_name, const char* user_passwd);

//服务器初始化配置
//参数: 服务器监听listenfd描述符, 服务器监听最大数量listen_max, 服务器的地址信息server_addr, 
//		服务器Ip字符串server_ip, 服务器端口port.
void server_init(int *listenfd, int listen_max, struct sockaddr_in *server_addr,
				 const char *server_ip, int port);
//其中服务器监听服务, 为每一个客户端建立一个进程进行服务.
void server_listen(int listenfd);
//新进程中, 服务器为客户端进行的服务函数. 更改此函数即可更改服务器的服务功能.
void server_client(int clientfd);	

//实现服务器的聊天功能; 一直等待读取用户的输入并发送.
//内部调用server_boardcast(), 实现广播数据给所有客户端.
void server_chat();
//供线程使用的函数, 用于持续读取对方socket的数据并显示出来.
//参数: arg, 传递对方sockfd的指针sockfd_arg.
void *receive_socket(void *sockfd_arg);
void *send_socket(void *sockfd_arg);

//输出固定格式的服务端系统信息, 表示服务器的当前状态.
void server_log(const char *message);

//共享内存所用的结构体, 服务器一个进程写, 其他服务于客户端的子进程都读.
//用于服务器广播消息;
typedef struct boardbuffer
{
	char buffer[1024];
	int buffer_len;				//字符串的有效长度
	int board_index;			//广播序列号, 各个子进程对比序号, 决定是否广播.
} boardbuffer;

//对信号进行P操作, 请求获取临界区资源
void sem_p(int semid);
//对信号进行V操作, 释放临界区资源.
void sem_v(int semid);
//设置信号量的初始值，就是资源个数
union sem_un    //Mac ox X中, <sys/sem.h>实现了同样的union semun.
{
	int val;
	struct semid_ds *buf;
	ushort *array;
};

//配置共享内存的消息为buffer的内容, 其中共享内存按照struct boardbuffer格式存储.
//参数: 共享内存地址share_board, 数据指针buffer, 信号组标识符semid.
void set_share_memory(struct boardbuffer *share_board, void *buffer, int semid);

//-----------------------------------------main-------------------------------------------
int main(int argc, char **argv)
{
	int	listenfd;
	struct sockaddr_in server_addr;
	int child_pid;

	//初始化服务器配置.
	server_init(&listenfd, LISTENQ, &server_addr, SERVER_IP, LISTEN_PORT);

	//为SIGCHLD匹配自定义的函数, 使得处理子进程僵死的问题.
	signal(SIGCHLD, sig_child);	

	//服务器分为监听客户端进程, 监听用户进程, 服务进程.
	child_pid = fork();
	if(child_pid < 0) //fail to fork
	{
		perror("fork");
		exit(1);
	}
	else if(child_pid == 0)
	{
		//子进程监听用户输入, 将服务器端用户输入写入共享内存, 供各客户端读取.
		server_chat();
	}
	else	//父进程, 监听进程.
	{
		//主进程就为一个监听端口, 为每个连接fork一个新的进程.
		while(1)
		{
			server_listen(listenfd);	//监听, 遇到连接则fork进程, 否则阻塞等待连接.
		}
	}
}

//----------------------------------------函数实现----------------------------------------
void sig_child(int signo)
{
	pid_t pid;

	//pid = wait(&state);	//等待一个子进程的结束
	while( (pid = waitpid(-1, NULL, WNOHANG)) > 0)	//使用非阻塞的waitpid等待可结束的所有子进程
		printf("[Server Log]: child pid[%d] terminated.\n", pid);
}

//服务器初始化配置
//参数: 服务器监听listenfd描述符, 服务器监听最大数量listen_max, 服务器的地址信息server_addr, 
//		服务器Ip字符串server_ip, 服务器端口port.
void server_init(int *listenfd, int listen_max, struct sockaddr_in *server_addr,
				 const char *server_ip, int port)
{
	socklen_t addr_len = sizeof(*server_addr);		//获得结构体的长度.

	//创建socket
	if((*listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socke create");
		exit(-1);
	}

	//inet_pton, 点分十进制->整数
	//将输入参数argv[1]的点分十进制转为in_addr结构体的整数.
	if (inet_pton(AF_INET, server_ip, &(server_addr->sin_addr)) <= 0)
	{   
		fprintf(stderr, "inet_pton error for %s\n", server_ip);
		exit(1);
	}  
	bzero(server_addr, sizeof(*server_addr));
	server_addr->sin_family = AF_INET;
	server_addr->sin_port = htons(port);

	// int bind(int sockfd, struct sockaddr * my_addr, int addrlen);
	// bind()用来设置给参数sockfd 的socket 一个名称. 
	// 此名称由参数my_addr 指向一个sockaddr 结构, 对于不同的socket domain 定义了一个通用的数据结构
	if(bind(*listenfd, (struct sockaddr *) server_addr, addr_len) < 0)
	{
		perror("socket bind");
		exit(1);
	}

	// int listen(int s, int backlog);
	// listen()用来监听描述符s 的socket连接请求. 
	// 参数backlog 指定同时能处理的最大连接要求, 如果连接数目达此上限则client 端将收到ECONNREFUSED 的错误. 
	// listen()并未开始接收连接请求, 只设置socket 为listen 模式,真正接收client 端连线的是accept().
	// 通常listen()会在socket(), bind()之后调用, 接着才调用accept().
	// 成功则返回0, 失败返回-1, 错误原因存于errno
	// listen()只适用SOCK_STREAM 或SOCK_SEQPACKET 的socket 类型. 
	// 如果socket 为AF_INET 则参数backlog 最大值可设至128.
	if(listen(*listenfd, listen_max) < 0)
	{
		perror("socket listen");
		exit(1);
	}

	//服务器配置完成, 等待连接.
	server_log("Server init successful, waiting for client to connect...");
}


//其中服务器监听服务, 为每一个客户端建立一个进程进行服务.
void server_listen(int listenfd)
{
	int connfd;	//客户端连接socket描述符
	struct sockaddr_in client_addr;
	socklen_t addr_len;
	char buffer[MAXLINE];
	pid_t child_pid;

	addr_len = sizeof(client_addr);
	// int accept(int s, struct sockaddr * addr, int * addrlen);
	// accept()用来接受描述符s 的socket连接请求. 
	// socket 必需先经bind()、listen()函数处理过, 
	// 当有连接请求进来时, accept()会返回一个新的socket 处理代码,  往后的数据传送与读取就是经由新的socket处理, 
	// 而原来参数s 的socket 能继续使用accept()来接受新的连线要求. 
	// 连线成功时, 参数addr 所指的结构会被系统填入远程主机的地址数据, 参数addrlen 为scokaddr 的结构长度.
	// 成功则返回新的socket 处理代码, 失败返回-1, 错误原因存于errno 中.
	if((connfd = accept(listenfd, (struct sockaddr *) &client_addr, &addr_len)) < 0)
	{
		perror("socket accept");
		exit(1);
	}

	//创建子进程处理客户端请求, 主进程继续监听.
	child_pid = fork();
	if(child_pid < 0)	//failed to fork a process.
	{
		perror("fork");
		exit(1);
	}
	else if(child_pid == 0) //the child process.
	{
		close(listenfd);	//close listenfd in child process, 子进程负责不监听.
		server_client(connfd);	//服务器对客户端的服务函数.
		exit(0);
	}
	else	// the parent process, 父进程不使用此客户端socket, 关闭, 继续监听.
		close(connfd);		//close connfd in parent process.
	//调用close()只会减少对应socket描述符的引用数, 当引用数为0才会清楚对应的socket.
}

void server_client(int clientfd)	//服务器为客户端进行的服务. 更改此函数即可更改服务器的服务功能.
{
	User user;
	pthread_t	recv_thread;		//负责接收客户端数据的线程.
	char server_response = 0;	//服务器传递给客户端的响应, 第一个字节.
	char message_buffer[30];

	//1. 接收客户端发送的用户信息;
	if(read(clientfd, &user, sizeof(user)) == 0)      //从socket中读取返回数据
	{
		perror("failed to read data from server.server terminated prematurely.");
		exit(1);
	}
	//2. 验证用户信息.
	
	//3. 验证成功, 发送响应给服务端, 开始给客户端服务.
	server_response = '1';	//'1'表示连接成功. '2'表示连接失败.
	write(clientfd, &server_response, sizeof(server_response));
	//这里需要广播.
	//sprintf(message_buffer, "[Server Log]: User(%s) login.\n", user.username);
	printf("[Server Log]: User(%s) login.\n", user.username);
	//server_boardcast(message_buffer);

	//4. 开始服务.
	//创建一个线程等待客户端数据, 并在服务器端显示;
	pthread_create(&recv_thread, NULL, receive_socket, &clientfd); //读取客户端数据
	//本线程则用于将服务器共享内存的内容发送给客户端.
	send_socket(&clientfd);
}

//输出固定格式的服务端系统信息, 表示服务器的当前状态.
void server_log(const char *message)
{
	printf("[Server Log]: %s\n", message);
}

//供线程使用的函数, 用于持续读取客户端数据并显示出来.
void* receive_socket(void *sockfd_arg)
{
	char buffer[MAXLINE] = {0};
	int sockfd = (int)*((int *)sockfd_arg);    //传递socket描述符.
	ssize_t n = 0;
	key_t key = ftok(IPC_FTOK_FNAME, IPC_FTOK_ID);
	key_t semkey = ftok(IPC_FTOK_FNAME, IPC_FTOK_ID2);
	int shmid = shmget(key, 4096, IPC_CREAT);
	int semid = semget(semkey, 1, IPC_CREAT);
	struct boardbuffer *share_board = (struct boardbuffer *)shmat(shmid, NULL, 0);
	FILE *fp = NULL;

again:  
	while( (n = read(sockfd, buffer, MAXLINE)) > 0)
	{   
		buffer[n] = '\0';  //bug, 没有传送'\0',导致n与strlen(buffer)不同.导致fputs()输出以前的数据.
		//判断是否是客户端发送文件
		if(buffer[0] == '#')
		{
			//发送文件选项; 服务器应该接收文件
			char filename[20];
			strcpy(filename, buffer+1);
			printf("receiving file: %s\n", filename);
			if( (fp = fopen(filename, "w+")) == NULL)
			{
				server_log("open file error!");
				exit(1);
			}
			int length = 0;
			//读取文件长度.
			read(sockfd, &length, sizeof(int));	
			length = recv(sockfd, buffer, length, 0);	
			//想文件中写入数据
			int write_length = fwrite(buffer, sizeof(char), length, fp);  
			if (write_length < length)  
			{  
				printf("File write failed!\n");  
				break;  
			} 
			if (length < 0)  
			{  
				printf("Recieve data failed!\n");  
				break;  
			}  

			printf("Recieve file finished!\n");  

			fclose(fp);  	  
			fp = NULL;
			continue;
		}
		else	//接收用户的普通聊天信息并显示出来.
		{
			//1. 显示客户端数据至服务器用户界面
			if(fputs(buffer, stdout) == EOF) //显示服务器返回的数据
			{   
				//遇到EOF
				fprintf(stderr, "fputs() error.\n");
				exit(1);
			}   
			//2. 将客户端数据发送至共享内存区域, 供其他客户端读取.
			set_share_memory(share_board, buffer, semid);
		}
	}   
	if(n < 0 && errno == EINTR)
		goto again;
	else if( n < 0)
		perror("failed to read data from server.server terminated prematurely.");

	return NULL;
}
void *send_socket(void *sockfd_arg)
{
	//将服务器共享内存中的数据发送给客户端.
	int sockfd = (int)*((int *)sockfd_arg);    //传递socket描述符.
	key_t key = ftok(IPC_FTOK_FNAME, IPC_FTOK_ID);
	key_t semkey = ftok(IPC_FTOK_FNAME, IPC_FTOK_ID2);
	int shmid = shmget(key, 4096, IPC_CREAT);
	int semid = semget(semkey, 1, IPC_CREAT);
	struct boardbuffer *share_board = (struct boardbuffer *)shmat(shmid, NULL, 0);
	int index = 0;	//记录共享内存的消息索引值.

	while(1)
	{
		//1. P操作
		sem_p(semid);
		//2. 访问临界区数据
		//判断是否需要发送数据
		if(index != share_board->board_index)
		{
			//发送数据
			index = share_board->board_index;
			write(sockfd, share_board->buffer, share_board->buffer_len); 
		}
		//3. V操作
		sem_v(semid);
	}	
}
//与所有客户端进行聊天.使用共享内存. 负责各客户端的进程中各有一个线程负责接收此数据并发送.
void server_chat()
{
	char buffer[1024];
	char *buffer_p = NULL;
	union sem_un sem_u;
	key_t key = ftok(IPC_FTOK_FNAME, IPC_FTOK_ID);		//创建IPC键 
	key_t semkey = ftok(IPC_FTOK_FNAME, IPC_FTOK_ID2);	
	int shmid = shmget(key, 4096, IPC_CREAT | IPC_EXCL | 0666);	//新建共享内存区域
	int semid = semget(semkey, 1, IPC_CREAT | IPC_EXCL | 0666);	//创建同步信号量
	struct boardbuffer *share_board = (struct boardbuffer*)shmat(shmid, NULL, 0);//映射共享内存区域

	//变量sem_u(union sem_un) 初始化信号量组semid.
	sem_u.val = 1;					//这里只使用一个同步信号量
	semctl(semid, 0, SETVAL, sem_u); //使用union sem_un变量, 对信号semid进行初始化.
	//union sem_un中的array数组中的每一个值, 用于初始化信号组的一个信号量;

	//按照boardbuffer结构体格式初始化共享内存.
	bzero(share_board->buffer, sizeof(share_board->buffer));
	share_board->buffer_len = 0;
	share_board->board_index = 0;

	bzero(buffer, 1024);
	strcpy(buffer, "[#Server]: ");	//服务器消息前缀
	buffer_p = buffer + 11;

	while( (fgets(buffer_p, 1000, stdin) != NULL)) //读取用户在终端的输入, 准备发送至客户端.
	{
		set_share_memory(share_board, buffer, semid);
	}
}
//对信号进行P操作, 请求获取临界区资源
void sem_p(int semid)
{
	/*************
	//存储信号操作结构体.
	struct sembuf
	{
		unsigned short sem_num; // semaphore number,操作信号在信号集中的编号
		short sem_op;           //semaphore operation,为0表示释放所控资源的使用权, 为负表示用于获取资源的使用权
		short sem_flg;          //operation flags, 信号操作标志，可能的选择有两种,  
		// IPC_NOWAIT 对信号的操作不能满足时，semop()不会阻塞，并立即返回，同时设定错误信息。
		// SEM_UNDO 程序结束时(不论正常或不正常), 保证信号值会被重设为semop()调用前的值。
	};
	 ************/
	struct sembuf sembuf_p;
	//对0号信号进行'-1'操作.
	sembuf_p.sem_num = 0;
	sembuf_p.sem_op = -1; 

	//int semop(int semid, struct sembuf *sops, unsigned nsops);
	//参数: 信号集的识别码, 可使用semget获取.
	//      指向存储信号操作结构的数组指针sops.
	//      信号操作结构的数量nsops,恒大于或等于1.
	//返回: 成功返回0，失败返回-1并设置errno.
	if(semop(semid, &sembuf_p, 1) == -1) 
		printf("p operation is fail\n");   
}
//对信号进行V操作, 释放临界区资源.
void sem_v(int semid)
{
	struct sembuf sembuf_v;
	sembuf_v.sem_num = 0;
	sembuf_v.sem_op = 1;
	
	if(semop(semid, &sembuf_v, 1) == -1)
		printf("v operation is fail\n");
}

//配置共享内存的消息为buffer的内容.
void set_share_memory(struct boardbuffer *share_board, void *buffer, int semid)
{
	//1. P操作
	sem_p(semid);
	//2. 操作临界区域
	strcpy(share_board->buffer, buffer);
	share_board->buffer_len = strlen(share_board->buffer);
	share_board->board_index++;	//递增索引, 准备发送.
	//3. V操作
	sem_v(semid);
}
