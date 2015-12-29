/****************************************************************
            Copyright (C) 2014 All rights reserved.
    > File Name:         < client.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/19 >
    > Last Changed: 
    > Description:
	
Usage: ./client <server IP address>
eg. ./a.out 127.0.0.1

****************************************************************/

#include <stdio.h>
#include <stdlib.h>		//exit();
#include <string.h>		//bzero();
#include <sys/socket.h>	
#include <netinet/in.h>	//struct sockaddr_in;
#include <arpa/inet.h>	//inet_pton();
#include <unistd.h>		//read(), write(), close();
#include <errno.h>		//perror(), errno.
#include <pthread.h>	//pthread_create();
#include <unistd.h>		//usleep();
#include "user.h"		//struct User;

#define MAXLINE 4096	/* max text line length */
#define SERVER_PORT 1359    //服务器监听端口

//客户端初始化配置, 建立与Server的连接.
//参数: 客户端连接符sockfd描述符, 服务器的地址信息client_addr, 
//      服务器Ip字符串server_ip, 服务器端口port.
void client_init(int *sockfd, struct sockaddr_in *server_addr, const char *server_ip, int port);

//发送客户端系统消息给用户, 显示系统状态之用.
void client_log(const char *message);
//菜单, 显示客户端的操作, 返回用户选择的功能;
//返回: 0, 退出程序. 1, 登录. 2, 注册. 
int client_menu();
//提醒用户输入帐号信息;
//参数: User指针.
//功能: 与用户进行交互, 提醒用户输入正确的User信息.
void input_user(User *user);

//与服务器进行聊天.
//参数: 服务器端口sockfd, 本客户端的用户信息user.
void chat(int sockfd, User user);
//供线程使用的函数, 用于持续读取对方socket的数据并显示出来.
//参数: arg, 传递对方sockfd的指针.
void* receive_socket(void *sockfd_arg);

//本程序是IPv4协议相关的. 因为sockaddr_in结构体, AF_INET.
//IPv4 [ IPv6 ]
//sockaddr_in [ sockaddr_in6 ]
//AF_INET [ AF_INET6 ]
int main(int argc, char **argv)
{
	int i, sockfd;
	char buffer[MAXLINE + 1];
	struct sockaddr_in	server_addr;
	User user;
	int option = 0;		//菜单选项
	char server_response = 0; //服务器端给客户端的响应码, 总为第一个字节.

	if (argc != 2)
	{
		fprintf(stderr, "usage: %s <Server IP address>\neg. $ ./a.out 127.0.0.1\n", argv[0]);
		exit(1);
	}

	client_init(&sockfd, (struct sockaddr_in *)&server_addr, argv[1], SERVER_PORT);

	while((option = client_menu()) != 0)	//打印菜单, 返回option = 0表示退出程序.
	{
		switch(option)
		{
			case 1:		//登录log in
				{
					input_user(&user);
					//send User message for Verification.
					write(sockfd, &user, sizeof(user));      //发送用户数据至socket发送缓冲区
					client_log("Connect to Server Database...");
					//阻塞等待服务器应答.
					if(read(sockfd, &server_response, sizeof(char)) == 0)      //从socket中读取返回数据
					{
						perror("failed to read data from server.server terminated prematurely.");
						exit(1);
					}
					switch(server_response)
					{
						case '1':	//用户信息验证成功;
							{
								//开启一个线程读取服务器的数据, 本线程用来读取用户的输入发至服务器
								pthread_t receive_thread;	//负责接收服务器数据的线程.
								client_log("User information successfully verified.");
								pthread_create(&receive_thread, NULL, receive_socket, &sockfd);
								chat(sockfd, user);
							}
							break;
						case '2': 	//用户信息验证失败;
							break;
					}
				}
				break;
		}	
	}

	//结束客户端
	client_log("Goodbye bye!");


/*
*/
	return 0;
}

//发送客户端系统消息给用户, 显示系统状态之用.
void client_log(const char *message)
{
	printf("[Client Log]: %s\n", message);
}

//提醒用户输入帐号信息;
//参数: User指针.
//功能: 与用户进行交互, 提醒用户输入正确的User信息.
void input_user(User *user)
{
	printf("[Sign In]: Please input your usernaem: ");
	fgets(user->username, USERNAME_SIZE, stdin);
	user->username[strlen(user->username) - 1] = '\0';	//将最后的一个回车符去除.
	printf("[Sign In]: Please input your password: ");
	fgets(user->password, PASSWORD_SIZE, stdin);
	user->password[strlen(user->password) - 1] = '\0';
}

//菜单, 显示客户端的操作, 返回用户选择的功能;
//返回: 0, 退出程序. 1, 登录. 2, 注册. 
int client_menu()
{
	int option = 0;

	printf("\t\t\t******************************************\n");
	printf("\t\t\t*                ChatRoom                *\n");
	printf("\t\t\t*                                        *\n");
	printf("\t\t\t*               0. exit                  *\n");
	printf("\t\t\t*               1. sign in               *\n");
	printf("\t\t\t*               2. sign up               *\n");
	printf("\t\t\t*                                        *\n");
	printf("\t\t\t*(C) 2014 Sean Guo. All rights reserved. *\n");
	printf("\t\t\t******************************************\n");

	do
	{
		printf("\t\t\t               >");
		scanf("%d",&option); 		//读取用户输入
		getchar();					//eat '\n'
		if(option >= 0 && option <= 2) 
			return option;
		else
		{
			//输入错误, 继续等待输入
			client_log("Error input!");
			continue;
		}
	}while(1);

	return 0;
}

