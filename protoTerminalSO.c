#define _XOPEN_SOURCE_EXTENDED 1

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

#include "protoTerminalSO.h"

// begin definições gerais
const char *validModifiers[] = {"<", ">", "|", "&"};
const char *validCommands[] = {"ls", "cd", "pwd"};
const char *customCommandNames[] = {"./lsd", "./cd", "./pwd"};
unsigned numberOfCommands = 3;
unsigned numberOfModifiers = 4;

const char delimiter[2] = " ";
// end

int main(int argc, char **argv){

    char *input, *stringToParse;

    int parserFlag;

    input = (char*) (calloc(MAX_ENTRY_LENGTH, sizeof(char))); // aloca variável para leitura de entradas, com comprimento definido em parserSO.h
    printf("%s$ ", getcwd(NULL, PATH_MAX));
    scanf("%[^\n]%*c", input); // lê primeira string de entrada

    while(strcmp(input, "exit") != 0){ // continua lendo e processando entradas, até que a string "exit" seja recebida como entrada
        parserFlag = parserSO(input);

        printf("%s$ ", getcwd(NULL, PATH_MAX));
        scanf("%[^\n]%*c", input);
    }

}

int parserSO (char *input){

    char *stringToParse, *token, *command; // declara variáveis para tokenização da entrada
    char *tokenizedString[100];
    char *argsString1[10] = { 0 };
    char *argsString2[10] = { 0 };
    unsigned args1Index = 0;
    unsigned args2Index = 0;
    int tokenTypes[100];    // 0 para ls
                            // 1 para cd
                            // 2 para pwd
                            // 3 para programa
                            // 4 para argumentos
                            // 5 para nomes válidos
                            // 6 para redirecionamento de input
                            // 7 para redirecionamento de output
                            // 8 para pipe
                            // 9 para execução em segundo plano
                            // -1 para o restante

    bool runParallel, setOutput, setInput, isPipe; // declara flags de modificadores
    char *customInput, *customOutput;

    // for(int k = 0; k < 10; ++k){
    //     printf("%s\n", argsString1[k]);
    // }

    int commandID, modifierID, patternTest, tokensIndex = 0;
    regex_t regexProg, regexArg, regexFile;

    if(regcomp(&regexProg, REGEX_PATTERN_RUNPROGRAM, REG_EXTENDED|REG_NOSUB) != 0){
        return -1;
    }
    if(regcomp(&regexArg, REGEX_PATTERN_ARGUMENT, REG_EXTENDED|REG_NOSUB) != 0){
        return -1;
    }
    if(regcomp(&regexFile, REGEX_PATTERN_FILENAME, REG_EXTENDED|REG_NOSUB) != 0){
        return -1;
    }

    if(input[0] != 0){ // checa se a entrada é uma string vazia (daria problema ao usar strtok)

        runParallel = setOutput = setInput = isPipe = false; // inicializa flags com valores padrão

        stringToParse = strdup(input); // copia string de entrada (por precaução, já que strtok modifica a string)
        token = strtok(stringToParse, delimiter);

        while(token != NULL){
            tokenizedString[tokensIndex] = strdup(token);

            commandID = commandCheckerSO(token, validCommands, numberOfCommands);
            // printf("commandID: %d\n", commandID);
            if(commandID != -1){
                tokenTypes[tokensIndex] = commandID;
            }
            else{
                modifierID = modifierCheckerSO(token, validModifiers, numberOfModifiers);
                if(modifierID != -1){
                    tokenTypes[tokensIndex] = modifierID;
                }
                else{
                    tokenTypes[tokensIndex] = typeChecker(token, regexProg, regexArg, regexFile);
                }
            }

            ++tokensIndex;
            token = strtok(NULL, delimiter);
        }
        tokenizedString[tokensIndex] = 0;

        // for(int l = 0; l < tokensIndex; ++l){
        //     printf("tokenizedString[%d]: %s\ntokenTypes[%d]: %d\n", l, tokenizedString[l], l, tokenTypes[l]);
        // }
        // puts("-----------------");
        
        for(int j = 0; j < tokensIndex; ++j){
            // printf("tokenTypes[%d] = %d\n", j, tokenTypes[j]);
            switch(tokenTypes[j]){
                case -1:
                    printf("%s : Token não reconhecido\n", tokenizedString[j]);
                    return -1;
                case 1:
                    argsString1[0] = strdup(tokenizedString[j]);

                    if(j < (tokensIndex - 1)){
                        argsString1[1] = strdup(tokenizedString[j+1]);
                    }

                    cdcall(argsString1, 2);
                    j = tokensIndex;

                    break;
                    /*
                    if(tokenizedString[j+1] != 0){
                        puts("print de caso.");
                        argsString1[0] = strdup(tokenizedString[j]);
                        argsString1[1] = strdup(tokenizedString[j+1]);

                        strcpy(argsString1[0], tokenizedString[j]);
                        strcpy(argsString1[1], tokenizedString[j+1]);

                        cdcall(argsString1, 2);
                        j = tokensIndex;
                    }
                    break;
                    */

                default:
                    break;
                
                // case 5:
                //     break;
                // case 6:
                //     if(!setInput){
                //         if((j < (tokensIndex-1)) && (tokenTypes[j+1] == 5)){
                //             setInput = true;
                //             customInput = strdup(tokenizedString[j+1]);
                //             ++j;
                //         }
                //         else{
                //             puts("Operador '<' requer um nome de arquivo como argumento.");
                //             return -1;
                //         }
                //     }
                //     else{
                //         puts("Terminal não suporta múltiplos redirecionamentos de entrada.");
                //     }
                //     break;
                // case 7:
                //     if(!setOutput){
                //         if((j < (tokensIndex-1)) && (tokenTypes[j+1] == 5)){
                //             setOutput = true;
                //             customOutput = strdup(tokenizedString[j+1]);
                //             ++j;
                //         }
                //         else{
                //             puts("Operador '>' requer um nome de arquivo como argumento.");
                //             return -1;
                //         }
                //     }
                //     else{
                //         puts("Terminal não suporta múltiplos redirecionamentos de saída.");
                //         return -1;
                //     }
                //     break;
                // case 8:
                //     if(!isPipe){
                //         if((j < (tokensIndex-1)) && (tokenTypes[j+1] == 3)){
                //             isPipe = true;
                //         }
                //     }
                //     else{
                //         puts("Terminal não suporta múltiplas instâncias de pipe.");
                //         return -1;
                //     }
                //     break;
                // case 9:
                //     runParallel = true;
                //     break;

                // case 0:
                //     puts("teste do case 0.");
                //     printf("%d\n", args1Index);
                //     strcpy(argsString1[args1Index], customCommandNames[0]);
                //     ++args1Index;
                //     break;
                // default:
                //     puts("print do default.");
                //     strcpy(argsString1[args1Index], tokenizedString[j]);
                //     // argsString1[args1Index] = strdup(tokenizedString[j]);
                //     ++args1Index;
                
            }
            
            // char *emptyString = (char *) (NULL);
            // strcpy(argsString1[args1Index], emptyString);
        }

        // for(int k = 0; k < 10; ++k){
        //     printf("-=-%s\n", argsString1[k]);
        // }

        if(strcmp(argsString1[0], "cd") != 0){
            int id = fork();
            printf("argsString[0] = \nargsString[1] = \n");
            printf("id do processo: %d\n", id);
            if(id == 0){
                puts("teste do ifammm.");
                if(setInput){
                    int fileDIn = open(customInput, O_RDONLY);
                    dup2(fileDIn, STDIN_FILENO);
                }
                if(setOutput){
                    int fileDOut = open(customInput, O_WRONLY|O_CREAT|O_TRUNC);
                    dup2(fileDOut, STDOUT_FILENO);
                }
                const char *argsConst[] = {argsString1[0], argsString1[1], 0};
                printf("argsConts[0] = %s\nargsConst[1] = %s\n", argsConst[0], argsConst[1]);
                execvp(argsConst[0], argsConst);
                // execvp(argsString1[0], &argsString1);
                puts("teste do execvp.");
            }
            else{
                if(!runParallel){
                    wait(0);
                }
            }
        }
        
        /*
        if((3 < tokenTypes[0]) && (tokenTypes[0] < 0)){
            printf("%s: token não é válido", tokenizedString[0]);
            return -1;
        }
        else{
            if(strcmp(tokenizedString[0], "cd") == 0){
                const char *args[2] = {"cd", tokenizedString[1]};
                cdcall(args, 2);
            }
            else{

            }
        }
        */
        /*
        for(int i = 1; i < tokensIndex; ++i){
            printf("typeID: %d  token: %s\n", tokenTypes[i], tokenizedString[i]);
            
        }
        */
        /*
        commandID = commandCheckerSO(token, validCommands, numberOfCommands);
        patternTest = patternCheckerSO(token, REGEX_PATTERN_RUNPROGRAM);

        if(patternTest == -1){
            puts("Falha na compilação da expressão regular.");
        }
        else{

            if((patternTest == 0) && (commandID == -1)){
                printf("%s: comando não reconhecido.\n", token);
            }
            else{
                while(token != NULL){
                    tokenizedString[tokensIndex] = strdup(token);
                    tokenTypes[tokensIndex] = 1;
                    ++tokensIndex;
                    
                    token = strtok(NULL, delimiter);


                }

                tokenizedString[tokensIndex] = 0;
                for(int i = 0; i < tokensIndex; ++i){
                    printf("%s\n", tokenizedString[i]);
                }
            }
        }
        */
    }

    // regfree(&regexProg);
    // regfree(&regexArg);
    // regfree(&regexFile);

    // free(customInput);
    // free(customOutput);
    // free(stringToParse);

}

