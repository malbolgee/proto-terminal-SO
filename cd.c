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
    int i,j = 0;
    char buffer[PATH_MAX];        
    setenv("PWD",getenv("HOME"),1); //pega o home da maquina   
    
    char *aux = malloc(sizeof(argv)*10);//aloca string aux na memoria e copia o argumento diretorio para ela
    char *aux2 = malloc(sizeof(argv)*10); 
    strcpy(aux,getenv("PWD"));    
    
    if(strcmp(argv[1],"..") && strcmp(argv[1],"../")){ // .. e ../ são comandos para retroceder diretório 
        strcat(aux,"/");
        strcat(aux,argv[1]);
        
        if(argc > 2){  //concatenação de diretorio para o caso de pasta com nome composto ex /Área de Trabalho
            for(i = 2; i < argc; i++){
                strcat(aux," ");
                strcat(aux, argv[i]);
            }
        }
    }

    else{
        dirname(aux); 
    }
    
    for(i = 1; i < argc; i++){ //função responsável por trocar pasta em um mesmo diretorio
        if(!strncmp(argv[i],"../",3) && strlen(argv[i]) > 3){
            strcpy(aux2,argv[i]);
            for(j = i + 1; j < argc; j++){
                strcat(aux2," ");
                strcat(aux2,argv[j]);
            }
            dirname(aux); 
            dirname(aux); 
            char* token = strtok(aux2, "../"); 
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
        snprintf(error, sizeof error, "cd: %s\n", aux);
        perror(error);
    }   
    else{
        printf("erro ao abrir diretiorio\n");
    }
    free(aux);
    free(aux2);
    closedir(directory); 
    return 0;
}       
        
        