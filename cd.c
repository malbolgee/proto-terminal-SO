#define _XOPEN_SOURCE_EXTENDED 1
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char **argv){
    DIR* directory;
    char error[50];
    int i = 0;
    char buffer[PATH_MAX];        
    setenv("PWD",getenv("HOME"),1);  
    
    char *aux = malloc(sizeof(argv)*10); //aloca string aux na memoria e copia o argumento diretorio para ela
    strcpy(aux,getenv("PWD"));    
    
    if(strcmp(argv[1], "-P") && strcmp(argv[1], "-L")){ //se argv[1] não for -P ou -L
        printf("caso 1\n");
        
        if(strcmp(argv[1],"..") && strcmp(argv[1],"../")){ // .. e ../ são comandos para retroceder diretório 
            strcat(aux,"/");
            strcat(aux,argv[1]);
        
            if(argc > 2){  //concatenação de diretorio para o caso de pasta com nome composto ex /Área de Trabalho
                for(i = 2; i < argc; i++){
                    strcat(aux," ");
                    strcat(aux, argv[i]);
                }
            }
        }else{
            dirname(aux); 
        }
    }else{
        printf("caso 2\n");
        if(strcmp(argv[2],"..") && strcmp(argv[2],"../")){  // .. e ../ são comandos para retroceder diretório 
            if(!strcmp(argv[1], "-P")){
                setenv("PWD",getcwd(buffer, PATH_MAX),1);
                free(aux);
                aux = malloc(sizeof(argv)*10);
                strcpy(aux,getenv("CWD"));
            }
            
            strcat(aux,"/");
            strcat(aux,argv[2]);
        
            if(argc > 3){  //concatenação de diretorio para o caso de pasta com nome composto ex /Área de Trabalho
                for(i = 3; i < argc; i++){
                    strcat(aux," ");
                    strcat(aux, argv[i]);
                }
            }
        }else{
            dirname(aux); 
        }
    }

    for(i = 1; i < argc; i++){ //função responsável por trocar pasta em um mesmo diretorio
        if(!strncmp(argv[i],"../",3) && strlen(argv[i]) > 3){
            dirname(aux);
            dirname(aux);
            char* token = strtok(argv[i], "../");
            strcat(aux,"/");
            strcat(aux,token);
        }
    }

    //verifica se o diretorio existe
    directory = opendir(aux); 
    if (directory){
        printf("diretorio existe\n");
        chdir(aux);
        printf("%s\n", aux);                
    }   
    else if (ENOENT == errno){
        snprintf(error, sizeof error, "cd: %s", aux);
        perror(error);
    }   
    else{
        printf("erro ao abrir diretiorio\n");
    }
    free(aux);
    closedir(directory); 
    return 0;
 }       
        
        