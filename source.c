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
int doCommand(char *cmd,char **arg);
int dopath(char *cmd,char **arg);
int doexec(char *cmd,char **arg);
int doPipeCommand(char **arg,int start,int place,int endNum);
int execPipe(char **arg,int startNum,int place,int endNum,int *pdfs);
char buffer[BUFFER];
char *pathlist[100];
int pathnumber=0;
int main()
{
	char* command=NULL;
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
        char *arg[50];
	char cur;
	int start=0;
	int argnum=1;
	if(cmd==NULL)
		return 1;
        cur=cmd[0];
	while(cur!=' ' &&  cur!='\0'){
		cmdBuffer[start]=cmd[start];
		cur=cmd[start+1];
		start++;
	}	
	cmdBuffer[start]='\0';
	
	while(cmd[start]==' '){
		start++;
 	}
	if(strcmp(cmdBuffer,"exit")==0){
		return 0;
	}
	arg[0]=(char*)malloc(sizeof(char)*strlen(cmdBuffer));
	strcpy(arg[0],cmdBuffer);
	int i=0;
	int flag=0;
	while(cmd[start]!='\0'){
		if(cmd[start]!=' '){
 			cur=cmd[start++];
			tmpArg[i++]=cur;
		}
		if(cmd[start]=='\0'){
			tmpArg[i]='\0';
			arg[argnum]=(char*)malloc(sizeof(char)*(i+1));
			strcpy(arg[argnum],tmpArg);
			argnum++;
			continue;
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
	int ret=0;
	int j=0;
	int f=0;
	for(j=0;j<argnum;j++){
		if(strcmp(arg[j],"|")==0){
			f=1;
			break;	
		}
	}
	if(f==1)
	{
		ret=doPipeCommand(arg,0,j,argnum);
		for(i=0;arg[i]!=NULL;i++)
			free(arg[i]);
		return ret;
	}
        ret=doCommand(arg[0],arg);
	for(i=0;arg[i]!=NULL;i++)
		free(arg[i]);
	return ret;

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
int doCommand(char *cmd,char **arg)
{
    	int pid;
	int rtValue=1;
	/*instruction   cd*/
	if(strcmp(cmd,"cd")==0){
		rtValue=chdir(arg[1]);
		if(rtValue==-1)
			perror("perror");
		return 1;
	}
	/*instruction  path*/
	if(strcmp(cmd,"path")==0){
		rtValue=dopath(cmd,arg);
		return rtValue;
	}
	/*other instructions*/ 
	pid=fork();	
	if(pid==0){
		rtValue=doexec(cmd,arg);
		if(rtValue<0)
		{
			perror("perror");
			return 0;
		}
	}
	else if(pid>0){
		wait(0);
	}
	else{
		perror("perror");
	}
	return pid+1;
}
int dopath(char *cmd,char **arg)
{
	int i;
	if(arg[1]==NULL){
		for(i=0;i<pathnumber-1;i++){
			printf("%s:",pathlist[i]);
		}
		if(i<pathnumber)
			printf("%s\n",pathlist[i]);
		return 1;
	}else if(strcmp(arg[1],"+")==0){
		if(arg[2]==NULL)
			return 1;
		pathlist[pathnumber]=
			(char*)malloc(sizeof(char)*strlen(arg[2]));
		strcpy(pathlist[pathnumber],arg[2]);
		pathnumber++;
		return 1;
	}else if(strcmp(arg[1],"-")==0){
		if(arg[2]==NULL)
			return 1;
		pathnumber--;
		free(pathlist[pathnumber]);
		pathlist[pathnumber]=NULL;
		return 1;
	}else{
		printf("error:please check your argument\n");
		return 1;
	}
}
char *putTogether(char* a,char *b)
{
	int lena=strlen(a);
	int lenb=strlen(b);
	char *totalCmd=(char*)malloc(sizeof(char)*(lena+lenb+1));
	strcpy(totalCmd,a);
	totalCmd=strcat(totalCmd,"/");
	totalCmd=strcat(totalCmd,b);
	return totalCmd;
}
int doexec(char *cmd,char **arg)
{
	char *totalCmd;
	int flag=pathnumber;
	int i=0;
	if(pathnumber==0){
		int ret=execv(cmd,arg);
		return ret;	
	}
	for(i=0;i<pathnumber;i++){
		if(flag!=pathnumber-i)
			break;
		totalCmd=putTogether(pathlist[i],cmd);
		if(execv(totalCmd,arg)==-1)
		{
			flag--;
		}
	}
	return flag-1;
}
int doPipeCommand(char **arg,int start,int place,int end)
{
	int pdfs[2];
	int pid;
	pid=fork();
	if(pipe(pdfs)==-1){
		perror("perror");
		return 1;
	}	
	if(pid>0){
		close(1);
		dup2(pdfs[1],1);
		close(pdfs[0]);
		wait(0);
	}else if(pid==0){
		int i=0,j=0;
		return execPipe(arg,start,place,end,pdfs);
	}else{
		perror("perror");
		return 1;
	}
}
int execPipe(char **arg,int startNum,int place,int endNum,int *pdfs)
{
	if(place==endNum){
		close(0);
		dup2(pdfs[1],1);
		close(pdfs[0]);
		return 1;	
	}	
	int i=0,j=0;
	int rtValue;
	int pid;
	char *tmparg[50];
	pid=fork();
	if(pid>0){	
		/*close(0);
		close(1);
		dup2(pdfs[0],0);
		dup2(pdfs[1],1);*/
		for(i=startNum;i<place;i++){
			tmparg[j]=(char*)malloc(sizeof(char)*strlen(arg[i]));
			strcpy(tmparg[j],arg[i]);
			j++;
		}
		tmparg[j]=NULL;
		rtValue=doexec(tmparg[0],tmparg);
		if(rtValue<0){
			for(i=0;tmparg[i]!=NULL;i++)
				free(tmparg[i]);
			perror("perror");
			return 1;
		}
	}else if(pid==0){
		int tmpPlace=endNum;
		j=0;
		for(i=place+1;i<endNum;i++){
			if(strcmp(arg[i],"|")==0){
				tmpPlace=i;
				break;
			}
		}
		return execPipe(arg,place+1,tmpPlace,endNum,pdfs);
	}else{
		perror("perror");
		return 1;
	}
}
