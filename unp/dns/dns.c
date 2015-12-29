/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < dns.c >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/13 >
    > Last Changed: 
    > Description:		使用 gethostbyname函数, 解析域名至IP,实现DNS效果.

	Usage:	./dns www.google.com
	与linux/unix系统本身的dig系统命令对比。
	使用dig -x xxx.xxx.xxx.xxx 进行IP反解析, 与./dns的测试结果进行比较.

>>>>>>>>>>>>>>>>>>>>>>>

	struct hostent *gethostbyname(const char* name);
	参数:域名或者主机名(const char* name), 例如"www.google.cn"等等。
	返回值: hostent的结构体指针。如果函数调用失败,将返回NULL。

	struct hostent
	{
		char    *h_name;				//主机的规范名(区分主机规范名与主机别名)
		char    **h_aliases;			//主机的别名(可以有很多个) 
		int     h_addrtype;				//主机ip地址的类型(ipv4 -> AF_INET or ipv6 -> AF_INET6)
		int     h_length;				//ip地址长度
		char    **h_addr_list; 			//ip地址(网络字节序存储)
		#define h_addr h_addr_list[0] 
	}; 

	主机规范名(eg. www.l.google.com), 主机别名(eg. www.google.com), 别名是为了便于记忆。

****************************************************************/


#include <stdio.h>
#include <stdlib.h> //fprintf()
//#include <sys/socket.h>
#include <netdb.h>		// gethostbyname(); struct hostent;
#include <arpa/inet.h>	// inet_ntop();
#define ERROR_EXIT	1
int main(int argc, char *argv[])
{
	//name_ptr指向主机名称的字符串, alias_pptr指向返回结构体中的主机别名列表.
	char   *name_ptr = NULL, **alias_pptr = NULL;
	char **addr_list_pptr = NULL;			//指向返回的结构体中的主机IP地址.
	struct hostent *host_ptr = NULL;
	char   ip_str[32];				//存储解析的IP地址

	if(argc != 2)
	{
		fprintf(stderr, "Usage: ./%s domain_name\n", argv[0]);
		exit(ERROR_EXIT);
	}
	name_ptr = argv[1];	//char name_ptr[] = "www.google.com";

	//调用gethostbyname()函数, 返回解析结果(存储于结构体中).
	if((host_ptr = gethostbyname(name_ptr)) == NULL) //调用失败返回NULL
	{
		printf("Error: gethostbyname() error for host:%s\n", name_ptr);
		return 0; 
	}

	//打印解析结果.
	printf("official hostname(->h_name):%s\n",host_ptr->h_name);
	for(alias_pptr = host_ptr->h_aliases; *alias_pptr != NULL; alias_pptr++)
		printf("alias hostname(->h_aliases):%s\n",*alias_pptr);
	switch(host_ptr->h_addrtype)
	{
		case AF_INET:
		case AF_INET6:
			addr_list_pptr = host_ptr->h_addr_list;
			for(; *addr_list_pptr != NULL; addr_list_pptr++)
				printf("Ip address list:%s\n", 
						inet_ntop(host_ptr->h_addrtype, *addr_list_pptr, ip_str, sizeof(ip_str)));
			//结构体中使用宏定义, 定义了 h_addr, 表示 h_addr_list[0], 解析的第一个IP值.
			printf("first address in addr_list: %s\n", 
					inet_ntop(host_ptr->h_addrtype, host_ptr->h_addr, ip_str, sizeof(ip_str)));
			break;
		default:
			printf("unknown address type(not AF_INET || AF_INET6)\n");
			break;
	}

	return 0;
}

