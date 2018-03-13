#include "ftclient.h"

int sock_control;

int read_reply()
{
		int retcode = 0;
		if(recv(sock_control,&retcode,sizeof(retcode),0) < 0)
		{
				perror("client:error reading message fron server");
				return -1;
		}
		return ntohl(retcode);
}

void print_reply(int rc)
{
		switch(rc)
		{
				case 220:
						printf("220 welcome,server ready\n");
						break;
				case 221:
						printf("221 Good bye!\n");
						break;
				case 226:
						printf("226 closing data connection,Request file successful!\n");
						break;
				case 550:
						printf("550 Requested action not taken,File unavailable\n");
						break;
		}
}

void read_input(char *buffer ,int size)
{
		char *temp = NULL;
		bzero(buffer,size);
		if(fgets(buffer,size,stdin) != NULL)
		{
				temp = strchr(buffer,'\n');  //查找字符串  返回位置指针
				if(temp)
						*temp = '\0';
		}
}

int client_send_cmd(struct command *cmd)   //发送密码
{
int len = 0,rc;
	char buffer[MAXSIZE];
	sprintf(buffer,"%s%s",cmd->code,cmd->arg);
        len = (int)strlen(buffer)+1;

	rc= send(sock_control,&len,sizeof(len),0);
	if(rc < 0)
	{
			perror("errro sending command length to server");
			return -1;
	}
        rc= send(sock_control,buffer,(int)strlen(buffer),0);
	if(rc < 0)
	{
			perror("errro sending command to server");
			return -1;
	}
	return 0;
}


void client_login()
{
	        struct command cmd;
		char user[256];

		 bzero(user,256);

		 printf("Name: ");
		 fflush(stdout);  //把缓冲区内容输出
		 read_input(user,256);

		
		 strcpy(cmd.code,"USER");
		 strcpy(cmd.arg,user);
		 client_send_cmd(&cmd);


		 int wait;
		 recv(sock_control,&wait,sizeof(wait),0);

		
		 char *pass = getpass("Password: ");  //密码不显示
		 strcpy(cmd.code,"PASS");
		 strcpy(cmd.arg,pass);
		 client_send_cmd(&cmd);


		int retcode = read_reply();
		switch(retcode)
		{
				case 430:
						printf("Invalid username/password\n");
						break;
				case 230:
						printf("Successsful login\n");
						break;
				default:
						perror("errro reading message from server\n");

						exit(1);
						break;
		}

}
int client_open_conn(int sock_con)
{
		 int sock_listen = creat_socket(CLIENT_PORT_ID);

		 int ack = 1;
		 if((send(sock_con,(char*)&ack,sizeof(ack),0))<0)
		 {

				 printf("client: ack write error:%d\n",errno);
				 exit(1);
		 }
		 int sock_conn = socket_accept(sock_listen);

		 close(sock_listen);
		 return sock_conn;

}

int client_read_cmd(char *buffer,int size,struct command *cmd)
{
	bzero(cmd->code,sizeof(cmd->code));
	bzero(cmd->arg,sizeof(cmd->arg));

	printf("ftclient>");
	fflush(stdout);

	read_input(buffer,size);
	char *arg = NULL;
	arg = strtok(buffer," ");

	arg = strtok(NULL," ");

	if(arg != NULL)
	{
			strncpy(cmd->arg,arg,strlen(arg));

	}
	if(strcmp(buffer,"list") == 0)
			strcpy(cmd->code,"LIST");
	else if(strcmp(buffer,"get") == 0)
			strcpy(cmd->code,"RETR");
	else if(strcmp(buffer,"quit") == 0)
			strcpy(cmd->code,"QUIT");
	else 
			return -1;
	bzero(buffer,400);
	strcpy(buffer,cmd->code);

	if(arg != NULL)
	{
			strcat(buffer," ");  //分隔
			strncat(buffer,cmd->arg,strlen(cmd->arg));
	}

	return 0;
}

