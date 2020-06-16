#pragma once

#define MAX_PROGARGS 10
#define MAX_PROGARG_BUFFER_SIZE 4096
#define MAX_PROGNAME_SIZE MAX_PROGARG_BUFFER_SIZE
#define MAX_USERNAME MAX_PROGARG_BUFFER_SIZE
#define MAX_HOSTNAME MAX_USERNAME

typedef struct __prog{

    unsigned argc;
    char prog_args[MAX_PROGARGS][MAX_PROGARG_BUFFER_SIZE];
    unsigned char mod;

} prog_t;

#define PIPE_READ_END 0x0
#define PIPE_WRITE_END 0x1

#define IS_NMOD_PIPE 0x1
#define IS_NMOD_OUTPUT 0x2
#define IS_NMOD_INPUT 0x4
#define IS_MOD_AMPER 0x8

#define IS_PMOD_PIPE (IS_NMOD_PIPE << 0x4)
#define IS_PMOD_OUTPUT (IS_NMOD_OUTPUT << 0x4)
#define IS_PMOD_INPUT (IS_NMOD_INPUT << 0x4)

#define REGEX_PATTERN_PROGNAME "^(\\./)?[a-zA-Z0-9._-]+$"
#define REGEX_PATTERN_ARGNAME "^-[a-zA-Z]$"
#define REGEX_PATTERN_MODIFIER "^[|<>&]$"
#define REGEX_PATTERN_PATHNAME "^(((\\.\\.|\\.)?/[/a-zA-Z0-9._-]+)|(\\.{1,2}(/)?)|([/a-zA-Z0-9._-]+))$"
#define REGEX_PATTERN_NPATHNAME "^(\\.\\.|\\.\\./)$"

#define GREEN "\033[1;32m"
#define BLUE "\033[1;34m"
#define DEFAULT "\033[0m"

void show_path_str(void);
void check_modifier(char);
char **tolkenizer(char *);
int cdcall(int, char **);
int pwdcall(int, char **);
bool match(const char *, const char *);
prog_t *parse(int, char *const __argv[]);

void close_stdout_pipe(int fd[2]);
void close_stdin_pipe(int fd[2]);
void close_stdin_input(char *);
void close_stdout_output(char *);
void close_fd(int (*fd)[2], size_t);
void fill_args(char (*args)[MAX_PROGARG_BUFFER_SIZE], char *myargs[], int);