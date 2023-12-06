#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "parse.h"
#include "alias.h"
#include "history.h"

/*Creates a child that calls "clear"*/
void clearTerminal(){
    pid_t pid = fork();
    switch (pid)
    {
        case -1:
            perror("fork");
            exit(1);
        case 0:
            execlp("clear","clear",NULL);
            perror("execlp");
            exit(1);
        default:
            waitpid(pid,NULL,0);
    }
}

/*Returns whether a job is the built-in "quit" function*/
int isQuit(job* job){
    if (!job) return 0;
    process *p = job->first_process;
    if (!p) return 0;
    if (!p->argv) return 0;
    if (!p->argv[0]) return 0;
    if ((strcmp(p->argv[0],"quit") == 0) && (!p->argv[1]))
        return 1;
    return 0;
}

/*To ignore SIGINT and SIGTSTP and continue the running of the shell.*/
void shell_handler(int signum){
    /*write is safer than printf in signal handlers */
    write(STDOUT_FILENO,"\n",1);
    write(STDOUT_FILENO,PROMPT,strlen(PROMPT)+1);
}

void type_prompt(){
    printf(PROMPT);
    fflush(stdout);
}

/*Initializes the signal handler, and some variables*/
void init_shell(){
    struct sigaction sa = {0};
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = shell_handler;
    sigaction(SIGINT,&sa,NULL);
    sigaction(SIGTSTP,&sa,NULL);

    clearTerminal();
    first = NULL;
    instantiate_history();
}

void destroy_shell(){
    free_aliases();
    free_history();
}