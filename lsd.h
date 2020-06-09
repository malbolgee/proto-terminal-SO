#include <time.h>

#pragma once

/* Abreviação para a função de ordenação. */
typedef void (*__sort_t)(void *, size_t, size_t, __compar_fn_t);

#define BUFFER_MAX 4096

#define IS_TFILE 0x1        // Define um arquivo;
#define IS_TDIR 0x2         // Define um diretório;
#define IS_FDIR 0x4         // Define um diretório de nível (. ou ..)

/* Máscaras para o parser das flags */

#define IS_L_FLAG 0x1       // Aciona o bit para -l
#define IS_A_FLAG 0x2       // Aciona o bit para -a
#define IS_T_FLAG 0x4       // Aciona o bit para -t
#define IS_S_FLAG 0x8       // Aciona o bit para -S
#define IS_LA_FLAG 0x3      // Testa para as flags -l -a
#define IS_LAT_FLAG 0x7     // Testa para as flags -l -a -t
#define IS_LAS_FLAG 0xB     // Testa para as flags -l -a -S
#define IS_LT_FLAG 0x5      // Testa para as flagas -l -t
#define IS_LS_FLAG 0x9      // Testa para as flags -l -S

// ======================================

#define REGEX_PATTERN_PATH "^\\/[/a-zA-Z0-9_]+$"     // Expressão regular para diretório;
#define REGEX_PATTERN_FPATH "^\\.{1,2}$"             // Expressão regular para diretório de nível;
// #define REGEX_PATTERN_FPATH

/* Struct que representa uma entidade recuperada do diretório. */
typedef struct __entry
{

    char *file_name;        // Nome do arquivo/diretório;
    char *permissions;      // String de permissões;
    char *owner_name;       // Nome do dono do arquivo/diretório;
    char *group_name;       // Nome do grupo do arquivo/diretório;
    long file_size;         // Tamanho do arquivo/diretório;
    int color;              // Código de cor para impresão;
    unsigned char type : 3; // Tipo de entrada (arquivo ou diretório);
    struct tm *dmod;        // Struct com a data de última modifiação feita;

} entry_t;

/* Enumeraval com os códigos de cores para impressão. */
typedef enum
{
    RED,
    BLE,
    GRN,
    DFT
} colors_t;

int compare_str_name(const void *, const void *);
int compare_date_mod(const void *, const void *);
int compare_file_size(const void *, const void *);

char *make_str_permission(const int, const bool);
inline bool check_x_permissions(const int);
inline struct tm *tm_alloc(const time_t *timer);
bool match(const char *, const char *);
void showOutput(entry_t *, size_t, const char *, unsigned int);
void showAllInfo(entry_t *, const size_t, const char *, __sort_t, __compar_fn_t);
void showMinInfo(entry_t *, const size_t, const char *, __sort_t, __compar_fn_t);