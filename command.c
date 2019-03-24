#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "command.h"
#include <string.h>
#include <fcntl.h>
#include<sys/un.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<stdlib.h>
#include<pthread.h>
	int dir()
	{
		DIR *dp;
		struct dirent *entry;
		struct stat statbuf;
		char buf[256]="";
		getcwd(buf,sizeof(buf));
		dp=opendir(buf);
		while((entry=readdir(dp))!=NULL)
		{
			lstat(entry->d_name,&statbuf);
			if(S_ISDIR(statbuf.st_mode))
			{
				if(strcmp(".",entry->d_name)==0||strcmp("..",entry->d_name)==0)
				{
					continue;
				}
				printf("dir:%s\n",entry->d_name);
			}
			else{
				printf("file:%s\n",entry->d_name);
			}
		}
		closedir(dp);
		return 1;
	}

	int cd(const char* dest)
	{
    	    if(chdir(dest)==0)
		{
			return 1;
		}else
		{
			printf("路径不存在\n");
			return -1;
		}
	}

	int pwd()
	{
		char buf[256]="";
		if(getcwd(buf,sizeof(buf))==NULL)
		{
			printf("getcwd failed");
			return -1;
		}
		printf("%s\n",buf);
		memset(buf,0,sizeof(buf));
		return 1;
	}

	int creat_dir(const char* dir_name){
  	char tempdir[128]="";
  	char buf[128]="";
   	char *predir=dir_name;
  	int step=0;
  	char tempbuf[256]="";
   	getcwd(tempbuf,sizeof(tempbuf));
   	if(*dir_name=='/'){
        	//绝对路径
          chdir("/home");
          *predir++;
    	}
   	 while(1)
	{
        //相对路径   
         	if(*predir=='/'||*predir=='\0'){
         	strncpy(tempdir,predir-step,step);
        	 step=0;
         	if(*predir!='\0')
        	{
            	 	predir++;
         	}
        	if(getcwd(buf,sizeof(buf))!=NULL)
		{
	    		strcat(buf,"/");
            		strcat(buf,tempdir);
            	if(opendir(buf)==NULL)
		{
             		 mkdir(buf,0777);
              		 chdir(buf);
           		 //printf("buf:%s\n",buf); 
            	 }
            	else{
              chdir(buf);
              getcwd(buf,sizeof(buf));
             //printf("buf2:%s\n",buf);
            }
	} 
         memset(tempdir,0,128);
         if(*predir=='\0')  break;
     	 }
      
     	step++;
     	predir++;
    
   	}
   	chdir(tempbuf);
   	printf("创建成功，当前目录为:%s\n",tempbuf);
   	return 1;
	}

	int delete_dir(const char *dir_name)
	{
 	 char buf[128]="";
  	if(*dir_name!='/')
  	{
  	  getcwd(buf,sizeof(buf));
   	 strcat(buf,"/");
  	}
  	strcat(buf,dir_name);
  	DIR *dp;
  	struct dirent *entry;
  	struct stat statbuf;
  	if((dp=opendir(dir_name))!=NULL)
  	{
  	   chdir(buf);
   	  while((entry=readdir(dp))!=NULL)
   	  {
		//printf("获取目录信息\n");
     	   lstat(entry->d_name,&statbuf);
       	   if(S_ISDIR(statbuf.st_mode)){
           if(strcmp(".",entry->d_name)==0||strcmp("..",entry->d_name)==0)
              continue;
           //printf("即将递归\n");
           strcat(buf,"/");
           strcat(buf,entry->d_name);
           delete_dir(buf);
           //printf("递归结束\n");
	 }
        else{
           
           strcat(buf,"/");
           strcat(buf,entry->d_name);
           unlink(buf);
        }
        
     	}
     	 rmdir(buf);
    	 chdir("..");
     	 closedir(dp);
  	}
  	else{
	printf("无法删除:'%s'，目录不存在\n",dir_name);
        return -1;
        }
  	rmdir(dir_name);
  	return 1;
	}


