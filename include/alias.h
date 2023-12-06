#pragma once

typedef struct alias{
    char* key;
    char* value;
    struct alias* next;
}alias;

void create_alias(const char* key, char* value);
void destroy_alias(const char* key);
void free_alias(alias* al);
void free_aliases();
int containsAlias(const char* processString);
char* replace_alias(const char* processString);
alias* first;
 