#ifndef DBGEKO_UTILS_H
#define DBGEKO_UTILS_H



void procmsg(const char *format, ...);
int is_prefix(const char *pre, const char *str);
void remove_newline(char *str);
void split(char *str, char *delim, char **tokens, int *tokens_size);

enum action {WAIT, ERROR, QUIT,NO_ACTION};
typedef enum action action;

#endif //DBGEKO_UTILS_H