int ls(char* show)
{
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	char buf[256]="";
	getcwd(buf,sizeof(buf));
	dp=opendir(buf);
	while((entry=readdir(dp))!=NULL)
	{
		lstat(entry->d_name,&statbuf);
		if(S_ISDIR(statbuf.st_mode))
		{
			if(strcmp(".",entry->d_name)==0||strcmp("..",entry->d_name)==0)
			{
				continue;
			}
			strcat(show,"dir:");
			strcat(show,entry->d_name);
			//strcat(show,"	-->dir	");
			strcat(show,"\n");
		}
		else{
			strcat(show,"file:");
			strcat(show,entry->d_name);
			//strcat(show,"	-->file	");
			strcat(show,"\n");
		}
	}
	closedir(dp);
	return 1;
}
int client_put(int sockfd,char* msg){
			char param1[256]="";
			char param2[256]="";
			char param3[6]="";
			char tempparam2[256]="";
			char tempparam3[6]="";			
			char *ptr=msg;
			ptr+=4;
			int step=0;
			int arg=1;
			//int opfd; 
			int nread;
			//char filesize[10];
			struct message{
				char end;
				char filebuf[256];			
			};
			struct message sendmsg;                            
			while(1){
				if(*ptr==' '&&arg==1){
					strncpy(param1,ptr-step,step);
					step=0;
					arg++;
					ptr++;
				}
				if(*ptr==' '&&arg==2){
					strncpy(param2,ptr-step,step);
					step=0;
					arg++;
					ptr++;
				}
				if(*ptr=='\0'){
					strncpy(param3,ptr-step,step);
					break;
				}
				ptr++;
				step++;
			}
			//test
			//printf("param1:%s,param2:%s,param3:%s\n",param1,param2,param3);
			strcpy(tempparam2,param2);
			strcpy(tempparam3,param3);
			
			int opfd; 
			if((opfd=open(param1,O_RDWR))<0)
			{
				printf("[client] open transpoting file failure\n");
			}
			//read from file
			while((nread=read(opfd,sendmsg.filebuf,256))>0){
				if(nread==256){
					sendmsg.end='0';
					send(sockfd,&sendmsg.end,sizeof(sendmsg.end),0);
					send(sockfd,sendmsg.filebuf,sizeof(sendmsg.filebuf),0);
				}
				else{
					char lastread[10]="";
					sprintf(lastread,"%d",nread);
					sendmsg.end='1';
					char lastread1[10]="2";
					strcat(lastread1,lastread);
					send(sockfd,lastread1,sizeof(lastread1),0);
					send(sockfd,&sendmsg.end,sizeof(sendmsg.end),0);
					send(sockfd,sendmsg.filebuf,nread,0);
					//printf("nread:%d\n",nread);
					//printf("trans:%s\n",lastread1);
					break;
				}
			}
			memset(param1,0,sizeof(param1));
			memset(param2,0,sizeof(param2));
			memset(param3,0,sizeof(param3));
			memset(tempparam2,0,sizeof(tempparam2));
			memset(tempparam3,0,sizeof(tempparam3));
			memset(ptr,0,sizeof(ptr));
			close(opfd);

			
			return 1;
}

