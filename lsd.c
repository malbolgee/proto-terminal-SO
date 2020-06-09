#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <regex.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "lsd.h"

const char *colorsCodes[] = {"\033[1;31m", "\033[1;34m", "\033[1;32m", "\033[0m"};
const int pflags[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP, S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};

int main(int argc, char **argv)
{

    unsigned idx = 0;
    unsigned char flag = 0;
    bool is_l, is_t, is_s, is_a, has_path;

    // printf("-l: %8d\n", IS_L_FLAG);
    // printf("-l -a: %8d\n", IS_L_FLAG | IS_A_FLAG);
    // printf("-l -a -t: %8d\n", IS_L_FLAG | IS_A_FLAG | IS_T_FLAG);
    // printf("-l -a -S: %8d\n", IS_L_FLAG | IS_A_FLAG | IS_S_FLAG);
    // printf("-l -t: %8d\n", IS_L_FLAG | IS_T_FLAG);
    // printf("-l -S: %8d\n", IS_L_FLAG | IS_S_FLAG);
    // printf("-a: %8d\n", IS_A_FLAG);
    // printf("-S: %8d\n", IS_S_FLAG);
    // printf("-t: %8d\n", IS_T_FLAG);

    if (argc >= 2)
    {

        is_l = is_a = is_t = is_s = has_path = false;
        for (size_t i = 1; i < argc; ++i)
        {

            bool is_match = match(argv[i], REGEX_PATTERN_PATH) || match(argv[i], REGEX_PATTERN_FPATH);
            if ((strcmp(argv[i], "-l") == 0) && !is_t)
                flag |= IS_L_FLAG, is_l = true;
            else if ((strcmp(argv[i], "-a") == 0) && !is_a)
                flag |= IS_A_FLAG, is_a = true;
            else if ((strcmp(argv[i], "-t") == 0) && !is_t && !is_s)
                flag |= IS_T_FLAG, is_t = true;
            else if ((strcmp(argv[i], "-S") == 0) && !is_s && !is_t)
                flag |= IS_S_FLAG, is_s = true;
            else if (is_match && !has_path)
                idx = i, has_path = true;
            else
            {
                printf("Erro de sintaxe próximo ao token inesperado `%s'\n", argv[i]);
                return 1;

            }

        }

    }

    DIR *folder;
    struct group *grp;
    struct passwd *pwd;
    struct dirent *entry;
    struct stat filestat;
    unsigned entryCount = 0;

    if (has_path)
        chdir(argv[idx]), folder = opendir(getcwd(NULL, PATH_MAX));
    else
        folder = opendir(getcwd(NULL, PATH_MAX));

    if (folder == NULL)
    {

        printf("Unable to read directory");
        perror("Unable to read directory");
        return 1;
    }

    // Aloca vetor do tipo entry_t com 100 posições, por enquanto vou deixar assim
    // mas se tiver mais do que 100 entradas no diretório, vai dar problema;
    entry_t *entries = (entry_t *)calloc(100, sizeof(entry_t));

    // Esse laço recupera o que tem dentro de folder;
    while ((entry = readdir(folder)))
    {

        stat(entry->d_name, &filestat);
        pwd = getpwuid(filestat.st_uid);
        grp = getgrgid(filestat.st_gid);

        bool is_dir = S_ISDIR(filestat.st_mode);
        bool is_exec = check_x_permissions(filestat.st_mode);

        bool is_fdir = match(entry->d_name, REGEX_PATTERN_FPATH); 
        if (entry->d_name[0] == '.')
            if (!is_fdir)               // Quero saber se o arquivo é oculto, se for, só passa direto;
                continue;
            else if (is_a)              // Se a flag -a foi passada, então eu preciso imprimir diretório de nível (. e ..);
                entries[entryCount].type |= IS_FDIR;
            else if (!is_a)
                continue;

        // TODO: Revisar esse if acima, acho que não precisa dessa flag pra diretório de nível;
        
        entries[entryCount].file_name   = entry->d_name;
        entries[entryCount].permissions = make_str_permission(filestat.st_mode, is_dir);
        entries[entryCount].owner_name  = pwd->pw_name;
        entries[entryCount].group_name  = grp->gr_name;
        entries[entryCount].file_size   = filestat.st_size;
        entries[entryCount].dmod        = tm_alloc(&filestat.st_mtime);
        entries[entryCount].type        |= is_dir ? IS_TDIR : IS_TFILE;
        entries[entryCount].color       = DFT;

        if (is_exec)
            if (is_dir)
                entries[entryCount].color = BLE;
            else
                entries[entryCount].color = GRN;

        ++entryCount;
    }

    char *formated_str = is_l ? "%s %d %-5.8s %-5.8s %6ld %s %s%s%s\n" : "%s%s%s%c";
    showOutput(entries, entryCount, formated_str, flag);
    
    closedir(folder);
    return 0;
}

/* Função checa e constroi a string de permissões para ser exibida no terminal. */
char *make_str_permission(const int mode, const bool is_dir)
{

    const char *modes = "rwx";
    char *string = (char *)calloc(10, sizeof(char));
    strcpy(string, "----------");

    if (is_dir)
        string[0] = 'd';

    // Laço vai construir a string de permissões;
    // Aplica a máscara do índice i - 1 na variável de modo do arquivo e se for diferente de 0
    // a string na posição i recebe o caracter no índice j % 3 apropriado;
    for (size_t i = 1ULL, j = 0ULL; string[i]; ++i, ++j)
        if (mode & pflags[i - 1ULL])
            string[i] = modes[j % 3ULL];

    return string;
}

