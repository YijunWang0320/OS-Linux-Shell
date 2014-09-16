#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#define BUFFER 1024
#define PIPE_READ 0
#define PIPE_WRITE 1
int parse(char *cmd, int commandLen);
char *getCommand(int *inputLen);
int doCommand(char *cmd, char **arg);
int dopath(char *cmd, char **arg);
int doexec(char *cmd, char **arg);
int doPipeCommand(char **arg, int start, int place, int end, int in_fd);
int PipeCommand(char **arg, int start, int place, int end, int in_fd);
void errorHandler(void);
