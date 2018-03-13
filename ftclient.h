#ifndef FTCLIENT_H
#define FTCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <netdb.h>
#include <sys/wait.h>
#include <unistd.h>

#define DUBUG                           1
#define MAXSIZE 			512 	
#define CLIENT_PORT_ID		30020

struct command
{
	char code[5];
	char arg[255];
};

int read_reply();
void print_reply(int rc);
void read_input(char *buffer ,int size);
int client_send_cmd(struct command *cmd);
void client_login();
int client_open_conn(int sock_con);
int client_read_cmd(char *buffer,int size,struct command *cmd);
int client_list(int sock_data,int sock_conn);
int client_get(int sock_data,int sock_conn,char *arg);

int creat_socket(int port)
{
	int server_sockfd;   //服务器套接字
int yes = 1;
	struct sockaddr_in sock_addr; //服务器套接字地址结构
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port=htons(port);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if((server_sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
   	{
 			perror("socket()error");
			return -1;
   }
if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
	{
		close(server_sockfd);
		perror("setsockopt() error");
		return -1; 
	}
	if(bind(server_sockfd,(struct sockaddr *)&sock_addr,sizeof(sock_addr))<0) //第二个参数强制类型转换为通用地址
	{
		close(server_sockfd);
		perror("bind()error");
		return -1;
}
	if(listen(server_sockfd,5)<0)
	{
		close(server_sockfd);
		perror("listen()error");
		return -1;
}
return server_sockfd;
}


int socket_accept(int socket_listen)//套接字接受请求
{
	int sockfd;
	struct sockaddr_in client_addr;
	socklen_t len=sizeof(client_addr);

	sockfd = accept(socket_listen,(struct sockaddr *)&client_addr,&len);
	if(sockfd < 0)
	{
		perror("accept()error");
		return -1;
}
return sockfd;
}

#endif