int server_get(int sockfd,char* msg){
			char param1[256]="";
			char param2[256]="";
			char param3[6]="";
			char tempparam2[256]="";
			char tempparam3[6]="";			
			char *ptr=msg;
			ptr+=4;
			int step=0;
			int arg=1;
			//int opfd; 
			int nread;
			//char filesize[10];
			struct message
			{
				char end;
				char filebuf[256];			
			};
			struct message sendmsg;                            
			while(1){
				if(*ptr==' '&&arg==1){
					strncpy(param1,ptr-step,step);
					step=0;
					arg++;
					ptr++;
				}
				if(*ptr==' '&&arg==2){
					strncpy(param2,ptr-step,step);
					step=0;
					arg++;
					ptr++;
				}
				if(*ptr=='\0'){
					strncpy(param3,ptr-step,step);
					break;
				}
				ptr++;
				step++;
			}
			//test
			//printf("param1:%s,param2:%s,param3:%s\n",param1,param2,param3);
			strcpy(tempparam2,param2);
			strcpy(tempparam3,param3);

			FILE *fp;
			char user_temp[256]="";
			char user_info[100]="";
			int can_download=0;
			getcwd(user_temp,sizeof(user_temp));
			strcat(user_temp,"/authority");
			fp=fopen(user_temp,"r");
			if(NULL==fp){
				printf("open count_all error\n");	
			}
			while(fgets(user_info,100,fp)!=NULL)
			{
				if(strncmp(user_info,param1,strlen(param1))==0)
				{
					char *pointer=user_info;
					pointer=pointer+strlen(param1)+1;
					//recommand printf to recheck
					if(0==strncmp(pointer,"admin1",6)||0==strncmp(pointer,param3,6))
					{
						can_download=1;					
					}
					//printf("*pointer:%s  can_download:%d\n",pointer,can_download);
				}

			}
			
			fclose(fp);
			
			if(can_download==0){
				//printf("sorry you are not authorized to download this file\n");
				return -1;
			}
			int opfd; 
			if((opfd=open(param1,O_RDWR))<0){
				printf("[client] open transpoting file failure\n");
				//continue;
			}
			//read from file
			while((nread=read(opfd,sendmsg.filebuf,256))>0){
				if(nread==256){
					sendmsg.end='0';
					send(sockfd,&sendmsg.end,sizeof(sendmsg.end),0);
					send(sockfd,sendmsg.filebuf,sizeof(sendmsg.filebuf),0);
				}
				else{
					char lastread[10]="";
					sprintf(lastread,"%d",nread);
					sendmsg.end='1';
					char lastread1[10]="2";
					strcat(lastread1,lastread);
					send(sockfd,lastread1,sizeof(lastread1),0);
					send(sockfd,&sendmsg.end,sizeof(sendmsg.end),0);
					send(sockfd,sendmsg.filebuf,nread,0);
					//printf("nread:%d\n",nread);
					//printf("trans:%s\n",lastread1);
					
					break;
				}
			}
			memset(param1,0,sizeof(param1));
			memset(param2,0,sizeof(param2));
			memset(param3,0,sizeof(param3));
			memset(user_temp,0,sizeof(user_temp));
			memset(user_info,0,sizeof(user_info));
			memset(tempparam2,0,sizeof(tempparam2));
			memset(tempparam3,0,sizeof(tempparam3));
			memset(sendmsg.filebuf,0,sizeof(sendmsg.filebuf));
			close(opfd);

			
			return 1;
}

