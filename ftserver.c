#include "ftserver.h"

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
void trimstr(char *str, int n)
{
	int i;
	for (i = 0; i < n; i++) 
	{
		if (isspace(str[i])) str[i] = 0;
		if (str[i] == '\n') str[i] = 0;
	}
}

int check_user(char *user,char*pass)  //登录确认
{
	int flag = 0;
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	size_t num_read = 0;
	char username[MAXSIZE];
	char password[MAXSIZE];
	char buf[MAXSIZE];
	char *pch;
	fp = fopen("user.txt","r");
	if(fp == NULL)
	{
		perror("file not found");

		exit(1);
	}
while((num_read = getline(&line,&len,fp)) != -1)
{

		bzero(buf,MAXSIZE);
		strcpy(buf,line);

		pch = strtok(buf," ");  //分割字符串

		strcpy(username,pch);
		if(pch != NULL)
		{

				pch = strtok(NULL," ");

				strcpy(password,pch);

		}
		trimstr(password,(int)strlen(password));

	 if((strcmp(username,user)==0) && (strcmp(pass,password)==0))
	 {
			 flag=1;
			 break;
	 }

}

	free(line);
	fclose(fp);

	return flag;
}
//接受数据
int recv_data(int socket_control,char *buf)
{
	int rc,i,num;
	size_t len;
	bzero(buf,MAXSIZE);
	rc= recv(socket_control,&num,4,0);

//printf("length%d\n",num);

		len = recv(socket_control,buf,num,0);

		if(len < 0)
			return -1;		
		return len;
}

 
int server_login(int sockfd) //获取用户登录密码
{
	char buf[MAXSIZE];
	char user[MAXSIZE];
	char pass[MAXSIZE];

	bzero(buf,MAXSIZE);
	bzero(user,MAXSIZE);
	bzero(pass,MAXSIZE);
	

    if((recv_data(sockfd,buf)) == -1)
	{
		perror("recv()error");	
		exit(1);

	}


	int i = 4;
	int n = 0;
	while(buf[i] != 0)

		user[n++] = buf[i++];


	
	send_response(sockfd,331);

	bzero(buf,MAXSIZE);
	if((recv_data(sockfd,buf)) == -1)
		{	
			perror("recv()error");
			exit(1);
		}


	i = 4;
	n = 0;

	while(buf[i] != 0)

		pass[n++] = buf[i++];

      return (check_user(user,pass));


}

int send_response(int sockfd,int rc)
{
	int conv = htonl(rc);

	if(send(sockfd,&conv, sizeof(conv),0) < 0)  //将conv copy到sockfd的数据缓冲区
	{
		perror("send()error");
		return -1;
	}
return 0;

}

int recv_cmd(int socket_control,char *cmd,char *arg)
{
                int num= 0 ;
		int rc = 200;
		char buffer[MAXSIZE];

		bzero(buffer,MAXSIZE);
		bzero(cmd,5);
		bzero(arg,MAXSIZE);
num = recv_data(socket_control,buffer);
		if(num == -1)
		{
				perror("recv_data()error");
				return -1;
		}

		strncpy(cmd,buffer,4);
		char *temp =buffer+5;
		strcpy(arg,temp);
		if(strcmp(cmd,"QUIT") == 0)
				rc = 221;
		else if(strcmp(cmd,"USER") == 0 || strcmp(cmd,"PASS") == 0 || strcmp(cmd,"LIST") == 0 || strcmp(cmd,"RETR")==0)
				rc = 200;
		else 
				rc = 502;
		send_response(socket_control,rc);
		return rc;
}
int socket_conn(int port,char*host)
{ 

        int sockfd;  					
	struct sockaddr_in dest_addr;

	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{ 
        	perror("error creating socket");
        	return -1;
    }

	
	bzero(&dest_addr,sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);

	dest_addr.sin_addr.s_addr = inet_addr(host);

	
	if(connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0 )
	{

                 perror("error connecting to server");
		return -1;
    }    
	return sockfd;
		
}