int commandCheckerSO (char *stringToCheck, const char **commandsList, unsigned numberOfCommands){
    unsigned i;

    for(i = 0; i < numberOfCommands; ++i){
        if(strcmp(stringToCheck, commandsList[i]) == 0){
            return i;
        }
    }

    return -1;

}

int modifierCheckerSO (char *stringToCheck, const char **modifiersList, unsigned numberOfModifiers){
    unsigned i;
    // printf("Testando para modificadores... Entrada: %s\n", stringToCheck);

    for(i = 0; i < numberOfModifiers; ++i){
        // printf("Símbolo a comparar com entrada: %s\n", modifiersList[i]);
        if(strcmp(stringToCheck, modifiersList[i]) == 0){
            // printf("Match entre '%s' e '%s', resultado de saída: %d\n", stringToCheck, modifiersList[i], (i + 6));
            return (i + 6);
        }
    }

    return -1;
}

int typeChecker (char *string, regex_t progStruct, regex_t argStruct, regex_t nameStruct){
    if(patternCheckerSO (string, progStruct) == 1){
        return 3;
    }

    if(patternCheckerSO (string, argStruct) == 1){
        return 4;
    }

    if(patternCheckerSO (string, nameStruct) == 1){
        return 5;
    }
    
    return -1;
}

int patternCheckerSO (const char *string, regex_t patternStruct){
    /*  retorna:
        0 para padrão não correspondente
        1 para string no padrão testado
    */

    int status = regexec(&patternStruct, string, 0, NULL, 0);

    if(status != REG_NOMATCH){
        return 1;
    }
    else{
        return 0;
    }
}

int cdcall(char **argv, int argc){
    DIR* directory;
    char error[50];
    int i, j = 0;
    char buffer[PATH_MAX]; 

    // for(int i = 0; i < argc; ++i){
    //     printf("%s\n", argv[i]);
    // }
    
    char *aux = malloc(sizeof(char) * 100);//aloca string aux na memoria e copia o argumento diretorio para ela
    // char *aux2 = malloc(sizeof(char) * 100); 
    strcpy(aux,getcwd(NULL, PATH_MAX));

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
    /*
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
    */
    //verifica se o diretorio existe
    directory = opendir(aux); 
    if (directory){
        chdir(aux);           
    }
    else{
        perror("Erro ao abrir diretorio");
    }
    free(aux);
    closedir(directory); 
    return 0;
}       