int server_put(int sockfd,char* msg){
			char *ptr=msg;
			ptr+=4;
			char param1[256]="";
			char param2[256]="";
			char param3[6]="";
			char tempparam2[256]="";
			char tempparam3[6]="";	
			int step=0;
			int arg=1;
			int recnum;
			char end;
			int num;
			char filebuf[256]="";
			while(1){
				if(*ptr==' '&&arg==1){
					strncpy(param1,ptr-step,step);
					step=0;
					arg++;
					ptr++;
				}
				if(*ptr==' '&&arg==2){
					strncpy(param2,ptr-step,step);
					step=0;
					arg++;
					ptr++;
				}
				if(*ptr=='\0'){
					strncpy(param3,ptr-step,step);
					break;
				}
				ptr++;
				step++;
			}
			//test
			//printf("param1:%s,param2:%s,param3:%s\n",param1,param2,param3);
			int fd;
			//puts(param2);
			strcpy(tempparam2,param2);
			strcpy(tempparam3,param3);
			if((fd=open(param2,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR))<0){
				puts("open file error\n");
			}
			
			//recv(client_sockfd,tempfilebuf,256,0)
			while(1){
				
				recv(sockfd,&end,sizeof(end),0);
				//puts("test1");
				//printf("end:%c\n",end);
				if(end=='0'){
				
					recv(sockfd,filebuf,sizeof(filebuf),0);
					//printf("%s",filebuf);
					write(fd,filebuf,sizeof(filebuf));
					//puts("test2\n");
				}
				else {
					//printf("AAAAA\n");
					char lastread[10]="";
				
					//memset(filebuf,' ',sizeof(filebuf));
					recv(sockfd,lastread,sizeof(lastread),0);
					printf("file transforming completed!  ps:lastread:%s\n",lastread);
					num=atoi(lastread);
					//printf("num:%d",num);				
					recv(sockfd,filebuf,num,0);
					write(fd,filebuf,num);	
					goto p1;

								
				}
				
			}
			p1:close(fd);
			memset(filebuf,0,sizeof(filebuf));
			memset(param1,0,sizeof(param1));
			memset(param2,0,sizeof(param2));
			memset(param3,0,sizeof(param3));

			FILE *p;
			char user_temp[256]="";
			getcwd(user_temp,sizeof(user_temp));
			strcat(user_temp,"/authority");
			p=fopen(user_temp,"a+");
			if(NULL==p){
				printf("open count_all error\n");	
			}
			strcat(tempparam2,":");
			strcat(tempparam2,tempparam3);
			strcat(tempparam2,"\n");
			fprintf(p,"%s",tempparam2);
			fclose(p);
			memset(tempparam2,0,sizeof(tempparam2));
			memset(tempparam3,0,sizeof(tempparam3));
			memset(user_temp,0,sizeof(user_temp));
			
			return 1;
}
int client_get(int sockfd,char* msg){
			char *ptr=msg;
			ptr+=4;
			char param1[256]="";
			char param2[256]="";
			char param3[6]="";
			char tempparam2[256]="";
			char tempparam3[6]="";	
			int step=0;
			int arg=1;
			int recnum;
			char end;
			int num;
			char filebuf[256]="";
			
			
			while(1){
				if(*ptr==' '&&arg==1){
					strncpy(param1,ptr-step,step);
					step=0;
					arg++;
					ptr++;
				}
				if(*ptr==' '&&arg==2){
					strncpy(param2,ptr-step,step);
					step=0;
					arg++;
					ptr++;
				}
				if(*ptr=='\0'){
					strncpy(param3,ptr-step,step);
					break;
				}
				ptr++;
				step++;
			}
			//test
			//printf("test2\n");
			//printf("param1:%s,param2:%s,param3:%s\n",param1,param2,param3);
			int fd;
			//puts(param2);
			strcpy(tempparam2,param2);
			strcpy(tempparam3,param3);
			if((fd=open(param2,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR))<0){
				puts("open file error\n");
			}
			
			//recv(client_sockfd,tempfilebuf,256,0)
			while(1){
				//puts("test0.1\n");
				recv(sockfd,&end,sizeof(end),0);
				//puts("test1\n");
				//printf("end:%c\n",end);
				if(end=='3'){
					remove(param2);
					memset(tempparam2,0,sizeof(tempparam2));
					memset(tempparam3,0,sizeof(tempparam3));
					memset(filebuf,0,sizeof(filebuf));
					memset(param1,0,sizeof(param1));
					memset(param2,0,sizeof(param2));
					return -1;
				}
				else if(end=='0'){
				
					recv(sockfd,filebuf,sizeof(filebuf),0);
					//printf("%s",filebuf);
					write(fd,filebuf,sizeof(filebuf));
					//puts("test2\n");
				}
				else {
					
					char lastread[10]="";
					recv(sockfd,lastread,sizeof(lastread),0);
					printf("file transforming completed!  ps:lastread:%s\n",lastread);
					num=atoi(lastread);
					//printf("num:%d",num);				
					recv(sockfd,filebuf,num,0);
					write(fd,filebuf,num);	
					
					break;

								
				}
				
			}
			close(fd);
			memset(tempparam2,0,sizeof(tempparam2));
			memset(tempparam3,0,sizeof(tempparam3));
			memset(filebuf,0,sizeof(filebuf));
			memset(param1,0,sizeof(param1));
			memset(param2,0,sizeof(param2));
			return 1;
}

