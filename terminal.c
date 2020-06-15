#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdio.h>
#include <regex.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "parser.h"

bool is_pipe;
bool is_amper;
bool is_input;
bool is_pinput;
bool is_output;
bool is_poutput;
bool wait_for_prog;
bool wait_for_arg;

unsigned token_c;
unsigned program_counter;
unsigned char mod_flag;

int main(int argc, char **argv)
{

    prog_t *program;
    char string[PATH_MAX];
    
    show_path_str();
    while (scanf("%[^\n]%*c", string), strcmp(string, "exit") != 0)
    {

        if (string[0] != 0)
        {

            char **str_tokens = tolkenizer(string);
            program = parse(token_c, str_tokens);

            if (program == NULL)
            {

                show_path_str();
                continue;

            }

        }

        if (strcmp(program[0].prog_args[0], "cd") == 0)
        {
            char *myargs[10] = {NULL};
            fill_args(program[0].prog_args, myargs, program[0].argc);
            cdcall(2, myargs);

        }
        else if (strcmp(program[0].prog_args[0], "pwd") == 0)
        {

            char *myargs[10] = {NULL};
            fill_args(program[0].prog_args, myargs, program[0].argc);
            pwdcall(1, myargs);

        }
        else
        {

            int fd[program_counter][2];
            if (is_pipe)
                for (size_t i = 0; i < program_counter - 1; ++i)
                    pipe(fd[i]);

            for (size_t i = 0; i < program_counter; ++i)
            {

                char *myargs[10] = {NULL};
                int id = fork();
                if (id == -1)
                {

                    perror("Erro");
                    return errno;

                }
                else if (id == 0)
                {
                    
                    /* O modificador do programa anterior é um pipe, 
                    fecha o stdin do programa atual e troca pelo fd[0]
                    do programa anterior. */
                    if (program[i].mod & IS_PMOD_PIPE)
                        close_stdin_pipe(fd[i - 1]);
                    
                    /* O modificador do programa atual é um pipe,
                    fecha o stdout do programa atual e troca pelo fd[1]
                    do programa atual. */
                    if (program[i].mod & IS_NMOD_PIPE)
                        close_stdout_pipe(fd[i]);

                    /* O modificador do programa atual é um '<',
                    fecha o stdin do programa atual e passa como argumento
                    o nome do arquivo. */
                    int idx = program[i].argc - 1;
                    if (program[i].mod & IS_NMOD_INPUT)
                        close_stdin_input(program[i].prog_args[idx]);

                    /* O modificador do programa atual é um '>',
                    fecha o stdout do programa atual e passa como argumento
                    o nome do arquivo. */
                    if (program[i].mod & IS_NMOD_OUTPUT)
                        close_stdout_output(program[i].prog_args[idx]);

                    /* Caso o redirecionamento seja de/para arquivo, o argumento com o nome
                    do arquivo não deve ser passado. */
                    if (program[i].mod & IS_NMOD_OUTPUT || program[i].mod & IS_NMOD_INPUT)
                        fill_args(program[i].prog_args, myargs, program[i].argc - 1);
                    else
                        fill_args(program[i].prog_args, myargs, program[i].argc);

                    close_fd(fd, program_counter);
                    execvp(myargs[0], myargs);
                    perror("Erro");
                    return errno;

                }

            }

            close_fd(fd, program_counter);
            if (!is_amper)
                while ((wait(NULL) != -1) || (errno != ECHILD));

        }

        show_path_str();

    }

    return 0;
}

prog_t *parse(int __argc, char *const __argv[])
{

    /* vou alocar 10 posições aqui, o que signigica que só pode ter 10 programas em chain. */
    prog_t *programs = (prog_t *)calloc(10, sizeof(prog_t));

    int k, l;
    k = l = 0;
    program_counter = 0;

    wait_for_prog = true;
    wait_for_arg  = false;
    is_amper = is_pipe = false;
    is_input = is_pinput = false;
    is_output = is_poutput = false;

    for (size_t i = 0; i <= __argc; ++i)
    {

        if (match(__argv[i], REGEX_PATTERN_PROGNAME) && wait_for_prog && !wait_for_arg)
        {
            l = 1;
            ++program_counter;
            ++programs[k].argc;
            wait_for_prog = false;
            wait_for_arg = true;
            strcpy(programs[k].prog_args[0], __argv[i]);
        }
        else if ((match(__argv[i], REGEX_PATTERN_ARGNAME) || match(__argv[i], REGEX_PATTERN_PATHNAME)) && !wait_for_prog && wait_for_arg)
            strcpy(programs[k].prog_args[l++], __argv[i]), ++programs[k].argc;
        else if (match(__argv[i], REGEX_PATTERN_MODIFIER) && !is_amper)
        {

            check_modifier(__argv[i][0]);

            if (is_pipe && (mod_flag & IS_NMOD_INPUT))
            {
                printf("Erro de sintaxe próximo ao token `%s': Não pode haver um '|' seguido de um '<'.\n", __argv[i]);
                return NULL;
            }

            if (is_pinput && is_input)
            {
                printf("Erro de sintaxe próximo ao token `%s': Não pode haver dois '<' seguidos.\n", __argv[i]);
                return NULL;
            }
            else
                is_pinput = is_input, is_input = false;

            if (is_poutput && is_output)
            {
                printf("Erro de sintaxe próximo ao token `%s': Não pode haver dois '>' seguidos.\n", __argv[i]);
                return NULL;
            }
            else
                is_poutput = is_output, is_output = false;

            if (mod_flag & IS_NMOD_PIPE)
            {
                wait_for_arg = false;
                programs[k++].mod |= mod_flag;
                programs[k].mod   |= (mod_flag << 4); // Setando a flag do anterior no próximo;

            }
            else
                programs[k].mod |= mod_flag;
            
        }
        else
        {

            printf("Erro de sintaxe próximo ao token `%s'\n", __argv[i]);
            return NULL;
        }

    }
   
    if (wait_for_prog)
    {

        printf("Erro: o uso de modificadores '|', '>' e '<' requerem um segundo argumento.\n");
        return NULL;
    }
   
    return programs;
}

