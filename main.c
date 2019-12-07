#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h> 
#include<readline/history.h> 
#include <errno.h>

#define USER_SIZE 1000
#define PARSER_SIZE 1000
#define ARGUMENTS_SIZE 100
#define clear() printf("\033[H\033[J") 

void clearDisplay() {
    clear();
}

void greetUser() {
    printf("\n\n\n\n\nhello user\n\n\n\n\n\n");
    // sleep(1);
    clear();
}

char* getUsernameDisplay() {
    char *usrDisplay = malloc(USER_SIZE);
    strcpy(usrDisplay, "Best Moldavian GuyzZz@");
    strcat(usrDisplay, getenv("USER"));
    strcat(usrDisplay, ": ");
    return usrDisplay;
}

char* takeInput() {
    char *buff = malloc(PARSER_SIZE * sizeof(char)), *username;

    username = getUsernameDisplay();
    buff = readline(username);
    if (buff != NULL) {
        add_history(buff);
    } 
    return buff;
}

char* trimInput(char* input) {
    int cursorLeft = 0, cursorRight = strlen(input) - 1;
    while (cursorLeft <= cursorRight && input[cursorLeft] == ' ')
        cursorLeft++;

    while (cursorRight >= cursorLeft && input[cursorRight] == ' ')
        cursorRight--;

    input[cursorRight + 1] = 0;
    char* buff = malloc((cursorRight - cursorLeft + 2) * sizeof(char));
    strcpy(buff, input + cursorLeft);
    buff[strlen(buff)] = 0;

    return buff;
}

char** parseInputBySeparator(char *input, char* separator) {
    char **inputArguments = malloc(ARGUMENTS_SIZE * sizeof(char*));
    int inputArgumentsSize = 0;
    char *currentWord;

    while ((currentWord = strsep(&input, separator)) != NULL) {
        // printf("--- %d --- %s\n", strlen(currentWord), currentWord);
        inputArguments[inputArgumentsSize] = malloc(strlen(currentWord) * sizeof(char));
        strcpy(inputArguments[inputArgumentsSize], currentWord);
        inputArgumentsSize++;
    }

    inputArguments[inputArgumentsSize] = NULL;

    return inputArguments;
} 

int executeCommand(char **input) {
    pid_t pid;
    pid = fork();

    if (pid < 0) {
        return -1;
    } else if (pid == 0) {
        if (execvp(input[0], input) != 0) {
            return errno;
        }
        exit(0);
    } else {
       wait(NULL);
       return 0;
    }
}

int executeChainedCommand(char **parsedInput) {
    int commandId = 0;
    char* trimmedInput;
    for (; parsedInput[commandId] != NULL; commandId++) {
        if (strlen(parsedInput[commandId]) == 0) 
            continue;
        
        trimmedInput = trimInput(parsedInput[commandId]);
        if (executeCommand(parseInputBySeparator(trimmedInput, " ")) != 0) {
            return -1;
        }
    }

    return 0;
}


int main() {
    greetUser();

    char *currentInput;
    char *trimmedInput;
    char **parsedInput;

    while (1) {
        currentInput = takeInput();
        trimmedInput = trimInput(currentInput);
        parsedInput = parseInputBySeparator(trimmedInput, "&&");

        if (executeChainedCommand(parsedInput) != 0) {
            printf("Command not found\n");
            continue;
        }

        
    }


    return 0;
}