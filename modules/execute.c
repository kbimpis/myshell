#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#include "execute.h"
#include "config_io.h"
#include "alias.h"
#include "history.h"

/*Returns the number of all processes in a job*/
int get_proc_total(job* job){
    int nProcs = 0;
    process *proc = job->first_process;
    while (proc){
        nProcs++;
        proc = proc->next;
    }
    return nProcs;
}

/*Executes a job that is native to the shell,
and therefore does not need an exec() call*/
void execute_builtin(job* j){
    char* temp;
    char* value;
    process* p = j->first_process;
    switch(j->type)
    {
        case CD:
            chdir(p->argv[0]);
            return;
        case CREATE_ALIAS:
            create_alias(p->argv[0],p->argv[1]);
            return;
        case DESTROY_ALIAS:
            destroy_alias(p->argv[0]);
            return;
        case SHOW_HISTORY:
            showHistory();
            return;
    }
}
/*Tidy up after an exec error so that there are no leaks.
Functionally useless, but prevents valgrind error messages*/
void handle_exec_error(job* j){
    fprintf(stderr,"%s","Command not found.\n");
    free_remaining_jobs(j);
    free_history();
    free_aliases();
    exit(1);
}

/*The only difference from shell_handler is this one does not print the prompt*/
void job_handler(int signum){
    //write is safer than printf in signal handlers
    write(STDOUT_FILENO,"\n",1);
}

void setup_signals(){
    struct sigaction sa = {0};
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = job_handler;
    sigaction(SIGINT,&sa,NULL);
    sigaction(SIGTSTP,&sa,NULL);
}

/*Executes job, that is executes all processes inside the job, concurrently.
Sets up necessary pipes, and input/output redirection if needed.
For each non-built-in processes in the job, a child is created and the job-process is executed through an execvp() call in the child.
If the job is background, a proccess group id is set as the pid of the first child.*/
void execute_job(job* job){
    if (!job) return;

    setup_signals();

    if (job->type != NORMAL){
        execute_builtin(job);
        return;
    }

    // pid_t pgid = 0;
    process* p = job->first_process;
    if (!p->next){ //There is only a single process in this job       

        int pid = fork();
        if (pid < 0){
            perror("fork"); 
            exit(1);
        }

        if (pid == 0){
            if (job->isBackground){
                printf("\n[PID]: %d\n", getpid());
            }
            /*This order means that input operator ('<') must be before the output operator ('>' / '>>') else it will not get recognized*/
            setupOutput(p);
            setupInput(p);
            execvp(p->argv[0],p->argv);
            handle_exec_error(job);

        }
        if (job->isBackground){
            waitpid(pid,NULL,WNOHANG);
            return;
        }
        /*Wait for the child (which is a fg process) to end*/
        waitpid(pid,NULL,0); 
        return;
    }

    const int PROCESSES = get_proc_total(job);
    const int PIPES = PROCESSES - 1;

    /*Create necessary pipes. Each process (with index i) reads from the READ part of the previous (==fdPipe[i-1][0]) pipe,
    and writes into the WRITE part of its own pipe (==fdPipe[i][1])*/
    int **fdPipe = malloc(PIPES*sizeof(int*));
    for (int i=0; i<PIPES; i++)
        fdPipe[i] = malloc(2*sizeof(int));
    

    /*Create pipes for the communication between processes*/
    for (int i=0; i<PIPES; i++){
        if (pipe(fdPipe[i]) == -1){
            perror("pipe");
            exit(1);
        }
    }

    /*Now a child is created for each process of the job and executed.
    The first and last processes are executed differently, because in their cases there is the need to check for a possible input/output redirection.
    All other processes communicate only through pipes*/

    /*Create the first child*/
    pid_t pid = fork();

    if (pid < 0){
        perror("fork");
        exit(1);
    }
    if (pid == 0){
        closePipes(0,PIPES,fdPipe);

        if (p->next){
            dup2(fdPipe[0][WRITE],STDOUT_FILENO);
            close(fdPipe[0][WRITE]);
        }
        else setupOutput(p);

        if (job->isBackground){
            if (setpgid(0,0) == -1){
                perror("setpgid");
                exit(1);
            }
            printf("\n[PID]: %d\n", getpid());
        }

        setupInput(p);

        execvp(p->argv[0],p->argv);
        handle_exec_error(job);
    }

    /*Create and execute the middle children/processes*/
    int proc_index = 1;
    p = p->next;
    while(p->next){
        pid = fork();

        if (pid < 0){
            perror("fork");
            exit(1);
        }

        if (pid > 0){
            p = p->next;
            continue;
        }

        /*---------In child process---------*/

        /*Close all unnecessary pipes*/
        closePipes(proc_index,PIPES,fdPipe);

        /*Input is taken from the previous pipe*/
        dup2(fdPipe[proc_index-1][READ],STDIN_FILENO);
        close(fdPipe[proc_index-1][READ]);

        /*Output is written in this pipe*/
        dup2(fdPipe[proc_index][WRITE],STDOUT_FILENO);
        close(fdPipe[proc_index][WRITE]);

        execvp(p->argv[0],p->argv);
        handle_exec_error(job);

        /*------End of child process------*/
    }

    /*Create and execute the last child-process*/
    pid = fork();
    if (pid < 0){
        perror("fork");
        exit(1);
    }

    if (pid == 0){
        closePipes(proc_index,PIPES,fdPipe);

        /*Only the input is duplicated, output is as normal in stdout*/
        dup2(fdPipe[proc_index-1][READ],STDIN_FILENO);
        close(fdPipe[proc_index-1][READ]);

        /*There may be an output redirection to a file*/
        setupOutput(p);
        

        execvp(p->argv[0],p->argv);
        handle_exec_error(job);
    }

    for (int i=0; i<PIPES; i++){
        close(fdPipe[i][READ]);
        close(fdPipe[i][WRITE]);
    }

    /*Wait all processes to end before moving on with the next job*/
    if (!job->isBackground) while(wait(NULL) > 0);

    for (int i=0; i<PIPES; i++){
        free(fdPipe[i]);
    }
    free(fdPipe);
 
}