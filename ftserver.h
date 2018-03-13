#ifndef FTSERVER_H
#define FTSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXSIZE   512 	
#define CLIENT_PORT_ID		30020
struct command
{
	char code[5];
	char arg[255];
};

int creat_socket();
int socket_accept(int socket_listen);
int check_user(char *user,char*pass);  //登录确认
int recv_data(int socket_control,char *buf);
int server_login(int sockfd); //获取用户登录密码
int send_response(int sockfd,int rc);
int recv_cmd(int socket_control,char *cmd,char *arg);
int socket_conn(int port,char*host);
int start_data_conn(int socket_control);
int server_list(int socket_data,int socket_control);
void server_retr(int socket_data,int socket_control,char *filename);
void server_process(int socket_control); //处理客户端请求;
void trimstr(char *str, int n);
#endif
