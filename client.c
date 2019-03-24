#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/un.h>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<dirent.h>
#include"command.h"
char current_user[6]="";
int main()
{
	int sockfd;
	int len;
	char msg[256]="";
	int client_num;
	struct sockaddr_in address;
	int result;
	char buf[256]="";
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	address.sin_family=AF_INET;
	address.sin_port=htons(9000);	
	address.sin_addr.s_addr=inet_addr("127.0.0.1");
	len=sizeof(address);
	result=connect(sockfd,(struct sockaddr*)&address,len);
	if(result==-1)
	{
		printf("[client]connect() error\n");
		exit(1);
	}

//login
	while(1){
		printf("input username:password\n");
		gets(msg);
		if(write(sockfd,msg,sizeof(msg))<0){
		printf("[client]write error");
		}
		if(read(sockfd,buf,sizeof(buf))<0){  //potential problem maybe
		printf("[client]read error");
		}
		//printf("buf:%s\n",buf);
			if(strncmp(buf,"OK",2)==0){
			char temp[2]="";
			sprintf(temp,"%c",buf[2]);
			client_num=atoi(temp);
			printf("you are client %c\n",buf[2]);
			//printf("client_num assigned! %d\n",client_num);
			strncpy(current_user,msg,6);
			break;
			}
			if(strncmp(buf,"NOTOK",5)==0){
			printf("username or password error!\n");
			memset(buf,'0',sizeof(buf));		
			}
	}



	while(1){
		printf("myftp>");
		gets(msg);
//本地命令
		if(strcmp(msg,"")==0){}
		else if(strncmp(msg,"lpwd",4)==0){
			pwd();
		}
		else if(strncmp(msg,"lcd",3)==0){
			char* dest;
			dest=msg;
			dest=dest+4;
			cd(dest);
			memset(dest,0,sizeof(dest));
		}
		else if(strncmp(msg,"dir",3)==0){
			dir();
		}
		else if(strncmp(msg,"lmkdir",6)==0){
			char* dest;
			dest=msg;
			dest=dest+7;
			creat_dir(dest);
			memset(dest,0,sizeof(dest));
		}
		else if(strncmp(msg,"lrmdir",6)==0){
			char* dest;
			dest=msg;
			dest=dest+7;
			delete_dir(dest);
			memset(dest,0,sizeof(dest));
		}


//服务器命令
		else if(strncmp(msg,"mkdir",5)==0){
			if(result=write(sockfd,msg,sizeof(msg))<=0)
			{
				printf("[client]write error\n");
			}		
		}
		else if(strncmp(msg,"rmdir",5)==0){
			if(result=write(sockfd,msg,sizeof(msg))<=0)
			{
				printf("[client]write error\n");
			}
		}
		else if(strncmp(msg,"pwd",3)==0){
			if(result=write(sockfd,msg,sizeof(msg))<=0)
			{
				printf("[client]write error\n");
			}
			if(result=read(sockfd,buf,sizeof(buf))<0)
			{
				printf("[client]read error\n");
			}
			printf("%s\n",buf);
			memset(buf,0,256);
		}
		else if(strncmp(msg,"cd",2)==0){
			if(result=write(sockfd,msg,sizeof(msg))<=0)
			{
				printf("[client]write error\n");
			}
		}
		else if(strncmp(msg,"ls",2)==0){
			if(result=write(sockfd,msg,sizeof(msg))<=0)
			{
				printf("[client]write error\n");
			}
			if(result=read(sockfd,buf,sizeof(buf))<0)
			{
				printf("[client]read error\n");
			}
			printf("%s\n",buf);
			memset(buf,0,sizeof(buf));
		}


//put,get

		else if(strncmp(msg,"put",3)==0){
			strcat(msg," ");
			strcat(msg,current_user);
			if(result=write(sockfd,msg,sizeof(msg))<=0)
			{
				printf("[client]write error\n");
			}
			client_put(sockfd,msg);
		}
		else if(strncmp(msg,"get",3)==0){
			strcat(msg," ");
			strcat(msg,current_user);
			if(result=write(sockfd,msg,sizeof(msg))<=0)
			{
				printf("[client]write error\n");
			}
			if(-1==client_get(sockfd,msg)){
				printf("sorry you are not authorized to download this file\n");
			}
			//printf("sada\n");

		}


		else if(strncmp(msg,"quit",4)==0){
			char temp[2]="";			
			sprintf(temp,"%d",client_num);
			strcat(msg,temp);
			//printf("msg assigned! %s\n",msg);
			if(result=write(sockfd,msg,sizeof(msg))<=0)
			{
				printf("[client]write error\n");
			}
			memset(msg,0,sizeof(msg));
			memset(temp,0,sizeof(temp));
			break;
		}

		else{
			printf("command not found\n");
		}
		
		/*if(result=write(sockfd,msg,sizeof(msg))<=0)
		{
			printf("[client]write error\n");
		}
		if(result=read(sockfd,buf,sizeof(buf))<0)
		{
			printf("[client]read error\n");
		}
		printf("String from server=%s\n",buf);
		memset(msg,0,256);
		memset(buf,0,256);*/
		memset(msg,0,sizeof(msg));
	}

	close(sockfd);
	exit(0);
	return 0;
}
