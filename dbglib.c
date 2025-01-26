#include "dbglib.h"



breakpoint *create_breakpoint(pid_t pid, void *addr){
    breakpoint *bp = (breakpoint *)malloc(sizeof(breakpoint));
    bp->addr = addr;
    enable_breakpoint(pid, bp);
    return bp;
}

static void enable_breakpoint(pid_t pid, breakpoint *bp){
    bp->orig_data = ptrace(PTRACE_PEEKTEXT, pid, bp->addr, 0);
    unsigned long data_with_trap = (bp->orig_data & 0xFFFFFFFFFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT,pid, bp->addr, (void*)data_with_trap);
}

static void disable_breakpoint(pid_t pid, breakpoint *bp){
    ptrace(PTRACE_POKETEXT, pid, bp->addr, (void*)bp->orig_data);
}

void free_breakpoint(breakpoint *bp){
    free(bp);
}

int resume_from_breakpoint(pid_t pid, breakpoint *bp) {
    struct user_regs_struct regs;
    int wait_status;

    ptrace(PTRACE_GETREGS, pid, 0, &regs);
    procmsg("Resuming from breakpoint, EIP = 0x%08x\n", regs.rip);

    disable_breakpoint(pid, bp);
    regs.rip -= 1;
    ptrace(PTRACE_SETREGS, pid, 0, &regs);

    if(ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL)) {
        perror("ptrace");
        return -1;
    }
    wait(&wait_status);
    if (WIFEXITED(wait_status)) {
        return 0;
    }

    enable_breakpoint(pid, bp);
    return 1;
}

//initialize list_of_breakpoints
list_of_breakpoints *init_breakpoints_list() {
    list_of_breakpoints *list = (list_of_breakpoints *)malloc(sizeof(list_of_breakpoints));
    list->size = 10;
    list->curr_index = 0;
    list->list = (breakpoint **)malloc(list->size * sizeof(breakpoint *));
    return list;
}

//add breakpoint pointer to a list_of_breakpoints, resizing the list if necessary
void add_breakpoint_to_list(list_of_breakpoints *list, breakpoint *bp) {
    if (list->curr_index == list->size) {
        list->size *= 2;
        list->list = (breakpoint **)realloc(list->list, list->size * sizeof(breakpoint *));
    }
    list->list[list->curr_index] = bp;
    list->curr_index++;
}

//free list and all breakpoints
void free_breakpoints_list(list_of_breakpoints *list) {
    for (int i = 0; i < list->curr_index; i++) {
        free_breakpoint(list->list[i]);
    }
    free(list->list);
    free(list);
}

//find breakpoint index in list by address
int find_breakpoint_index(list_of_breakpoints *list, void *addr) {
    for (int i = 0; i < list->curr_index; i++) {
        if (list->list[i]->addr == addr) {
            return i;
        }
    }
    return -1;
}
