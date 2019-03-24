#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<stdio.h>
#include<sys/un.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<dirent.h>
#include<pthread.h>
#include"command.h"



int client_sockfd;
int count=0;
int count_all;
pthread_mutex_t mutex;
int kill_flag=0;
char users[5][7];
int online[5]={0,0,0,0,0};
//char current_user[6]=""
static void *server_control(void *arg){

	char msg[256]="";
	while(1){
		gets(msg);
		if(strcmp("",msg)==0){}
		else if(strncmp(msg,"count current",13)==0){
			printf("the number of current user is:%d\n",count);	
		}
		else if(strncmp(msg,"count all",9)==0){
			FILE *p;
			char user_temp[256]="";
			char count[3]="";
			getcwd(user_temp,sizeof(user_temp));
			strcat(user_temp,"/count_all");
			p=fopen(user_temp,"r");
			if(NULL==p){
				printf("open count_all error\n");	
			}
			fgets(count,3,p);
			printf("The total visit time is %s\n",count);
			fclose(p);
		}
		else if(strncmp(msg,"list",4)==0){
			int haveuser=0;
			for(int j=0;j<5;j++){
				if(1==online[j])
				{
				printf("online user:%s\n",users[j]);
				haveuser=1;
				}			
			}
			if(0==haveuser)
			{
				printf("No user online!\n");			
			}
		}
		else if(strncmp(msg,"kill",4)==0){
			char *p1=msg;
			p1+=5;
			if(strncmp(p1,"admin1",6)==0){
				printf("You are not allowed to delete admin1!\n");
				continue;			
			}
			FILE *p;
        		char user_info[100]="";
        		char user_temp[256]="";
			char new_file[256]="";
			char test_have_user[256]="";
			getcwd(user_temp,sizeof(user_temp));
			strcat(user_temp,"/user_info");
			p=fopen(user_temp,"r");
			if(NULL==p){
				printf("open user_info error\n");	
			}
			
			while(fgets(user_info,100,p)!=NULL)
			{
				strcat(test_have_user,user_info);
				if(strncmp(user_info,p1,6)!=0){
					strcat(new_file,user_info);
				}
			
				
			}
			if(fclose(p)) {printf("file close error!\n");}
				if(strcmp(test_have_user,new_file)==0)
				{
					printf("Username doesn't exist!Check again!\n");
				}
				else
				{
					printf("kill user succeeded!\n");
				}
			FILE *fp;
			if((fp=fopen(user_temp,"wa"))==NULL)
    			{
       				 printf("open file error\n");

    			}
			fprintf(fp,"%s",new_file);
   			if(fclose(fp)) {printf("file close error!\n");}
			
		}
		else if(strncmp(msg,"quit",4)==0)
		{
			exit(0);
		}
		else
		{
			printf("command not found\n");
		}
	}
}
static void* thread(void* arg){
	//char buf[256]="";
	char msg[256]="";
	FILE *p;
	char user_temp[256]="";
	char counts[3]="";
	getcwd(user_temp,sizeof(user_temp));
	strcat(user_temp,"/count_all");
	p=fopen(user_temp,"r");
	if(NULL==p){
		printf("open count_all error\n");	
	}
	fgets(counts,3,p);
	count_all=atoi(counts);
	fclose(p);
	FILE *fp;

	if(pthread_mutex_lock(&mutex)!=0){printf("lock failed\n");}

	count++;
	count_all++;
	if((fp=fopen(user_temp,"wa"))==NULL){printf("open file error\n");}
	fprintf(fp,"%d",count_all);
	if(fclose(fp)) {printf("file close error!\n");}

	if(pthread_mutex_unlock(&mutex)!=0){printf("unlock failed\n");}



	while(1){
		read(client_sockfd,msg,sizeof(msg));
		if(strcmp(msg,"")==0){}
		else if(strncmp(msg,"pwd",3)==0){
			char pwd[256]="";
			if(getcwd(pwd,sizeof(pwd))!=NULL)
			{
				write(client_sockfd,pwd,sizeof(pwd));
			}
			memset(pwd,0,sizeof(pwd));
		}
		else if(strncmp(msg,"cd",2)==0){
			char* dest;
			dest=msg;
			dest=dest+3;
			cd(dest);
			memset(dest,0,sizeof(dest));
		}
		else if(strncmp(msg,"ls",2)==0){
			char show[256]="";
			ls(show);
			write(client_sockfd,show,sizeof(show));
			memset(show,0,sizeof(show));
		}

		else if(strncmp(msg,"mkdir",5)==0){
			char* dest;
			dest=msg;
			dest=dest+6;
			creat_dir(dest);
			memset(dest,0,sizeof(dest));
		}
		else if(strncmp(msg,"rmdir",5)==0){
			char* dest;
			dest=msg;
			dest=dest+6;
			delete_dir(dest);
			memset(dest,0,sizeof(dest));
		}
		else if(strncmp(msg,"quit",4)==0)
		 {	//count--;
			char temp[2]="";
			sprintf(temp,"%c",msg[4]);
			online[atoi(temp)]=0;
			break;
		 }
		else if(strncmp(msg,"put",3)==0){
			server_put(client_sockfd,msg);
		}
		else if(strncmp(msg,"get",3)==0){
			int k;
			if((k=server_get(client_sockfd,msg))==-1){
				char fail='3';
				send(client_sockfd,&fail,sizeof(fail),0);
			}
			//printf("%d\n",k);
		}
		else{
			printf("command not found\n");
		}
		/*strcat(msg,"recevied message:");
		strcat(msg,buf);
		write(client_sockfd,msg,sizeof(msg));*/
		memset(msg,0,sizeof(msg));
		
	}
	pthread_mutex_lock(&mutex);
	count--;
	pthread_mutex_unlock(&mutex);
	close(client_sockfd);
	pthread_exit(NULL);

}

