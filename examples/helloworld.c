//print hello world
#include <stdio.h>
#include "../build/_deps/linenoise-src/linenoise.h"

int main(){
    printf("Hello, World!\n");
    char *line;
    while((line = linenoise("hello> ")) != NULL) {
        printf("You wrote: %s\n", line);
        linenoiseHistoryAdd(line);
        linenoiseFree(line); /* Or just free(line) if you use libc malloc. */
    }
    return 0;
}