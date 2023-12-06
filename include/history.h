// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>

#pragma once

#define HIST_LENGTH 20
#define HIST_KEYWORD "myHistory"

typedef struct history{
    char** commands;
    int current;                    //stores the number of commands saved in the struct
}* history;

history h;

void addToHistory(char* command);

void showHistory();
char* getHistoryCommand(int index);
void free_history();
void instantiate_history();
int isHistory(const char* command);
int isHistoryArgument(const char* token);
