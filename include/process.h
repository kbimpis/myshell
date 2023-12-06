#include <unistd.h>
#include <termios.h>
#pragma once

/*The category of a job*/
typedef enum JobType{
  NORMAL, CD, CREATE_ALIAS, DESTROY_ALIAS, SHOW_HISTORY, GET_HISTORY
}JobType;


/* A process is a single process.  */
typedef struct process{
  char **argv;                /* for exec */      
  struct process *next;       /* next process in pipeline */
} process;

/* A job is a pipeline of processes.  */
typedef struct job{
  struct job *next;           /* next active job */
  process *first_process;     /* list of processes in this job */
  JobType type;
  char isBackground;          /* whether it is a background job*/
} job;

process* init_process(char* process_string);
job* create_job();
void assign_process(job* job, process* proc);
void free_job(job*);
void free_remaining_jobs(job* old);
