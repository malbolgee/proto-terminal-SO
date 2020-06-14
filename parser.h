#pragma once

#define MAX_PROGARG_BUFFER_SIZE 4096
#define MAX_PROGNAME_SIZE MAX_PROGARG_BUFFER_SIZE
#define MAX_PROGARGS 10

typedef struct __prog{

    char progname[MAX_PROGNAME_SIZE];
    char prog_args[MAX_PROGARGS][MAX_PROGARG_BUFFER_SIZE];
    unsigned argc;

} prog_t;

#define REGEX_PATTERN_PROGNAME "^(\\./)?[a-zA-Z0-9._-]+$"
#define REGEX_PATTERN_ARGNAME "^-[a-zA-Z]$"
#define REGEX_PATTERN_MODIFIER "^[|<>&]$"
#define REGEX_PATTERN_PATHNAME "^(((\\.\\.|\\.)?/[/a-zA-Z0-9._-]+)|(\\.{1,2}(/)?)|([/a-zA-Z0-9._-]+))$"
#define REGEX_PATTERN_NPATHNAME "^(\\.\\.|\\.\\./)$"

#define BLUE "\033[1;34m"
#define DEFAULT "\033[0m"

bool match(const char *, const char *);
bool is_modifier(void);
void check_modifier(char);
prog_t *parse(int, char *const __argv[]);
char **tolkenizer(char *);
int cdcall(int, char **);
int pwdcall(int, char **);
int pipecall(prog_t *);
int outputcall(prog_t *);
int inputcall(prog_t *);