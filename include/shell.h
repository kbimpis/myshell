#include "process.h"

#pragma once



void init_shell();
void type_prompt();
void destroy_shell();


job* assign_jobs();
int isQuit(job*);