int main()
{
	int server_sockfd;
	int server_len,client_len;
	pthread_t tid;
	pthread_t tid2;
	//int flag=0;
	pthread_mutex_init(&mutex,NULL);
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	unlink("server_socket");
	server_sockfd=socket(AF_INET,SOCK_STREAM,0);
	server_address.sin_family=AF_INET;
	server_address.sin_addr.s_addr=inet_addr("127.0.0.1");
	server_address.sin_port=htons(9000);
	server_len=sizeof(server_address);
	pthread_create(&tid2,NULL,*server_control,NULL);
	bind(server_sockfd,(struct sockaddr*)&server_address,server_len);
	listen(server_sockfd,5);

//login varies
	FILE *p;
        char user_info[100]="";
        char user_temp[256]="";
	char notifyok[10]="";
	char notify_notok[10]="";
	//char user_input_temp[100]="";
        int flag=0;
	int user_num=0;
	getcwd(user_temp,sizeof(user_temp));
	strcat(user_temp,"/user_info");
	p=fopen(user_temp,"r");
	if(NULL==p){
		printf("[client]open user_info error\n");	
	}
//login
    while(1){
	printf("server waiting......\n");
	client_len=sizeof(client_address);   //client_address is unnecessary
	client_sockfd=accept(server_sockfd,(struct sockaddr*)&client_address,(socklen_t*)&client_len);
	  while(1){
			
			
			if(read(client_sockfd,user_temp,sizeof(user_temp))<0)
			{
				printf("[server]read error\n");
			}

		//save username	
				user_num=0;
			while(fgets(user_info,100,p)!=NULL)
			{
				strncpy(users[user_num],user_info,6);
				strcat(users[user_num],"\0");
			        user_num++;
			}
			fseek(p,0L,SEEK_SET);
				user_num=0;
		//get all users's uname&password
			while(fgets(user_info,100,p)!=NULL){
			     if(0==strncmp(user_temp,user_info,13))
			     {

			     	flag=1;
			     	online[user_num]=1;
			     	strcpy(notifyok,"OK");

			     break;
			     }
				user_num++;
			}
			fseek(p,0L,SEEK_SET);
			if(1==flag)  
			{
			char unum[2]="";
			sprintf(unum,"%d",count);
			strcat(notifyok,unum);
			write(client_sockfd,notifyok,sizeof(notifyok));
			break;
			}
			else{
			//printf("send notok\n");
			strcpy(notify_notok,"NOTOK");				
			write(client_sockfd,notify_notok,sizeof(notify_notok));
			}
		}
		pthread_create(&tid,NULL,*thread,(void *)&client_sockfd);
	}
	pthread_mutex_destroy(&mutex);

	return 0;
}
