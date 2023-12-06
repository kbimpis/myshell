#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "delims.h"
#include "alias.h"
#include "history.h"

void parse_CreateAlias(job** j, char* jobString){
    process* p = (process*)malloc(sizeof(*p));
    p->next = NULL;
    p->argv = (char**)malloc(3*sizeof(char*));

    assign_process(*j,p);
    

    char* ptr;
    char* word1 = strtok_r(jobString,WORD_DELIMS,&ptr); //This will always be == "createalias"

    char* word2 = strtok_r(NULL,WORD_DELIMS,&ptr);
    if (!word2){
        p->argv[0] = NULL;
        p->argv[1] = NULL;
        return;
    }
    p->argv[0] = (char*)malloc((strlen(word2)+1)*sizeof(char));
    strcpy(p->argv[0],word2);

    char* word3 = strtok_r(NULL,"\"",&ptr);
    if (!word3){
        p->argv[1] = NULL;
        return;
    }
    p->argv[1] = (char*)malloc((strlen(word3)+1)*sizeof(char));
    strcpy(p->argv[1],word3);

    p->argv[2] = NULL;
}

void parse_DestroyAlias(job** j, char* jobString){
    process* p = malloc(sizeof(*p));
    p->next = NULL;
    p->argv = (char**)malloc(2*sizeof(char*));

    assign_process(*j,p);

    char* ptr;
    char* firstWord = strtok_r(jobString,WORD_DELIMS,&ptr); //This will always be == "destroyalias"

    char* secondWord = strtok_r(NULL,WORD_DELIMS,&ptr);
    if (!secondWord){
        p->argv[0] = NULL;
        return;
    }
    p->argv[0] = (char*)malloc((strlen(secondWord)+1)*sizeof(char));
    strcpy(p->argv[0],secondWord);

    p->argv[1] = NULL;
}

void parse_CD(job** j, char* jobString){
    char* ptr;
    char* firstWord = strtok_r(jobString,WORD_DELIMS,&ptr); //This will always be == cd

    char* secondWord = strtok_r(NULL,WORD_DELIMS,&ptr);
    if (!secondWord){ //If the input is simply "cd", change to home directory
        secondWord = getenv("HOME");
    }

    process* p = malloc(sizeof(*p));
    p->next = NULL;
    p->argv = (char**)malloc(2*sizeof(char*));

    p->argv[0] = (char*)malloc((strlen(secondWord)+1)*sizeof(char));
    strcpy(p->argv[0],secondWord);

    p->argv[1] = NULL;

    (*j)->first_process = p;
}

char* parse_Hist2(job** j, char* jobString){
    char* ptr;
    char* firstWord = strtok_r(jobString,WORD_DELIMS,&ptr); //This will always be == HIST_KEYWORD

    char* secondWord = strtok_r(NULL,WORD_DELIMS,&ptr);
    int argument = isHistoryArgument(secondWord);
    if (!isHistoryArgument(secondWord)){ //Usage: myHistory <x> where x is an int between 1 and the number of current records in history
        fprintf(stderr,"%s","Invalid myHistory argument\n");
        return NULL;
    }

    (*j)->type = NORMAL;

    return getHistoryCommand(argument);

}


int isFirstWord(const char* word, const char* string){
    char* tempString = (char*)malloc((strlen(string)+1)*sizeof(char));
    strcpy(tempString,string);

    char* ptr;
    char* token = strtok_r(tempString,WORD_DELIMS,&ptr);

    int cmp = strcmp(token,word);
    free(tempString);
    if (cmp == 0) return 1;
    return 0;
}

/*Returns whether a string is "myHistory" or "myHistory <x>"*/
int myHistory_version(const char* string){
    char* tempString = (char*)malloc((strlen(string)+1)*sizeof(char));
    strcpy(tempString,string);

    /*version=0 means it is not myHistory*/
    int version = 0;

    if (isFirstWord(HIST_KEYWORD,tempString)){
        version = 1; //Means it has no argument, it is just the "display" version

        char* ptr;
        char* first_word = strtok_r(tempString,WORD_DELIMS,&ptr); //This will always be HIST_KEYWORD
        char* second_word = strtok_r(NULL,WORD_DELIMS,&ptr);

        if (second_word)
            version = 2;
    }

    free(tempString);
    return version;
}

