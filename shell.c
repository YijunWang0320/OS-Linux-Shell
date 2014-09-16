/*homework1 for os*/
/* Wang Yijun */
/*
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#define BUFFER 1024
#define PIPE_READ 0
#define PIPE_WRITE 1
int parse(char *cmd);
char *getCommand(int *inputLen);
int doCommand(char *cmd, char **arg);
int dopath(char *cmd, char **arg);
int doexec(char *cmd, char **arg);
int doPipeCommand(char **arg, int start, int place, int end, int in_fd);
int PipeCommand(char **arg, int start, int place, int end, int in_fd);
void errorHandler(void);
*/
#include"shell.h"
int pathnumber = 0;
struct pathlist {
	char *pathname;
	struct pathlist *next;
};
struct pathlist *pathHead;

int main(void)
{
	char *command = NULL;
	int commandLen = 0;
	int result = 0;

	pathHead = NULL;
	while (1) {
		printf("$");
		if (command)
			free(command);
		command = getCommand(&commandLen);
		result = parse(command, commandLen);
		if (result == 0)
			exit(0);
	}
}
int parse(char *cmd, int commandLen)
{
	char *cmdBuffer;
	char *tmpArg;
	char *arg[50];
	char cur;
	int start = 0;
	int argnum = 1;
	int i = 0;
	int flag = 0;

	cmdBuffer = (char *)malloc(sizeof(char)*commandLen);
	tmpArg = (char *)malloc(sizeof(char)*commandLen);
	if (cmd == NULL)
		return 1;
	cur = cmd[0];
	while (cur != ' ' &&  cur != '\0' && cur != '|') {
		cmdBuffer[start] = cmd[start];
		cur = cmd[start+1];
		start++;
	}
	cmdBuffer[start] = '\0';
	while (cmd[start] == ' ' || cmd[start] == '\t')
		start++;
	arg[0] = (char *)malloc(sizeof(char)*strlen(cmdBuffer));
	strcpy(arg[0], cmdBuffer);
	while (cmd[start] != '\0') {
		while (cmd[start] != ' ' && cmd[start] != '\t'
			&& cmd[start] != '|' && cmd[start] != '\0') {
			tmpArg[i++] = cmd[start++];
			flag = 1;
		}
		if (flag == 1) {
			arg[argnum] = (char *)malloc(sizeof(char)*(i+1));
			tmpArg[i] = '\0';
			strcpy(arg[argnum], tmpArg);
			argnum++;
			flag = 0;
			i = 0;
			continue;
		}
		if (cmd[start] == '|') {
			tmpArg[0] = '|';
			tmpArg[1] = '\0';
			arg[argnum] = (char *)malloc(sizeof(char)*(2));
			strcpy(arg[argnum], tmpArg);
			argnum++;
		}
		start++;
	}
	arg[argnum] = NULL;
	int ret = 0;
	int j = 0;
	int f = 0;

	for (j = 0; j < argnum; j++) {
		if (strcmp(arg[j], "|") == 0) {
			f = 1;
			break;
		}
	}
	if (f == 1) {
		int flag = 0;

		for (f = 0; f < argnum; f++) {
			if (strcmp(arg[f], "|") == 0 && !flag) {
				flag = 1;
				continue;
			} else if (strcmp(arg[f], "|") == 0 && flag) {
				flag = 10;
				break;
			}
			flag = 0;
		}
		if (flag == 10) {
			printf("error: two pipes together is not allowed\n");
			return 1;
		}
		ret = PipeCommand(arg, 0, j, argnum, STDIN_FILENO);
		for (i = 0; arg[i] != NULL; i++)
			free(arg[i]);
		return ret;
	}
	ret = doCommand(arg[0], arg);
	for (i = 0; arg[i] != NULL; i++)
		free(arg[i]);
	return ret;

}
char *getCommand(int *inputLen)
{
	char cur;
	char *tmp;
	char *buffer;
	int times = 1;

	buffer = (char *)malloc(sizeof(char)*BUFFER);
	(*inputLen) = 0;
	cur = getchar();
	if (cur == '\n')
		return NULL;
	while (cur == ' ' || cur == '\t')
		cur = getchar();
	while (cur != '\n') {
		if ((*inputLen) < BUFFER*times) {
			buffer[(*inputLen)++] = cur;
			cur = getchar();
		} else {
			times = times*2;
			buffer = realloc(buffer, BUFFER*times);
			buffer[(*inputLen)++] = cur;
			cur = getchar();
		}
	}
	/*if ((*inputLen) >= BUFFER) {
		printf("error: cannot process commandline over 1024\n");
		memset(buffer, 0, BUFFER);
		(*inputLen) = 0;
		return NULL;
	}*/
	buffer[(*inputLen)] = '\0';
	tmp = (char *)malloc(sizeof(char)*((*inputLen)+1));
	strcpy(tmp, buffer);
	free(buffer);
	return tmp;
}
int doCommand(char *cmd, char **arg)
{
	int pid;
	int rtValue = 1;

	if (strcmp(cmd, "exit") == 0)
		return 0;
	if (strcmp(cmd, "cd") == 0) {
		rtValue = chdir(arg[1]);
		if (rtValue == -1)
			errorHandler();
		return 1;
	}
	if (strcmp(cmd, "path") == 0) {
		rtValue = dopath(cmd, arg);
		return rtValue;
	}
	pid = fork();
	if (pid == 0) {
		rtValue = doexec(cmd, arg);
		if (rtValue < 0) {
			errorHandler();
			return 0;
		}
	} else if (pid > 0) {
		wait(0);
	} else {
		errorHandler();
		return 1;
	}
	return pid+1;
}
int dopath(char *cmd, char **arg)
{
	struct pathlist *tmp, *pre;
	int len;

	tmp = pathHead;
	if (arg[1] == NULL) {
		if (pathHead == NULL)
			return 1;
		while (tmp->next != NULL) {
			printf("%s:", tmp->pathname);
			tmp = tmp->next;
		}
		printf("%s\n", tmp->pathname);
		return 1;
	} else if (strcmp(arg[1], "+") == 0) {
		if (arg[2] == NULL) {
			printf("error: please give a path\n");
			return 1;
		}
		if (pathHead == NULL) {
			len = sizeof(struct pathlist);
			pathHead = (struct pathlist *)malloc(len);
			len = strlen(arg[2]);
			pathHead->pathname = (char *)malloc(sizeof(char)*len);
			strcpy(pathHead->pathname, arg[2]);
			pathHead->next = NULL;
			pathnumber++;
		} else {
			tmp = pathHead;
			while (tmp->next != NULL) {
				if (strcmp(tmp->pathname, arg[2]) == 0) {
					printf("error: path exists\n");
					return 1;
				}
				tmp = tmp->next;
			}
			if (strcmp(tmp->pathname, arg[2]) == 0) {
				printf("error: path exists\n");
				return 1;
			}
			len = sizeof(struct pathlist);
			tmp->next = (struct pathlist *)malloc(len);
			len = strlen(arg[2]);
			tmp->next->pathname = (char *)malloc(sizeof(char)*len);
			tmp = tmp->next;
			strcpy(tmp->pathname, arg[2]);
			tmp->next = NULL;
			pathnumber++;
		}
	} else if (strcmp(arg[1], "-") == 0) {
		if (arg[2] == NULL) {
			printf("error: please give a path\n");
			return 1;
		}
		if (pathHead == NULL) {
			printf("error: path not found\n");
			return 1;
		}
		if (strcmp(pathHead->pathname, arg[2]) == 0) {
			tmp = pathHead;
			pathHead = pathHead->next;
			free(tmp);
			pathnumber--;
			return 1;
		}
		tmp = pathHead;
		while (tmp->next != NULL) {
			if (strcmp(tmp->next->pathname, arg[2]) == 0) {
				pre = tmp->next;
				tmp->next = tmp->next->next;
				free(pre);
				pathnumber--;
				return 1;
			}
			tmp = tmp->next;
		}
		printf("error: path not found\n");
	} else {
		printf("error: invalid argument of path command\n");
	}
	return 1;
}
char *putTogether(char *a, char *b)
{
	int lena = strlen(a);
	int lenb = strlen(b);
	char *totalCmd = (char *)malloc(sizeof(char)*(lena+lenb+1));

	strcpy(totalCmd, a);
	totalCmd = strcat(totalCmd, "/");
	totalCmd = strcat(totalCmd, b);
	return totalCmd;
}
int doexec(char *cmd, char **arg)
{
	char *totalCmd;
	struct pathlist *tmp = pathHead;
	int flag = pathnumber;
	int i = 0;
	int ret = execv(cmd, arg);

	if (ret < 0 && pathnumber == 0)
		return ret;
	for (i = 0; i < pathnumber && tmp != NULL; i++) {
		totalCmd = putTogether(tmp->pathname, cmd);
		tmp = tmp->next;
		if (execv(totalCmd, arg) == -1)
			flag--;
	}
	return -1;
}
void fromTo(int from, int to)
{
	if (from != to) {
		if (dup2(from, to) == -1)
			errorHandler();
		else
			close(from);
	}
}
int noArgAfter(char **arg, int place, int end)
{
	int i = 0;

	for (i = place; arg[i] != NULL; i++)
		if (strcmp(arg[i], "|") == 0)
			return i;
	return -1;
}
int PipeCommand(char **arg, int start, int place, int end, int in_fd)
{
	int retValue = 1;
	int pid = fork();

	if (pid == 0) {
		retValue = doPipeCommand(arg, start, place, end, in_fd);
		return retValue;
	} else if (pid > 0) {
		wait(0);
		return 1;
	}
	errorHandler();
	return 1;
}
int doPipeCommand(char **arg, int start, int place, int end, int in_fd)
{
	int retValue = 0;
	int pid;
	int i = 0, j = 0;
	char *cmds[50];
	int p;
	int len;

	p = noArgAfter(arg, start, end);
	if (p == -1) {
		fromTo(in_fd, STDIN_FILENO);
		for (i = start; arg[i] != NULL; i++) {
			len = strlen(arg[i]+1);
			cmds[j] = (char *)malloc(sizeof(char)*len);
			strcpy(cmds[j], arg[i]);
			j++;
		}
		cmds[j] = NULL;
		retValue = doexec(cmds[0], cmds);
		if (retValue < 0) {
			errorHandler();
			return 0;
		}
	} else {
		int pfds[2];

		if (pipe(pfds) == -1) {
			errorHandler();
			return 0;
		}
		pid = fork();
		if (pid == -1) {
			errorHandler();
			return 0;
		} else if (pid == 0) {
			close(pfds[PIPE_READ]);
			fromTo(in_fd, STDIN_FILENO);
			fromTo(pfds[PIPE_WRITE], STDOUT_FILENO);
			for (i = start; i < p; i++) {
				len = strlen(arg[i]+1);
				cmds[j] = (char *)malloc(sizeof(char)*len);
				strcpy(cmds[j], arg[i]);
				j++;
			}
			cmds[j] = NULL;
			retValue = doexec(cmds[0], cmds);
			if (retValue < 0) {
				errorHandler();
				return 0;
			}
		} else {
			close(pfds[PIPE_WRITE]);
			close(in_fd);
			return doPipeCommand(arg, p+1,
				 place, end, pfds[PIPE_READ]);
		}
	}
	return 0;
}
void errorHandler(void)
{
	printf("error: %s\n", strerror(errno));
}
