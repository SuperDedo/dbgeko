#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <capstone/capstone.h>


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



// Function to disassemble an instruction from memory (up to 15 bytes)
char *disassemble_x32_instruction(unsigned long instruction, unsigned long long int addr) {
    csh handle;
    cs_insn *insn;
    size_t count;
    char *result = NULL;

    // Convert the 64-bit instruction (unsigned long) to a byte array (little-endian)
    unsigned char code[8];  // Size = 8 bytes as per your instruction size
    for (int i = 0; i < 8; i++) {
        code[i] = (instruction >> (i * 8)) & 0xFF;
    }

    // Debugging step: Print the byte array to verify correctness
//    printf("Instruction bytes: ");
//    for (int i = 0; i < 8; i++) {
//        printf("%02x ", code[i]);
//    }
//    printf("\n");

    // Initialize Capstone disassembler for x86 in 32-bit mode (CS_MODE_32)
    if (cs_open(CS_ARCH_X86, CS_MODE_32, &handle) != CS_ERR_OK) {
        return strdup("Failed to initialize Capstone");
    }

    // Disassemble the instruction, passing the 8 bytes
    count = cs_disasm(handle, code, sizeof(code), addr, 1, &insn);
    if (count > 0) {
        // Allocate memory for the result string
        size_t length = snprintf(NULL, 0, "0x%" PRIx64 ": %s %s", insn[0].address, insn[0].mnemonic, insn[0].op_str);
        result = malloc(length + 1); // Allocate memory for the string
        snprintf(result, length + 1, "0x%" PRIx64 ": %s %s", insn[0].address, insn[0].mnemonic, insn[0].op_str);

        // Free memory allocated by Capstone
        cs_free(insn, count);
    } else {
        result = strdup("ERROR: Failed to disassemble the given instruction!");
    }

    // Close the Capstone handle
    cs_close(&handle);
    return result;

}

