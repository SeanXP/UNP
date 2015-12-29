/****************************************************************
            Copyright (C) 2014 All rights reserved.
					      									  
    > File Name:         < user.h >
    > Author:            < Shawn Guo >
    > Mail:              < iseanxp+code@gmail.com >
    > Created Time:      < 2014/06/22 >
    > Last Changed: 
    > Description:		简易聊天室-用户的数据结构
	客户端读取用户的信息，发送给服务器，服务器查询数据库，返回验证信息。

****************************************************************/


#ifndef __USER__
#define __USER__

#define USERNAME_SIZE	20		//用户名长度
#define	PASSWORD_SIZE	20		//用户密码长度

typedef struct User
{
	char username[USERNAME_SIZE];	//用户名
	char password[PASSWORD_SIZE];	//密码
} User;

#endif
