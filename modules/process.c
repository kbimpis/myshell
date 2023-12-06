#include "process.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include "delims.h"
#include <glob.h>
#include "alias.h"
#include "history.h"


/*Counts the number of tokens (separated by DELIMS) appear inside a string*/
int countTokens(char* string, const char* DELIMS){
    char *tempString = malloc(strlen(string)+1);
    strcpy(tempString,string);

    char* token, *ptr;
    int nTokens = 0;
    token = strtok_r(tempString,DELIMS,&ptr);
    while (token){
        nTokens++;
        token = strtok_r(NULL,DELIMS,&ptr);
    }
    free(tempString);
    return nTokens;
}

/*Takes in a process string, and counts the number of words it will have after dereferencing possible globs*/
int getWordCount(const char *processString){
    char *ptr;

    /*Copy to temporary string so as not to alter the original*/
    char *tempString = malloc((strlen(processString)+1)*sizeof(char));
    strcpy(tempString,processString);

    char *token = strtok_r(tempString,WORD_DELIMS,&ptr);
    int wordCount = 1;

    /*Initialize globbuf instance*/
    glob_t globbuf;
    globbuf.gl_offs = 0;
    int globFlag = 0; //If I use globfree() without having called glob() first, program crashes

    /*Iterate through the string using strtok_r to count the number of words*/
    while (token = strtok_r(NULL,WORD_DELIMS,&ptr)){
        if (glob(token,0,NULL,&globbuf)){
            /*No match found*/
            wordCount++;
        }
        else{
            wordCount += globbuf.gl_pathc;
        }
        globfree(&globbuf);
    }
    free(tempString);
    return wordCount;
}

/*Takes in an environment variable, and returns its value*/
char* parseEnvVar(char* token){
    if (token[1] == '{'){
        /*token must be "${var-name}" */
        if (strlen(token) < 4) return NULL;
        char *ptrTemp;
        token += 2; //Excluding '$' and '{'
        token = strtok_r(token,"}",&ptrTemp); //Excludes '}'
    }
    else token++; //Only excludes '$'
    const char* path = getenv(token);
    char *returnPtr; 
    if (path){
        returnPtr = malloc((strlen(path)+1)*sizeof(char));
        strcpy(returnPtr,path);
    }
    else{
        returnPtr = malloc(sizeof(char*));
        returnPtr = NULL;
    }
    return returnPtr;
}

int isEnv(char* argument){
    return (argument[0] == '$');
}

/*Takes in a processString, and breaks it down into a string array, that is easily passable to execvp().
Dereferences possible environment variables and wild characters on the way*/
char** parseProcess(char* processString){
    if (!processString) return NULL;

    /*Initiate glob_t instance*/
    glob_t globbuf;
    globbuf.gl_offs = 0;

    /*Whether to call globfree()*/
    int globFlag = 0;

    /*Number of total tokens, to allocate enough space*/
    const int nTokens = getWordCount(processString);

    char *ptr2;
    char** argv = (char**)malloc((nTokens+1)*sizeof(char*));

    char *token = strtok_r(processString,WORD_DELIMS,&ptr2);

    int index = 0;
    while (token){
        if (isEnv(token)){
            argv[index++] = parseEnvVar(token);

            /*To prevent further indenting due to else() statement*/
            token = strtok_r(NULL,WORD_DELIMS,&ptr2);
            continue;
        }
        globFlag = 1; //Glob will be called, globfree() necessary
        if (glob(token,0,NULL,&globbuf)){
            /*No match found*/
            argv[index] = malloc((strlen(token)+1)*sizeof(char));
            strcpy(argv[index++],token);
        }
        else{
            /*Add each pathv to the argv array, one by one*/
            for (int i=0; i<globbuf.gl_pathc; i++){
                argv[index] = malloc((strlen(globbuf.gl_pathv[i])+1)*sizeof(char));
                strcpy(argv[index++],globbuf.gl_pathv[i]);
            }
        }
        token = strtok_r(NULL,WORD_DELIMS,&ptr2);
        globfree(&globbuf);
    }
    argv[nTokens] = NULL;

    return argv;
}


process* init_process(char *process_string){
    process *p = malloc(sizeof(*p));
    p->argv = NULL;
    p->argv = parseProcess(process_string);
    p->next = NULL;
    return p;
}

job* create_job(){
    job* j = malloc(sizeof(*j));
    j->next = NULL;
    j->first_process = NULL;
    j->isBackground = 0;
    return j;
}

/*Pushes given process to end of the list of given job's processes*/
void assign_process(job* job, process* proc){
    if (!job->first_process){
        job->first_process = proc;
        proc->next = NULL;
        return;
    }

    process *p = job->first_process;
    while (p->next)
        p = p->next;

    p->next = proc;
    proc->next = NULL;
}

void free_remaining_jobs(job* old){
    if (!old) return;
    job* new;
    while (old){
        new = old->next;
        free_job(old);
        old = new;
    }
}

void free_job(job *job){
    if (!job) return;
    process *p1 = job->first_process;
    while (p1){
        if (!p1->argv){
            process* p2 = p1->next;
            free(p1);
            p1 = p2;
            continue;
        }
        int i = 0;
        while (p1->argv[i]){
            free(p1->argv[i++]);
        }
        free(p1->argv);
        process *p2 = p1->next;
        free(p1);
        p1 = p2;
    }
    free(job);
}