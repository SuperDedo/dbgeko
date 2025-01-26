#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <string.h>
#include "utils.h"
#include "dbglib.h"
#include "build/_deps/linenoise-src/linenoise.h"


list_of_breakpoints *breakpoints_list;

void run_debugger(pid_t pid);

void run_target(char *program_name);

action handle_command(pid_t pid, char *line, int *breakpoint_index);

int handle_return_from_wait(pid_t pid, int status);

int main(int argc, char **argv){
    if(argc < 2){
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    breakpoints_list = init_breakpoints_list();

    //fork a child process
    pid_t child_pid = fork();
    if(child_pid == 0){
        //child process
        procmsg("Child process\n");
        //execute the program
        run_target(argv[1]);

    }else if(child_pid > 0){
        //parent process
        procmsg("Parent process\n");
        run_debugger(child_pid);
        free_breakpoints_list(breakpoints_list);
    }else{
        //fork failed
        perror("fork");
        return 1;
    }
    return 0;

}

void run_target(char *program_name){
    procmsg("Running target: %s\n", program_name);
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    execl(program_name, program_name, NULL);
    perror("execl");

}

void run_debugger(pid_t pid){
    int wait_status;

    procmsg("Debugger started\n");
    wait(&wait_status);
    procmsg("Child started. pid: %d.\n",pid);

    char* line = NULL;
    action action;
    int breakpoint_index = -1;
    while((line = linenoise("<dbgeko> ")) != NULL) {
        action = handle_command(pid, line, &breakpoint_index);
        if (action == QUIT) {
            break;
        }else if (action == ERROR) {
            return;
        } else if(action == WAIT) {
            wait(&wait_status);
            breakpoint_index = handle_return_from_wait(pid, wait_status);
            if (breakpoint_index == -2) {
                break;
            }
        }
        linenoiseHistoryAdd(line);
        linenoiseFree(line);
    }
    procmsg("Exiting...\n");
}

int handle_return_from_wait(pid_t pid, int status) {
    if (WIFEXITED(status)) {
        procmsg("Child exited\n");
        return -2;
    }else if (WIFSTOPPED(status)) {
        int signal = WSTOPSIG(status);
        procmsg("Child stopped. Signal: %s\n", strsignal(signal));
        if (signal == SIGTRAP) {
            struct user_regs_struct regs;
            ptrace(PTRACE_GETREGS, pid, NULL, &regs);
            unsigned instruction = ptrace(PTRACE_PEEKTEXT, pid, regs.rip -1, NULL);
            //check if instruction is a breakpoint
            if((instruction&0x000000FF) == 0xCC){
                int i = find_breakpoint_index(breakpoints_list, (void *)regs.rip - 1);
                procmsg("Hit breakpoint %d at address: %08x. Instruction: %08x\n",i, regs.rip, breakpoints_list->list[i]->orig_data);
                return i;
            }

            procmsg("EIP: %08x, Instruction: 0x%08x\n", regs.rip, instruction);
            return -1;
        }
    } else {
        procmsg("Unexpected signal\n");
        procmsg("Child got a signal: %s\n", strsignal(WSTOPSIG(status)));
    }
}

action handle_command(pid_t pid, char *line, int *breakpoint_index) {
    int wait_status;
    char *tokens[5];
    int tokens_size = 0;
    split(line, " ", tokens, &tokens_size);
    if (tokens_size == 0) {
        return NO_ACTION;
    }
    char *command = tokens[0];

    if(is_prefix(command,"quit")){
        //exit
        return QUIT;
    }else if(is_prefix(command,"step")){
        if(*breakpoint_index != -1) {
            //resume from breakpoint
            int i = resume_from_breakpoint(pid, breakpoints_list->list[*breakpoint_index]);
            *breakpoint_index = -1;
            if (i == 0) {
                return QUIT;
            } else if (i == 1) {
                return NO_ACTION;
            } else if (i == -1){
                return ERROR;
            }
        }
        //single step the process
        if(ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL) <0){
            perror("ptrace");
            return ERROR;
        } return WAIT;
    }else if(is_prefix(command, "continue")) {
        if(*breakpoint_index != -1) {
            //resume from breakpoint
            resume_from_breakpoint(pid, breakpoints_list->list[*breakpoint_index]);
        }if (ptrace(PTRACE_CONT, pid, NULL, NULL) < 0) {
            perror("ptrace");
            return ERROR;
        } return WAIT;
    } else if(is_prefix(command, "breakpoint")){
        if (tokens_size < 2) {
            printf("Usage: breakpoint <address>\n");
            return NO_ACTION;
        }
        void *addr = (void *)strtol(tokens[1], NULL, 16);
        breakpoint *bp = create_breakpoint(pid, addr);
        add_breakpoint_to_list(breakpoints_list, bp);
    }
    else{
        printf("Invalid command: %s", command);
        return NO_ACTION;

    }
    return NO_ACTION;
}



