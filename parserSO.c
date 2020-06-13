#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

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
    scanf("%[^\n]%*c", input); // lê primeira string de entrada

    while(strcmp(input, "exit") != 0){ // continua lendo e processando entradas, até que a string "exit" seja recebida como entrada
        parserFlag = parserSO(input);

        scanf("%[^\n]%*c", input);
    }

}

int parserSO (char *input){

    char *stringToParse, *token, *command; // declara variáveis para tokenização da entrada
    char *tokenizedString[100];
    int tokenTypes[100];    // 0 para ls
                            // 1 para cd
                            // 2 para pwd
                            // 3 para programa
                            // 4 para argumentos
                            // 5 para nomes válidos
                            // 6 para modificadores
                            // -1 para o restante

    bool runParallel, setOutput, setInput, pipe; // declara flags de modificadores

    int commandID, patternTest, tokensIndex = 0;
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

        runParallel = setOutput = setInput = pipe = false; // inicializa flags com valores padrão

        stringToParse = strdup(input); // copia string de entrada (por precaução, já que strtok modifica a string)
        token = strtok(stringToParse, delimiter);

        while(token != NULL){
            tokenizedString[tokensIndex] = strdup(token);

            commandID = commandCheckerSO(token, validCommands, numberOfCommands);
            if(commandID != -1){
                tokenTypes[tokensIndex] = commandID;
            }
            else{
                if(modifierCheckerSO (token, validModifiers, numberOfModifiers) == 1){
                    tokenTypes[tokensIndex] = 6;
                }
                else{
                    tokenTypes[tokensIndex] = typeChecker(token, regexProg, regexArg, regexFile);
                }
            }

            ++tokensIndex;
            token = strtok(NULL, delimiter);
        }
        tokenizedString[tokensIndex] = 0;

        for(int i = 0; i < tokensIndex; ++i){
            printf("typeID: %d  token: %s\n", tokenTypes[i], tokenizedString[i]);
        }
        /*
        commandID = commandCheckerSO(token, validCommands, numberOfCommands);
        patternTest = patternCheckerSO(token, REGEX_PATTERN_RUNPROGRAM);

        if(patternTest == -1){
            puts("Falha na compilação da expressão regular.");
        }
        else{

            if((patternTest == 0) && (commandID == -1)){
                printf("%s: command not found\n", token);
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

    regfree(&regexProg);
    regfree(&regexArg);
    regfree(&regexFile);

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

    for(i = 0; i < numberOfModifiers; ++i){
        if(strcmp(stringToCheck, validModifiers[i]) == 0){
            return 1;
        }
    }

    return 0;
}

int typeChecker (char *string, regex_t progStruct, regex_t argStruct, regex_t nameStruct){
    if(patternCheckerSO (string, progStruct) == 1){
        return 3;
    }
    else{
        if(patternCheckerSO (string, argStruct) == 1){
            return 4;
        }
        else{
            if(patternCheckerSO (string, nameStruct) == 1){
                return 5;
            }
            else{
                return -1;
            }
        }
    }
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
/*
int parserSO (char *input){

    char *stringToParse, *token, *command; // declara variáveis para tokenização da entrada
    char *tokenizedString[100];
    int tokenTypes[100];    // 1 para comandos, 2 para argumentos, 3 para modificadores, 0 para o restante

    bool runParallel, setOutput, setInput, pipe; // declara flags de modificadores

    int commandID, patternTest, tokensIndex = 0;
    regex_t regexProg;

    if(input[0] != 0){ // checa se a entrada é uma string vazia (daria problema ao usar strtok)

        runParallel = setOutput = setInput = pipe = false; // inicializa flags com valores padrão

        stringToParse = strdup(input); // copia string de entrada (por precaução, já que strtok modifica a string)
        token = strtok(stringToParse, delimiter);

        commandID = commandCheckerSO(token, validCommands, numberOfCommands);
        patternTest = patternCheckerSO(token, REGEX_PATTERN_RUNPROGRAM);

        if(patternTest == -1){
            puts("Falha na compilação da expressão regular.");
        }
        else{

            if((patternTest == 0) && (commandID == -1)){
                printf("%s: command not found\n", token);
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
    }

}
*/
/*
int patternCheckerSO (const char *string, const char *pattern){
    //  retorna:
    //  -1 para erro de compilação da expressão regular
    //  0 para padrão não correspondente
    //  1 para string no padrão testado
    regex_t re;

    if(regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0){
        return -1;
    }
    else{
        int status = regexec(&re, string, 0, NULL, 0);
        regfree(&re);

        if(status != REG_NOMATCH){
            return 1;
        }
        else{
            return 0;
        }
    }
}
*/
/*
int tokenHandlerSO (char *word, char *delimiter){

    int commandID = commandCheckerSO(word, validCommands, numberOfCommands);
    unsigned i = 0;

    char* args[10];
    char* currentWord = strtok(NULL, delimiter);

    // while()

    switch (commandID){
        case 0:
            // executar ls
            printf("%s\n", validCommands[commandID]);
            break;

        case 1:
            // executar cd
            printf("%s\n", validCommands[commandID]);
            break;

        case 2:
            // executar pwd
            printf("%s\n", validCommands[commandID]);
            break;

        case 3:
            // executar >
            printf("%s\n", validCommands[commandID]);
            break;
                
        default:
            printf("%s: command not found\n", token);
    }

}
*/
