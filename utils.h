#ifndef DBGEKO_UTILS_H
#define DBGEKO_UTILS_H
#include <stdlib.h>
#include <stdint.h>


void procmsg(const char *format, ...);
int is_prefix(const char *pre, const char *str);
void remove_newline(char *str);
void split(char *str, char *delim, char **tokens, int *tokens_size);
char *disassemble_x32_instruction(unsigned long instruction, unsigned long long int i);

enum action {WAIT, ERROR, QUIT,NO_ACTION};
typedef enum action action;

#endif //DBGEKO_UTILS_H