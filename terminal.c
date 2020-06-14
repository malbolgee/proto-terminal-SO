#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>
#include "parser.h"

bool is_pipe;
bool is_input;
bool is_output;
bool is_amper;

unsigned token_c;
unsigned program_counter;
bool wait_for_arg;
bool wait_for_prog;

int main(int argc, char **argv)
{

    char buff[PATH_MAX];
    char string[100];
    prog_t *program;
    printf("%s%s%s$ ", BLUE, getcwd(buff, PATH_MAX), DEFAULT);
    while (scanf("%[^\n]%*c", string), strcmp(string, "exit") != 0)
    {

        if (string[0] != 0)
        {

            char **str_tokens = tolkenizer(string);
            program = parse(token_c, str_tokens);
        }

        if (is_pipe)
            pipecall(program);
        else if (is_input)
            inputcall(program);
        else if (is_output)
            outputcall(program);
        else
        {

            if (strcmp(program[0].progname, "cd") == 0)
            {

                char *myargs[10] = {NULL};
                for (size_t i = 0; i < program[0].argc; ++i)
                    myargs[i] = program[0].prog_args[i];

                cdcall(2, myargs);
            }
            else if (strcmp(program[0].progname, "pwd") == 0)
            {

                char *myargs[3] = {"pwd", 0, 0};
                pwdcall(1, myargs);
            }
            else if (strcmp(program[0].progname, "./lsd") == 0)
            {

                char *myargs[10] = {NULL};
                for (size_t i = 0; i < program[0].argc; ++i)
                    myargs[i] = program[0].prog_args[i];

                int id = fork();
                if (id == -1)
                {

                    perror("Erro");
                    return errno;
                }
                else if (id == 0)
                    execvp(myargs[0], myargs);

                wait(0);
            }
        }

        printf("%s%s%s$ ", BLUE, getcwd(buff, PATH_MAX), DEFAULT);
    }

    return 0;
}