void determine_type(job** j, const char* jobString){
    int hist_version = myHistory_version(jobString);
    if (hist_version == 1)
        (*j)->type = SHOW_HISTORY;
    else if(hist_version == 2)
        (*j)->type = GET_HISTORY;
    else if(isFirstWord("cd",jobString))
        (*j)->type = CD;
    else if(isFirstWord("createalias",jobString))
        (*j)->type = CREATE_ALIAS;
    else if(isFirstWord("destroyalias",jobString))
        (*j)->type = DESTROY_ALIAS;
    else
        (*j)->type = NORMAL;
}

/*Finds the first jobString it finds in the commandLine string, and assigns it to the job.*/
char* parseJob(char* commandLine, job **job){
    char *jobString, *processString, *cmdLinePtr, *jobStringPtr;
    if (!(jobString = strtok_r(commandLine,JOB_DELIMS,&cmdLinePtr))){
        fprintf(stderr,"%s","Error during input\n");
        return NULL;
    }

    determine_type(job,jobString);

    char* hist2_string;
    switch ((*job)->type)
    {
        case CD:
            parse_CD(job,jobString);
            break;
        case CREATE_ALIAS:
            parse_CreateAlias(job,jobString);
            break;
        case DESTROY_ALIAS:
            parse_DestroyAlias(job,jobString);
            break;
        case GET_HISTORY:
            /*Parse it from the beginning as if it were a normal job. job->type is also assigned to NORMAL*/
            hist2_string = parse_Hist2(job,jobString);

            /*Let the user know which command they just requested*/
            printf("-----Running: %s\n",hist2_string);
            parseJob(hist2_string,job);
            free(hist2_string);
            /*job doesn't have any sub-processes, so return*/
            return strtok_r(NULL,JOB_DELIMS,&cmdLinePtr);
    }

    if ((*job)->type != NORMAL){
        return strtok_r(NULL,JOB_DELIMS,&cmdLinePtr);
    }

    (*job)->isBackground = (jobString[strlen(jobString)-1] == '&');

    processString = strtok_r(jobString,PROC_DELIMS,&jobStringPtr);

    process* oldProc;


    if (containsAlias(processString)){
        char *replacement = replace_alias(processString);
        oldProc = init_process(replacement);
        free(replacement);
    }
    else
        oldProc = init_process(processString);

    if (!oldProc)
        return strtok_r(NULL,JOB_DELIMS,&cmdLinePtr);
    
    assign_process((*job),oldProc);
    process *newProc;
    while (processString = strtok_r(NULL,PROC_DELIMS,&jobStringPtr)){
        newProc = init_process(processString);
        if (!newProc){
            continue;
        }
        assign_process((*job),newProc);
        oldProc->next = newProc;
        oldProc = newProc;
    }
    return strtok_r(NULL,JOB_DELIMS,&cmdLinePtr);
}

/*Checks whether input is valid(namely not simply a semicolon(;) or a newline character(\n) )*/
int validInput(char* input){
    if (!input) return 0;

    char* tempString = (char*)malloc((strlen(input)+1)*sizeof(char));
    strcpy(tempString,input);
    char* ptr;
    char* token = strtok_r(tempString,"\n;",&ptr);

    if (!token){
        free(tempString);
        return 0;
    }
    
    free(tempString);
    return 1;
}

/*Reads input from the command line, and assigns it to the appropriate number of jobs.
Returns the list of jobs it created*/
job* assign_jobs(){
    char* commandLine = malloc(INPUT_SIZE*sizeof(char));

    fgets(commandLine,INPUT_SIZE,stdin);
    while (!validInput(commandLine)){
        printf(PROMPT);
        fgets(commandLine,INPUT_SIZE,stdin);
    }

    job* firstJob = create_job();
    char* jobString, *ptr1, *ptr2;

    addToHistory(commandLine);

    jobString = parseJob(commandLine,&firstJob);

    job* job = firstJob;
    while (jobString){
        job->next = create_job();
        jobString = parseJob(jobString,&job->next);
    }
    free(commandLine);

    return firstJob;
}