#include "alias.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delims.h"
#include "process.h"

/*Returns whether a string matches any saved alias keys*/
int isAlias(const char* string){
    alias* al = first;
    while (al){
        if (strcmp(al->key,string) == 0)
            return 1;
        al = al->next;
    }
    return 0;
}

/*Given a possible alias key, returns its value.
Returns NULL if given string is not a key of an existing alias*/
char* deref_alias(const char* key){
    alias* al = first;
    while (al){
        if (strcmp(al->key,key) == 0)
            return al->value;
        al = al->next;
    }
    return NULL;
}

/*Given a processString that begins with an alias key,
returns a new string, where the key is replaced by its value*/
char* replace_alias(const char* processString){
    char* tempString = malloc(strlen(processString)+1);
    strcpy(tempString,processString);

    char *ptr, *token, *rest;

    token = strtok_r(tempString,WORD_DELIMS,&ptr);
    rest = strtok_r(NULL,JOB_DELIMS,&ptr);

    const char* value = deref_alias(token);
    int len = strlen(value) + 1;
    if (rest) len += strlen(rest)+1;

    char *new_string = malloc(len*sizeof(char));
    strcpy(new_string,value);
    if (rest) {
        strcat(new_string," ");
        strcat(new_string,rest);
    }
    free(tempString);
    return new_string;
}

/*Takes in a processString and checks whether the first word is an alias key*/
int containsAlias(const char* processString){
    char* tempString = malloc(strlen(processString)+1);
    strcpy(tempString,processString);

    char *ptr;
    char *token = strtok_r(tempString,WORD_DELIMS,&ptr);

    int result = isAlias(token);
    free(tempString);
    return result;
}

/*Given an alias key, frees up the memory allocated by the alias and updates the alias list*/
void destroy_alias(const char* key){
    if (!key) return;
    alias *prev = first;
    if (!prev) return;
    if (strcmp(prev->key,key)==0){
        first = prev->next;
        free_alias(prev);
        return;
    }
    alias *current = prev->next;
    while (current){
        if (strcmp(current->key,key)==0){
            prev->next = current->next;
            free_alias(current);
            return;
        }
        prev = current;
        current = current->next;
    }
    fprintf(stderr,"%s","alias doesn't exist\n");
}

/*Creates a new alias or updates an existing one with the same key, assigning it the new value*/
void create_alias(const char* key, char* value){
    alias *al = first;
    if (!key || !value) return;
    if (!al){
        al = malloc(sizeof(*al));
        al->key = malloc((strlen(key)+1)*sizeof(char));
        strcpy(al->key,key);
        al->value = malloc((strlen(value)+1)*sizeof(char));
        strcpy(al->value,value);
        al->next = NULL;
        first = al;
        return;
    }


    while (al){
        /*Check if alias with the same key already exists and in that case, update it with the new value*/
        if (strcmp(al->key,key) == 0){
            free(al->value);
            al->value = malloc((strlen(value)+1)*sizeof(char));
            strcpy(al->value,value);
            printf("Updated alias: %s -> %s\n",al->key,al->value);   
            return;
        }
    }
    /*----------*/
    /*Alias key doesn't exist, create a new one and store it at the end of the list*/
    /*----------*/

    /*Find the last saved alias*/
    while (al){ 
        al = al->next;
    }

    /*Allocate space for the new one*/
    alias* new_alias = malloc(sizeof(*new_alias));

    /*Update list*/
    al = new_alias;

    /*Initialize new alias*/
    new_alias->key = malloc((strlen(key)+1)*sizeof(char));
    strcpy(new_alias->key,key);
    new_alias->value = malloc((strlen(value)+1)*sizeof(char));
    strcpy(new_alias->value,value);
    new_alias->next = NULL;
    printf("New alias: %s -> %s\n",al->key,al->value);   
}

/*Takes in an alias pointer, and frees all memory associated with it*/
void free_alias(alias* al){
    free(al->key);
    free(al->value);
    al->next = NULL;
    free(al);
}

/*Frees all aliases in the shell*/
void free_aliases(){
    alias *old = first;
    while(old){
        alias *new = old->next;
        free_alias(old);
        old = new;
    }
}