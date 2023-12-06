#include "history.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "delims.h"
 

int is_int(const char* string){
    if (!string) return 0;
    for (int i=0; i<(strlen(string)-1); i++){
        if (!isdigit(string[i]))
            return 0;
    }
    return 1; 
}

int convert_to_int(const char* argument){
    if (!is_int(argument)) {
        fprintf(stderr,"%s","myHistory argument must be numeric");
        return 0;
    }
    int number = atoi(argument);
    if ((number < 1) || (number > h->current)){
        fprintf(stderr,"%s","Argument out of range");
        return 0;
    }
    return number;
}

int isHistoryArgument(const char* token){
    for (int i=0; i<(strlen(token)-1); i++){
        if (!isdigit(token[i]))
            return 0;
    }
    int number = atoi(token);
    if ((number < 1) || (number > h->current))
        return 0;
    return number;
}

void instantiate_history(){
    h = malloc(sizeof(*h));
    h->commands = (char**)malloc(HIST_LENGTH*sizeof(char*));
    for (int i=0; i<HIST_LENGTH; i++){
        h->commands[i] = NULL;
    }
    h->current = 0;
}

void addToHistory(char* command){
    char* ptr;
    char* tempString = (char*)malloc((strlen(command)+1)*sizeof(char));
    strcpy(tempString,command);
    char* cmd = strtok_r(tempString,"\n",&ptr);

    /*If the history instance is not full*/
    if (h->current < HIST_LENGTH){
        h->commands[h->current] = (char*)malloc((strlen(cmd))+1);
        strcpy(h->commands[h->current++],cmd);
        free(tempString);
        return;
    }

    /*Delete first record*/
    free(h->commands[0]);

    /*Move all records one slot back*/
    for (int i=0; i<(HIST_LENGTH-1); i++){
        h->commands[i] = h->commands[i+1];
    }

    /*Save the last command at the end*/
    h->commands[HIST_LENGTH-1] = (char*)malloc((strlen(cmd))+1);
    strcpy(h->commands[HIST_LENGTH-1],cmd);

    free(tempString);
}

void showHistory(){
    for (int i=0; i<h->current; i++){
        printf("[%d] %s\n",i+1,h->commands[i]);
    }
}

char* getHistoryCommand(int index){
    index--;
    char* command = (char*)malloc((strlen(h->commands[index])+1)*sizeof(char));
    strcpy(command,h->commands[index]);
    return command;
}

void free_history(){
    for (int i=0; i<h->current; i++){
        free(h->commands[i]);
    }
    free(h->commands);
    free(h);
}