int start_data_conn(int socket_control)
{
		char buf[1024];
		int sock_data,wait = 0;
		if((recv(socket_control,&wait,sizeof(wait),0) )< 0)
		{
				perror("error while waiting");
				return -1;
		}

	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	getpeername(socket_control,(struct sockaddr *)&client_addr,&len);
	inet_ntop(AF_INET,&client_addr.sin_addr,buf,sizeof(buf));
    
	if((sock_data = socket_conn(CLIENT_PORT_ID,buf)) < 0)
	{
			return -1;

	}
	return sock_data;

}
int server_list(int socket_data,int socket_control)
{
		char data[MAXSIZE];
		size_t num_read;
		FILE *fp;
		int rs = system("ls -l | tail -n+2 > temp.txt");
		if(rs < 0)
		{
				exit(0);
		}
		fp = fopen("temp.txt","r");
		if(!fp)
				exit(1);
		fseek(fp,SEEK_SET,0);
		send_response(socket_control,1);
		bzero(data,MAXSIZE);
		while((num_read = fread(data,1,MAXSIZE,fp)) > 0)
		{
				if(send(socket_data,data,num_read,0) < 0)
				{
						perror("error!");
				}
				bzero(data,MAXSIZE);
		}
		fclose(fp);
		send_response(socket_control,226);
		return 0;
}

int file_size(char *filename)
{
	 struct stat statbuf;
	stat(filename,&statbuf);
	int size = statbuf.st_size;
 //printf("file_size%d",size);
	return size;	
}
void server_retr(int socket_control,int socket_data,char *filename)
{
		FILE*fp = NULL;
		int size;
		char *data[MAXSIZE];
		size_t num_read;
		fp = fopen(filename,"r");
		if(!fp)
				send_response(socket_control,550);
		/*else
		{
				send_response(socket_control,150);
				do{
						num_read = fread(data,1,MAXSIZE,fp);
						if(num_read < 0)
								printf("error in fread()\n");
						if(send(socket_data,data,num_read,0) < 0)
								perror("error send file");
				}while(num_read >0);
				send_response(socket_control,226);
		}
		fclose(fp);*/



		else
		{
				send_response(socket_control,150);
				size = file_size(filename);
printf("size%d\n",size);
				send(socket_data,&size,sizeof(int),0);
				do{
						num_read = fread(data,sizeof(char),MAXSIZE,fp);
						if(num_read < 0)
								printf("error in fread()\n");
						if(send(socket_data,data,num_read,0) < 0)
								perror("error send file");
				}while(num_read >0);
				send_response(socket_control,226);
		}
		fclose(fp);
}


void server_process(int sock_control)
{
	int sock_data;
	char cmd[5];
	char arg[MAXSIZE];

	send_response(sock_control, 220); 

	
	if (server_login(sock_control) == 1)
		send_response(sock_control, 230);
                
	else 
	{
		send_response(sock_control, 430);
	
		exit(0);
	}	
	
	
	while (1) 
	{
		
		int rc = recv_cmd(sock_control, cmd, arg);
		
		if ((rc < 0) || (rc == 221))  
			break;
		
		if (rc == 200 ) 
		{
			if ((sock_data = start_data_conn(sock_control)) < 0) 
			{
				close(sock_control);
				exit(1); 
			}

			/* 执行指令 */
			if (strcmp(cmd, "LIST")==0) 
				server_list(sock_data, sock_control);
			
			else if (strcmp(cmd, "RETR")==0) 
				server_retr(sock_control, sock_data, arg);

			close(sock_data);// 关闭连接
		} 
	}
}


int main(int argc,char *argv[])
{
	int socket_listen,port,socket_control,pid;
	if (argc != 2)
	{
		printf("usage: ./ftserve port\n");
		exit(0);
	}

	
	port = atoi(argv[1]);

	if((socket_listen=creat_socket(port))<0) //创建监听套接字
	{
		perror("creating socket error");
		exit(1);
	}
	while(1) //循环监听
	{
		if((socket_control=socket_accept(socket_listen))<0) //监听得到控制套接字，传递控制信息
		{
			break;
		}
	         if ((pid = fork()) < 0) 
			perror("Error forking child process");
	
		
		else if (pid == 0)
		{ 
			close(socket_listen);    //引用减一
			server_process(socket_control);		
			close(socket_control);
			exit(0);
		}
			
		close(socket_control);
		
	}
	close(socket_listen);
	return 0;
}