//与服务器进行聊天.
//参数: 服务器端口sockfd, 本客户端的用户信息user.
void chat(int sockfd, User user)
{
	char buffer[MAXLINE] = {0};
	char name[USERNAME_SIZE + 10] = {0};
	FILE *fp = NULL;

	name[0] = '[';
	name[1] = '\0';
	strcat(name, user.username);
	strcat(name, "]");

	while( (fgets(buffer, MAXLINE, stdin) != NULL))	//读取用户在终端的输入, 准备发送至服务器.
	{
		//判断是传输消息, 还是传输文件指令; 传输文件使用'#'开头的命令;	
		if(buffer[0] == '#')
		{
			client_log("send file...");
			buffer[strlen(buffer) - 1] = '\0';
			printf("[Client log]: file name:(%s)\n", buffer+1);
			
			if((fp = fopen(buffer+1, "r")) == NULL)
			{
				//打开文件错误
				printf("[Client Log]: File:%s, No such file.\n[Client Log]: File transfer Termination.\n", buffer+1);  
			}
			else
			{
				//开始传输文件.
				int file_block_length = 0;
				
				//1. 将文件信息发送给服务器.
				write(sockfd, buffer, strlen(buffer));
				while( (file_block_length = fread(buffer, sizeof(char), MAXLINE, fp)) > 0)  
				{  
					printf("file_block_length = %d\n", file_block_length);  
					//2. 将文件长度发送给服务器.
					write(sockfd, &file_block_length, sizeof(int));

					//3. 发送文件数据.
					if (send(sockfd, buffer, file_block_length, 0) < 0)  
					{  
						printf("Send file failed!\n");  
						break;  
					}  

					bzero(buffer, sizeof(buffer));  
				}  
				//文件传输结束, 关闭文件.
				fclose(fp);  
				fp = NULL;
				printf("File transfer finished!\n");  
			}
		}	
		else	//传输消息.
		{
			//发送的数据前加入用户的信息；
			write(sockfd, name, strlen(name));			//打印用户名前缀.
			write(sockfd, buffer, strlen(buffer)); 		//发送至socket发送缓冲区
		}
	}
}

//供线程使用的函数, 用于持续读取对方*sockfd_arg的数据并显示出来.
void* receive_socket(void *sockfd_arg)
{
	char buffer2[MAXLINE] = {0};
	int sockfd = (int)*((int *)sockfd_arg);	//传递socket描述符.
	ssize_t n = 0;

again:	
	while( (n = read(sockfd, buffer2, MAXLINE)) > 0)
	{
		buffer2[n] = '\0';	//bug, 没有传送'\0',导致n与strlen(buffer2)不同. 导致fputs()输出以前的数据.
		if(fputs(buffer2, stdout) == EOF) //显示服务器返回的数据
		{
			//遇到EOF
			fprintf(stderr, "fputs() error.\n");
			exit(1);
		}
	}
	if(n < 0 && errno == EINTR)
		goto again;
	else if( n < 0)
		perror("failed to read data from server.server terminated prematurely.");

	return NULL;
}

//客户端初始化配置, 建立与Server的连接.
//参数: 客户端连接符sockfd描述符, 客户端的地址信息client_addr, 
//      服务器Ip字符串server_ip, 服务器端口port.
void client_init(int *sockfd, struct sockaddr_in *server_addr, const char *server_ip, int port)
{
	//建立网际(AF_INET)字节流(SOCK_STREAM)套接字
	//返回一个int型的文件描述符, 代替此套接字接口(一般为3开始, 0,1,2对应stdin,stdout,stderr)
	if ( (*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "create socket error");
		exit(1);
	}

	bzero(server_addr, sizeof(*server_addr));	//将servaddr结构体内容清零.

	//对于BSD,是AF,对于POSIX是PF。
	//UNIX系统支持AF_INET，AF_UNIX，AF_NS等，而DOS,Windows中仅支持AF_INET，它是网际网区域。
	//理论上建立socket时是指定协议，应该用PF_xxxx，设置地址时应该用AF_xxxx。
	server_addr->sin_family = AF_INET;		//AF 表示ADDRESS FAMILY 地址族，PF 表示PROTOCOL FAMILY 协议族，但这两个宏定义是一样的

	//设定端口号
	//htons(),将主机的无符号短整形数转换成网络字节顺序。简单说就是高低位字节互换.
	//网络字节顺序: (Big-Endian, 低地址存放高位),保证网络数据传输与CPU,OS无关.
	//这里使用著名端口号Port 13 (DAYTIME协议, 服务器通过TCP/IP,以ASCII字符返回当前日期时间)
	server_addr->sin_port   = htons(port);	/* daytime server */
	//htonl(), htons()... 网络字节(h)转(to)网络字节序(n), l(for long), s(for short)...
	//本质上struct sockaddr_in的内容都要写为网络字节序.
	//因为AF_INET一般被宏定义为0, 故可以不用调用htons().

	//inet_pton, 点分十进制->整数
	//将输入参数argv[1]的点分十进制转为in_addr结构体的整数.
	if (inet_pton(AF_INET, server_ip, &(server_addr->sin_addr)) <= 0)
	{
		fprintf(stderr, "inet_pton error for %s", server_ip);
		exit(1);
	}

	// struct sockaddr, 通用套接字地址结构.
	// connect(),建立TCP连接
	// 成功返回0, 失败返回-1并设置errno.
	if (connect(*sockfd, (struct sockaddr *) server_addr, sizeof(*server_addr)) < 0)
	{
		fprintf(stderr, "connect error\n");
		exit(1);
	}
	client_log("Connect to Server successful!");
}
