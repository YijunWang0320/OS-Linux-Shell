/*homework1 for os*/
/* Wang Yijun */
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<stdio.h>
#define BUFFER 1024
int parse(char* cmd);
char* getCommand(int* inputLen);
char buffer[BUFFER];
int main()
{
	char* command;
        int commandLen=0;
        int result=0;
	while(1){
        	printf("$>");
		if(command)
			free(command);
                command = getCommand(&commandLen);
	        result = parse(command);
                if(result == 0)
                	return;
   	}
        
}
int parse(char* cmd)
{
	char cmdBuffer[1024];
	char tmpArg[1024];
        char *arg[20];
	char cur;
	int start=0;
	int argnum=0;
	if(cmd==NULL)
		return 1;
        cur=cmd[0];
	while(cur!=' '){
		cmdBuffer[start]=cmd[start];
		cur=cmdBuffer[start];
		start++;
	}	
	cmdBuffer[start]='\0';
	while(cmd[start]==' '){
		start++;
 	}
	int i=0;
	int flag=0;
	while(cmd[start]!='\0'){
		if(cmd[start]!=' '){
 			cur=cmd[start++];
			tmpArg[i++]=cur;
		}
		if(cmd[start]==' ')
		{
			flag=1;
			start++;
		}
		if(flag && cmd[start]!=' '){
			tmpArg[i]='\0';
			arg[argnum]=(char*)malloc(sizeof(char)*(i+1));
			strcpy(arg[argnum],tmpArg);
			i=0;
			argnum++;
			flag=0;
		}
	}
	arg[argnum]=NULL;
	printf("%s",cmdBuffer);
	int j=0;
	while(arg[j]!=NULL){
		printf("%s",arg[j]);
		j++;
	}
        return 0;
}
char* getCommand(int *inputLen)
{
	char cur;
	char *tmp;
	(*inputLen)=0;
        cur=getchar();
	if(cur == '\n')
		return NULL;
	while(cur!='\n' && (*inputLen)<BUFFER){
		buffer[(*inputLen)++]=cur;
		cur=getchar();
	}
	if((*inputLen)>=BUFFER){
		printf("cannot process commandline over 1024,please try a shorter one\n");
		memset(buffer,0,BUFFER);
		(*inputLen)=0;
		return NULL;
	}
	else
		buffer[(*inputLen)]='\0';
	
	tmp=(char*)malloc(sizeof(char)*((*inputLen)+1));
	strcpy(tmp,buffer);
	memset(buffer,0,BUFFER);
	return tmp;
}