prog_t *parse(int __argc, char *const __argv[])
{

    // Vou alocar 2 posições desse vetor e, se precisar, depois faço uma versão mais genérica;
    prog_t *programs = (prog_t *)calloc(2, sizeof(prog_t));

    int i = 0;
    int k, l, j;
    k = l = j = 0;
    wait_for_arg = false;
    wait_for_prog = true;
    program_counter = 0;

    is_pipe = is_input = is_output = is_amper = false;
    while (i <= __argc)
    {

        if (match(__argv[i], REGEX_PATTERN_PROGNAME) && wait_for_prog && !wait_for_arg)
        {
            strcpy(programs[k].progname, __argv[i]);
            strcpy(programs[k].prog_args[0], programs[k].progname);
            wait_for_arg = true;
            wait_for_prog = false;
            ++program_counter;
            ++programs[k].argc;
            l = 1;
        }
        else
        {

            if (match(__argv[i], REGEX_PATTERN_ARGNAME) || match(__argv[i], REGEX_PATTERN_PATHNAME))
            {

                for (j = i; j <= __argc; ++j)
                {

                    if ((match(__argv[j], REGEX_PATTERN_ARGNAME) || match(__argv[j], REGEX_PATTERN_PATHNAME)) && !wait_for_prog && wait_for_arg)
                        strcpy(programs[k].prog_args[l++], __argv[j]), ++programs[k].argc;
                    else if (match(__argv[j], REGEX_PATTERN_MODIFIER) && !is_modifier())
                    {

                        ++k;
                        check_modifier(__argv[j][0]);
                        break;
                    }
                    else
                    {

                        printf("Erro de sintaxe próximo ao token `%s'\n", __argv[j]);
                        return NULL;
                    }
                }

                i = j;
            }
            else if (match(__argv[i], REGEX_PATTERN_MODIFIER))
                check_modifier(__argv[i][0]), ++k;
            else
            {

                printf("Erro de sintaxe próximo ao token `%s'", __argv[i]);
                return NULL;
            }
        }

        ++i;
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

    switch (ch)
    {

        case '|':
            is_pipe = true;
            wait_for_arg = false;
            wait_for_prog = true;
            break;
        case '<':
            is_input = true;
            wait_for_arg = false;
            wait_for_prog = true;
            break;
        case '>':
            is_output = true;
            wait_for_arg = false;
            wait_for_prog = true;
            break;
        case '&':
            is_amper = true;
            wait_for_arg = false;
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

/* Testa para ver se algum modificador está setado. */
bool is_modifier(void)
{

    return is_pipe || is_input || is_output;
}

/* Muda o diretório atual para o diretório passado como argumento. */
int cdcall(int argc, char **argv)
{

    if (argv[1] != NULL)
    {
        
        char buff[PATH_MAX];
        char *aux = (char *)calloc(PATH_MAX << 1, sizeof(char));

        if (match(argv[1], REGEX_PATTERN_NPATHNAME))
            strcat(aux, getcwd(buff, PATH_MAX)), dirname(aux);
        else
            strcat(aux, argv[1]);

        if (chdir(aux) == -1)
            perror("Erro ao abrir o diretório");

        free(aux);

    }

    return 0;
}

/* Exibe o diretório atual. */
int pwdcall(int argc, char **argv)
{

    char buffer[PATH_MAX];
    if (argc == 1 || !strcmp(argv[1], "-L"))
        printf("%s\n", getenv("PWD"));
    else if (!strcmp(argv[1], "-P"))
        printf("%s\n", getcwd(buffer, PATH_MAX));
    else
        perror("Error");

    return 0;
}

/* Faz uma chamda de função utilizando redirecionamento de saída. 
   O programa1 escreve na entrada do programa2. */
int pipecall(prog_t *program)
{

    int fd[2];
    pipe(fd);

    int id1 = fork();

    if (id1 == -1)
    {

        perror("Erro");
        return errno;
    }
    else if (id1 == 0)
    {

        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]), close(fd[0]);

        char *myargs[10] = {NULL};

        for (size_t i = 0; i < program[0].argc; ++i)
            myargs[i] = program[0].prog_args[i];

        execvp(myargs[0], myargs);
    }

    int id2 = fork();
    if (id2 == -1)
    {

        perror("Error");
        return errno;
    }
    else if (id2 == 0)
    {

        dup2(fd[0], STDIN_FILENO);
        close(fd[1]), close(fd[0]);

        char *myargs[10] = {NULL};

        for (size_t i = 0; i < program[1].argc; ++i)
            myargs[i] = program[1].prog_args[i];

        execvp(myargs[0], myargs);
    }

    close(fd[0]);
    close(fd[1]);

    if (!is_amper)
    {

        wait(0);
        wait(0);
    }

    return 0;

}

/* Execua um programa, mas redireciona a saída desse programa para um arquivo passado como argumento. */
int outputcall(prog_t *program)
{

    int id = fork();
    if (id == -1)
    {

        perror("Erro");
        return errno;
    }
    else if (id == 0)
    {

        int file = open(program[1].progname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(file, STDOUT_FILENO);
        close(file);

        char *myargs[10] = {NULL};
        for (size_t i = 0; i < program[0].argc; ++i)
            myargs[i] = program[0].prog_args[i];

        execvp(myargs[0], myargs);
    }

    if (!is_amper)
        wait(0);

    return 0;

}

/* Executa um programa e utiliza como entrada o stream de um arquivo. */
int inputcall(prog_t *program)
{

    int id = fork();
    if (id == -1)
    {

        perror("Erro");
        return errno;
    }
    else if (id == 0)
    {

        int file = open(program[1].progname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(file, STDIN_FILENO);
        close(file);

        char *myargs[10] = {NULL};
        for (size_t i = 0; i < program[0].argc; ++i)
            myargs[i] = program[0].prog_args[i];

        execvp(myargs[0], myargs);
    }

    if (!is_amper)
        wait(0);

    return 0;

}