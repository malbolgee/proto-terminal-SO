#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *validCommands[] = {"ls", "cd", "pwd", ">"};
unsigned numberOfCommands = 4;

int main(int argc, char **argv){

    char *input, *stringToParse, *token, *command;
    char *tokens[10];
    const char delimiter[2] = " ";

    // int commandID;

    input = (char*) (calloc(10000, sizeof(char)));
    scanf("%[^\n]%*c", input);

    if(input[0] != 0){
        stringToParse = strdup(input);

        token = strtok(stringToParse, delimiter);
        // command = strdup(token);
    }

}

int commandChecker (char *stringToCheck, char **commandsList, unsigned numberOfCommands){
    unsigned i;

    for(i = 0; i < numberOfCommands; ++i){
        if(strcmp(stringToCheck, commandsList[i]) == 0){
            return i;
        }
    }

    return -1;

}

int tokenHandler (char *word, char *delimiter){

    int commandID = commandChecker(word, validCommands, numberOfCommands);
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
