/****************************************************************
  Copyright (C) 2014 All rights reserved.

  > File Name:         < client.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2014/06/19 >
  > Description:
    Usage:
        ./server &
        ./echo_client <server IP address>
        killall echo_server
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>         //exit();
#include <string.h>         //bzero();
#include <sys/socket.h>     //socket();
#include <netinet/in.h>     //struct sockaddr_in;
#include <arpa/inet.h>      //inet_pton();
#include <unistd.h>         //read(), write(), close();
#include <errno.h>          //perror(), errno

#define BUFFER_MAX      4096        // max text line length
#define LISTEN_PORT     9669        //listen port

int main(int argc, char **argv)
{
    int i, sockfd;
    char buffer[BUFFER_MAX];
    struct sockaddr_in servaddr;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <Server IP address>\neg. $ ./%s 127.0.0.1\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    //{{{ socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "socket() error");
        exit(EXIT_FAILURE);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(LISTEN_PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        fprintf(stderr, "inet_pton(%s) error", argv[1]);
        exit(EXIT_FAILURE);
    }
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect() error");
        exit(EXIT_FAILURE);
    }
    //}}}

    //send data to server.
    snprintf(buffer, sizeof(buffer), "hello, this is client.\n");
    write(sockfd, buffer, strlen(buffer));

    close(sockfd);
    return 0;
}
