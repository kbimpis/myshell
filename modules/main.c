#include <sys/wait.h>
#include "execute.h"
#include "shell.h"
#include "parse.h"

int main(int argc, char* argv[]){
    init_shell();

    int quit = 0;
    while (!quit){
        type_prompt();

        /*Waits for possible finished children, mainly of background processes*/
        waitpid(-1,NULL,WNOHANG);

        /*Reads input and splits it into different jobs*/
        job* firstJob = assign_jobs();
        job* current = firstJob;

        while (current){
            if (isQuit(current)){
                free_remaining_jobs(current);
                quit = 1;
                break;
            }
            execute_job(current);
            job *new = current->next;
            free_job(current);
            current = new;
        }
    }

    destroy_shell();
    return 0;
}