#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#define START 4
#define END 16
#define BUFFER_MAX 4096

typedef struct __entry{

    char *file_name;
    char *permissions;
    char *owner_name;
    char *group_name;
    char *dmod;
    long file_size;
    int color;
    int type;

} entry_t;

typedef enum {RED, BLE, GRN, DFT} colors_t;
typedef enum {FLGL, FLGT, FLGD, FLGS, FLGA} flag_t;

int compare_str_name(const void *, const void *);
int compare_date_mod(const void *, const void *);

char *formatTime(char *);
char *make_str_permission(const int, const bool);
inline bool checkBits(const int);
void printEntry(const char *, const char *, const char *, const char *, const long, const char *, const int, const char *);
void printEntry2(const char *, const entry_t *, const size_t);

char *colorsCodes[] = {"\033[1;31m", "\033[1;34m", "\033[1;32m", "\033[0m"};
int pflags[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};

int main(int argc, char **argv)
{
    
    // char buffer[PATH_MAX + 1];

    // printf("%s\n", getcwd(buffer, PATH_MAX + 1));
    // printf("%s\n", getenv("PWD"));
    // return 0;

    DIR *folder;
    struct group *grp;
    struct passwd *pwd;
    struct dirent *entry;
    struct stat filestat;
    unsigned entryCount = 0;
    char permissions[] = "----------";

    int (*compare[])(const void *, const void *) = {compare_str_name, compare_date_mod};

    folder = opendir(getenv("PWD"));

    if (folder == NULL)
    {

        perror("Unable to read directory");
        return 1;

    }

    // TODO: Refatorar essa monstruosidade;
    // TODO: Implementar checagem de erros;
    // TODO: Implementar comportamentos para flags -l -d -t -S -a

    char printString[] = "%s %d %-5.8s %-5.8s %6ld %s %s%s%s\n";
    entry_t *entries = (entry_t *) calloc(16, sizeof(entry_t));

    while ((entry = readdir(folder)))
    {

        stat(entry->d_name, &filestat);
        pwd = getpwuid(filestat.st_uid);
        grp = getgrgid(filestat.st_gid);

        bool is_dir = S_ISDIR(filestat.st_mode);
        bool is_exec = checkBits(filestat.st_mode);

        entries[entryCount].file_name = entry->d_name;
        entries[entryCount].permissions = make_str_permission(filestat.st_mode, is_dir);
        entries[entryCount].owner_name = pwd->pw_name;
        entries[entryCount].group_name = grp->gr_name;
        entries[entryCount].file_size = filestat.st_size;
        entries[entryCount].dmod = (formatTime(ctime(&filestat.st_mtime)));
        entries[entryCount].type = 1;
        entries[entryCount].color = DFT;

        if (is_exec)
        {

            if (is_dir && (entry->d_name[0] != '.'))
            {

                entries[entryCount].type = 2;
                entries[entryCount].color = BLE;

            }
            else if (!is_dir)
                entries[entryCount].color = GRN;

        }
        else
            entries[entryCount].color = DFT;

        ++entryCount;

    }

    qsort(entries, entryCount, sizeof(entry_t), compare_str_name);
    printEntry2(printString, entries, entryCount);
    closedir(folder);
    return 0;

}

/* Função checa e constroi a string de permissões para ser exibida no terminal. */
char *make_str_permission(const int mode, const bool is_dir)
{

    char *string = (char *) calloc(10, sizeof(char));
    strcpy(string, "----------");

    if (is_dir)
        string[0] = 'd';

    // Laço vai construir a string de permissões;
    // Aplica a máscara do índice i - 1 na variável de modo do arquivo e se for diferente de 0
    // string na posição i recebe o caracter no índice j % 3 apropriado;
    char *modes = "rwx";
    for (size_t i = 1ULL, j = 0ULL; string[i]; ++i, ++j)
        if (mode & pflags[i - 1ULL])
            string[i] = modes[j % 3ULL];

    return string;

}

/* Função formata a string de data da última modificação no arquivo. */
char *formatTime(char *string)
{

    string[END] = 0; // 'Corta' a string no índice END, as infos depois desse índice não são necessárias;
    string[START] += 32; // Transforma o caractere no índice START para minúsculo;

    size_t size = strlen(&string[START]);

    char *str_return = (char *) calloc(size, sizeof(char) + 1);
    memcpy(str_return, &string[START], size);

    return str_return;

}

/* Função chega se o arquivo é executável. */
bool checkBits(const int mode)
{

    return (mode & S_IXUSR) || (mode & S_IXGRP) || (mode & S_IXOTH);
}

/* Função imprime as infos da entrada contida no diretório. */
void 
printEntry(const char *str_formated, const char *str_permissions, const char *str_ow_name, const char *str_gr_name, const long file_size, const char *str_dmod, const int color, const char *file_name)
{

    printf(str_formated, str_permissions, str_ow_name, str_gr_name, file_size, str_dmod, colorsCodes[color], file_name, colorsCodes[DFT]);
}

int compare_str_name(const void *a, const void *b)
{

    return strcmp(((entry_t *)a)->file_name, ((entry_t *)b)->file_name);
}


int compare_date_mod(const void *a, const void *b)
{

    // Needs implementation;

}

void printEntry2(const char *str_formated, const entry_t *entry, const size_t size)
{

    // Por enquanto vou deixar assim...
    for (size_t i = 0; i < size; ++i)
        if (entry[i].file_name[0] != '.')
            printf(str_formated, entry[i].permissions, entry[i].type, entry[i].owner_name, entry[i].group_name, entry[i].file_size, entry[i].dmod, colorsCodes[entry[i].color], entry[i].file_name, colorsCodes[DFT]);

}