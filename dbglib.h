#include <stdbool.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#include <stdio.h>
#include "utils.h"

#ifndef DBGEKO_DBGLIB_H
#define DBGEKO_DBGLIB_H



typedef struct breakpoint{
    void *addr;
    unsigned long orig_data;
}breakpoint;

typedef struct list_of_breakpoints {
    breakpoint **list;
    size_t size;
    int curr_index;
} list_of_breakpoints;

/* Initializes a list of breakpoints */
list_of_breakpoints *init_breakpoints_list();

/* Adds a breakpoint pointer to a list of breakpoints */
void add_breakpoint_to_list(list_of_breakpoints *list, breakpoint *bp);

/* Frees a list of breakpoints */
void free_breakpoints_list(list_of_breakpoints *list);

/* Creates and enables a breakpoint at addr */
breakpoint *create_breakpoint(pid_t pid, void *addr);

/* Enables a breakpoint */
static void enable_breakpoint(pid_t pid, breakpoint *bp);

/* Disables a breakpoint */
static void disable_breakpoint(pid_t pid, breakpoint *bp);

/* Frees a breakpoint */
void free_breakpoint(breakpoint *bp);

/* Resumes execution from a breakpoint */
int resume_from_breakpoint(pid_t pid, breakpoint *bp);

/*find breakpoint index in list by address */
int find_breakpoint_index(list_of_breakpoints *list, void *addr);

/*print all breakpoints in list */
void print_breakpoints(list_of_breakpoints *list);



#endif //DBGEKO_DBGLIB_H