/* Função checa se o arquivo é executável. */
bool check_x_permissions(const int mode)
{

    return (mode & S_IXUSR) || (mode & S_IXGRP) || (mode & S_IXOTH);
}

/* Função auxiliar para comparação de nomes dos arquivos/diretório. */
int compare_str_name(const void *a, const void *b)
{

    return strcmp(((entry_t *)a)->file_name, ((entry_t *)b)->file_name);
}

/* Função auxiliar para comparação da datas de modificação dos arquivos/diretório. */
int compare_date_mod(const void *a, const void *b)
{

    entry_t *ai = (entry_t *)a;
    entry_t *bi = (entry_t *)b;

    // Soma 1900 porque o campo ano da struct tem o valor da quantidade de anos que passou desde 1900;
    int yresult     = (ai->dmod->tm_year + 1900) - (bi->dmod->tm_year + 1900); 
    int hresult     = ai->dmod->tm_hour - bi->dmod->tm_hour;
    int mnresult    = ai->dmod->tm_min - bi->dmod->tm_min;
    int scresult    = ai->dmod->tm_sec - bi->dmod->tm_min;
    int dyresult    = ai->dmod->tm_yday - bi->dmod->tm_yday;

    /* Forçando retorno negativo para ordem decrescente. */
    if (!yresult)
        if (!dyresult)
            if (!hresult)
                if (!mnresult)
                    return -scresult;
                else
                    return -mnresult;
            else
                return -hresult;
        else
            return -dyresult;
    else
        return -yresult;
}

/* Função auxiliar para comparação do tamanho dos arquivos/diretório. */
int compare_file_size(const void *a, const void *b)
{

    return -(((entry_t *)a)->file_size - ((entry_t *)b)->file_size);
}

/* Função aloca uma struct tm com a data de TIMER */
struct tm *tm_alloc(const time_t *timer)
{

    struct tm *aux = (struct tm *)calloc(1, sizeof(struct tm));
    memcpy(aux, localtime(timer), sizeof(struct tm));
    return aux;
}


bool match(const char *string, const char *pattern)
{

    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0)
        return false;

    int status = regexec(&re, string, 0, NULL, 0);
    regfree(&re);

    return status != REG_NOMATCH ? true : false;

}

/* Seleciona de acordo com FLAG qual tipo de saída deve ser impressa. */
void showOutput(entry_t *entry, size_t size, const char *string, unsigned int flag)
{

    switch (flag)
    {

        case (IS_L_FLAG):
            showAllInfo(entry, size, string, qsort, compare_str_name);
            break;
        case (IS_A_FLAG):
            showMinInfo(entry, size, string, qsort, compare_str_name);
            break;
        case (IS_S_FLAG):
            showMinInfo(entry, size, string, qsort, compare_file_size);
            break;
        case (IS_T_FLAG):
            showMinInfo(entry, size, string, qsort, compare_date_mod);
            break;
        case (IS_LA_FLAG):
            showAllInfo(entry, size, string, qsort, compare_str_name);
            break;
        case (IS_LAT_FLAG):
            showAllInfo(entry, size, string, qsort, compare_date_mod);
            break;
        case (IS_LAS_FLAG):
            showAllInfo(entry, size, string, qsort, compare_file_size);
            break;
        case (IS_LT_FLAG):
            showAllInfo(entry, size, string, qsort, compare_date_mod);
            break;
        case (IS_LS_FLAG):
            showAllInfo(entry, size, string, qsort, compare_file_size);
            break;
        default:
            showMinInfo(entry, size, string, qsort, compare_str_name);
            
    }

}

/* Ordena ENTRY com SORT utilizando uma função FUNC_AUX para comparação e imprime a informação completa da entrada. */
void showAllInfo(entry_t *entry, const size_t size, const char *string, __sort_t ord_func, __compar_fn_t func_aux)
{

    char str_aux_dmod[BUFFER_MAX];
    ord_func(entry, size, sizeof(entry_t), func_aux);
    for (size_t i = 0; i < size; ++i)
    {   
        strftime(str_aux_dmod, sizeof(str_aux_dmod), "%b %-2d %H:%M", entry[i].dmod);
        printf(string, entry[i].permissions, entry[i].type, entry[i].owner_name, entry[i].group_name, entry[i].file_size, str_aux_dmod, colorsCodes[entry[i].color], entry[i].file_name, colorsCodes[DFT]);
    }

}

/* Ordena ENTRY com QSORT utilizando uma função FUNC_AUX para comparação e imprime a informação reduzida da entrada. */
void showMinInfo(entry_t *entry, const size_t size, const char *string, __sort_t ord_func, __compar_fn_t func_aux)
{
    ord_func(entry, size, sizeof(entry_t), func_aux);
    for (size_t i = 0; i < size; ++i)
        printf(string, colorsCodes[entry[i].color], entry[i].file_name, colorsCodes[DFT], i < size - 1 ? ' ' : '\n');
}
