#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>


//Print a message to stdout, prefixed by the process ID
void procmsg(const char *format, ...){
    va_list ap;
    fprintf(stdout, "[%d] ", getpid());
    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
}

int is_prefix(const char *pre, const char *str) {
    return strncmp(pre, str, strlen(pre)) == 0;
}

// Function to remove the newline character from a string
void remove_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// split a string by a delimiter and store the tokens in an array
void split(char *str, char *delim, char **tokens, int *tokens_size){
    //copy str to not change original string
    char *str_copy = strdup(str);
    char *token = strtok(str_copy, delim);
    int i = 0;
    while(token){
        tokens[i] = token;
        i++;
        token = strtok(NULL, delim);
    }
    *tokens_size = i;
}
