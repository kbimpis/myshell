#include "delims.h"
#include "execute.h"
#include "alias.h"
#include "history.h"
#include <signal.h>

#define READ 0
#define WRITE 1

int isInput(char* token);

int isOutput(char* token);

int isAppend(char* token);

void redirect_input(const char* inFile);

void redirect_output(char* outFile);

void append_output(char* appendFile);

void setupInput(process* proc);

void setupOutput(process* proc);

void closePipes(const int proc_index, const int nPipes, int **fd);