int client_list(int sock_data,int sock_conn)
{
		size_t num_recvd;
		char buf[MAXSIZE];
		int temp = 0;

		if(recv(sock_conn,&temp,sizeof(temp),0) < 0)
		{
				perror("client: error reading message from server\n");
				return -1;
		}
		bzero(buf,MAXSIZE);
		while((num_recvd = recv(sock_data,buf,MAXSIZE,0)) > 0)
		{
				printf("%s",buf);
				bzero(buf,sizeof(buf));
		}
		if(num_recvd < 0)
				perror("error");
		if(recv(sock_conn,&temp,sizeof(temp),0) < 0)
		{
				perror("client:error reading message from sever\n");
				return -1;
		}
		return 0;
}

int client_get(int sock_data,int sock_conn,char *arg)
{
		int file_length,rc;
		char data[MAXSIZE];
		int size= 0;
		FILE *fd = fopen(arg,"w");
		rc = recv(sock_data,&file_length,sizeof(int),0);
		while(size != file_length)
		{
		   rc = recv(sock_data,data,MAXSIZE,0);
		    fwrite(data,1,rc,fd);;
		   size += rc; 
		}
//printf("size%d file%d\n",size,file_length);
		
		if(size != file_length)
				perror("recive length error\n");
		fclose(fd);
		return 0;
}
/*int client_get(int sock_data,int sock_conn,char *arg)
{
int file_length,rc;
		char data[MAXSIZE];
		int size;
		FILE *fd = fopen(arg,"w");
rc = recv(sock_data,&file_length,sizeof(int),0);
		while((size = recv(sock_data,data,MAXSIZE,0)) > 0)
				fwrite(data,1,size,fd);;
		if(size < 0)
				perror("error\n");
		fclose(fd);
		return 0;
}*/

int main(int argc, char* argv[]) 
{int num,rc;		
	int data_sock, retcode, s;
	char buffer[MAXSIZE];
        char req[MAXSIZE];
	struct command cmd;	
	struct addrinfo hints, *res, *rp;

	
	if (argc != 3)
	{
		printf("usage: ./ftclient hostname port\n");
		exit(0);
	}

	char *host = argv[1]; 
	char *port = argv[2]; 

	bzero(&hints,sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	s = getaddrinfo(host, port, &hints, &res);
	if (s != 0) 
	{
		printf("getaddrinfo() error %s", gai_strerror(s));
		exit(1);
	}
	
	
	for (rp = res; rp != NULL; rp = rp->ai_next) 
	{
		sock_control = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol); 

		if (sock_control < 0)
			continue;

		if(connect(sock_control, res->ai_addr, res->ai_addrlen)==0)   
			break;
		
		else 
		{
			perror("connecting stream socket");
			exit(1);
		}
		close(sock_control);
	}
	freeaddrinfo(rp);


	
        print_reply(read_reply()); 
	
	client_login();

	while (1) 
	{ 
		if ( client_read_cmd(buffer, sizeof buffer, &cmd) < 0)
		{
			printf("Invalid command\n");
			continue;	
		}
num = (int)strlen(buffer)+1;
//sprintf(req,"%4d%s",num,buffer);
rc= send(sock_control,&num,sizeof(num),0);
	if(rc < 0)
	{
			perror("errro sending command length to server");
			return -1;
	}

rc = send(sock_control, buffer, (int)strlen(buffer), 0);
//printf("copy %d\n",rc);
		
		if (rc < 0 )
		{

			close(sock_control);
			exit(1);
		}

		retcode = read_reply();	
		if (retcode == 221)  
		{
			print_reply(221);		
			break;
		}
		
		if (retcode == 502) 
			printf("%d Invalid command.\n", retcode);
		else 
		{			
			
			if ((data_sock = client_open_conn(sock_control)) < 0) 
			{
				perror("Error opening socket for data connection\n");
				exit(1);
			}			
			
			/* 执行命令 */
			if (strcmp(cmd.code, "LIST") == 0) 
				client_list(data_sock, sock_control);
			
			else if (strcmp(cmd.code, "RETR") == 0) 
			{
				if (read_reply() == 550) // 等待回复
				{
					print_reply(550);		
					close(data_sock);
					continue; 
				}
				client_get(data_sock, sock_control, cmd.arg);
				print_reply(read_reply()); 
			}
			close(data_sock);
		}

	} 
	close(sock_control); 
    return 0;  
}