/* Tokeniza INPUT. */
char **tolkenizer(char *input)
{

    token_c = 0;
    for (size_t i = 0; input[i]; ++i)
        if (input[i] == ' ')
            ++token_c;

    char **str_tokens = (char **)calloc((token_c + 1), sizeof(char *));

    int idx = 1;
    char *str_token = strtok(input, " ");
    str_tokens[0] = str_token;

    while ((str_token = strtok(NULL, " ")))
        str_tokens[idx++] = str_token;

    return str_tokens;
}

/* Verifica qual modificador é CH. */
void check_modifier(char ch)
{

    mod_flag = 0;
    switch (ch)
    {

        case '|':
            is_pipe = true;
            mod_flag |= IS_NMOD_PIPE;
            wait_for_prog = true;
            wait_for_arg = false;
            break;
        case '<':
            is_input = true;
            mod_flag |= IS_NMOD_INPUT;
            wait_for_prog = false;
            wait_for_arg = true;
            break;
        case '>':
            is_output = true;
            mod_flag |= IS_NMOD_OUTPUT;
            wait_for_prog = false;
            wait_for_arg = true;
            break;
        case '&':
            mod_flag |= IS_MOD_AMPER;
            wait_for_arg = false;
            is_amper = true;
            break;
        default:
            puts("deu ruim");
    }
}

/* Usa expressão regular para testar STRING contra PATTERN. */
bool match(const char *string, const char *pattern)
{

    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB) != 0)
        return false;

    int status = regexec(&re, string, 0, NULL, 0);

    regfree(&re);
    return status != REG_NOMATCH ? true : false;
}

/* Muda o diretório atual para o diretório passado como argumento. */
int cdcall(int argc, char **argv)
{

    if (argv[1] != NULL)
    {
        
        char buff[PATH_MAX] = { 0 };
        char aux[PATH_MAX << 1] = { 0 };

        if (match(argv[1], REGEX_PATTERN_NPATHNAME))
            strcat(aux, getcwd(buff, PATH_MAX)), dirname(aux);
        else
            strcat(aux, argv[1]);

        if (chdir(aux) == -1)
            perror("Erro ao abrir o diretório");

    }

    return 0;
}

/* Exibe o diretório atual. */
int pwdcall(int argc, char **argv)
{

    char buffer[PATH_MAX];
    if (argc == 1 || (strcmp(argv[1], "-L") == 0))
        printf("%s\n", getenv("PWD"));
    else if (strcmp(argv[1], "-P") == 0)
        printf("%s\n", getcwd(buffer, PATH_MAX));
    else
        perror("Error");

    return 0;
}

/* Duplica FD[1] e fecha STDIN. */
void close_stdout_pipe(int fd[2])
{

    dup2(fd[PIPE_WRITE_END], STDOUT_FILENO);

}

/* Duplica FD[0] e fecha STDOUT. */
void close_stdin_pipe(int fd[2])
{

    dup2(fd[PIPE_READ_END], STDIN_FILENO);

}

/* Duplica o FD gerado para ARG e fecha STDIN. */
void close_stdin_input(char *arg)
{

    int file = open(arg, O_RDONLY);
    dup2(file, STDIN_FILENO);
    close(file);

}

/* Duplica o FD gerado para ARG e fecha o STDOUT. */
void close_stdout_output(char *arg)
{

    int file = open(arg, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    dup2(file, STDOUT_FILENO);
    close(file);

}

/* Preenche MYARGS com ARGC argumentos de ARGS. */
void fill_args(char (*args)[MAX_PROGARG_BUFFER_SIZE], char *__myargs[], int __argc)
{

    for (size_t i = 0; i < __argc; ++i)
        __myargs[i] = args[i];

}

/* Fecha SIZE - 1 files descriptors em FD. */
void close_fd(int (*fd)[2], size_t size)
{   

    for (size_t i = 0; i < size - 1; ++i)
    {
        close(fd[i][PIPE_READ_END]);
        close(fd[i][PIPE_WRITE_END]);
    }

}

void show_path_str(void)
{

    char buff[PATH_MAX];
    char *path = getenv("HOME");

    getcwd(buff, PATH_MAX);
    if (strcmp(path, buff) == 0)
        puts("$: ");
    else
    {

        char bufff[PATH_MAX] = "~";
        strcat(bufff , strrchr(buff, '/'));
        printf("%s%s%s$: ", BLUE, bufff, DEFAULT);

    }

}