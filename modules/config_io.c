#include "config_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


int isInput(char* token){
    if (token == NULL) return 0;
    int cmp = strcmp(token,"<");
    if (cmp == 0) return 1;
    else return 0;
}

int isOutput(char* token){
    if (token == NULL) return 0;
    int cmp = strcmp(token,">");
    if (cmp == 0) return 1;
    else return 0;
}

int isAppend(char* token){
    if (token == NULL) return 0;
    int cmp = strcmp(token,">>");
    if (cmp == 0) return 1;
    else return 0;
}

void redirect_input(const char* inFile){
    int fd = open(inFile,O_RDONLY);
    if (fd < 0){
        perror("open");
        exit(1);
    }

    dup2(fd,STDIN_FILENO);
    close(fd);    
}

void redirect_output(char* outFile){
    int fd = open(outFile,O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if (fd < 0){
        perror("open");
        exit(1);
    }

    dup2(fd,STDOUT_FILENO);
    close(fd);
}

void append_output(char* appendFile){
    int fd = open(appendFile,O_WRONLY | O_APPEND | O_CREAT, 0666);
    if (fd < 0){
        perror("open");
        exit(1);
    }

    dup2(fd,STDOUT_FILENO);
    close(fd);
}

void setupInput(process* proc){
    int i = 1;
    /*While we are in the current command*/
    while(proc->argv[i] != NULL){
        if (isInput(proc->argv[i])){
            redirect_input(proc->argv[i+1]);
            free(proc->argv[i]);
            proc->argv[i] = NULL;
        }
        i++;
    }
}

void setupOutput(process* proc){
    int i = 1;
    /*While we are in the current command*/
    while(proc->argv[i] != NULL){
        if (isOutput(proc->argv[i])){
            redirect_output(proc->argv[i+1]);
            free(proc->argv[i]);
            proc->argv[i] = NULL;
            /*There is no break command, because the final output is the one that matters*/
        }
        else if (isAppend(proc->argv[i])){
            append_output(proc->argv[i+1]);
            free(proc->argv[i]);
            proc->argv[i] = NULL;
        }
        i++;
    }

}

/*Closes all pipes, except for fd[index-1][READ] and fd[index][WRITE],
which are the pipes necessary for a process to communicate with the one before and the one after it*/
void closePipes(const int proc_index, const int nPipes, int **fd){
    for (int i=0; i<(proc_index-1); i++){
        close(fd[i][READ]);
        close(fd[i][WRITE]);
    }

    if (proc_index>0) close(fd[proc_index-1][WRITE]);

    /*The last command (proc_index = nPipes) has no pipe corresponding to it, so this is only needed for the rest*/
    if (proc_index < nPipes){
        close(fd[proc_index][READ]);

        for (int i=(proc_index+1); i<nPipes; i++){
            close(fd[i][READ]);
            close(fd[i][WRITE]);
        }
    }
}