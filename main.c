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
#define PIPES_SIZE 100 
#define clear() printf("\033[H\033[J") 

/*
    To do:
        *cd 
        *pipes 
        *help command
        *exit (ctrl + c de genu)
        *free memory
*/

void clearDisplay() {
    clear();
}

char* getUsernameDisplay() {
    char *usrDisplay = malloc(USER_SIZE);
    strcpy(usrDisplay, "Best Moldavian GuyzZz@");
    strcat(usrDisplay, getenv("USER"));
    strcat(usrDisplay, ": ");
    return usrDisplay;
}

void printCurrentDirectory() {
    char cwd[1000];
    getcwd(cwd, sizeof(cwd));
    char* username = getUsernameDisplay();
    printf("%s%s\n", username, cwd);
    free(username);
}

void greetUser() {
    printf("\n\n\n\n\n");
    printf("               ^           \n");
    printf("               ^           \n");
    printf("              ^^^           \n");
    printf("              ^^^           \n");
    printf("             ^^^^^           \n");
    printf("             ^^^^^           \n");
    printf("           ^^^^^^^^^           \n");
    printf("          ^^^^^^^^^^^           \n");
    printf("          ^^^^^^^^^^^           \n");
    printf("        ^^^^^^^^^^^^^^^           \n");
    printf("       ^^^^^^^^^^^^^^^^^           \n");
    printf("       ^^^^^^^^^^^^^^^^^            \n");
    printf("     ^^^^^^^^^^^^^^^^^^^^^           \n");
    printf("    ^^^^^^^^^^^^^^^^^^^^^^^          \n");
    printf("    ^^^^^^^^^^^^^^^^^^^^^^^            \n");
    printf("  ^^^^^^^^^^^^^^^^^^^^^^^^^^^           \n");
    printf(" ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^           \n");
    printf("              | |\n");
    printf("              | |\n");
    printf("              | |\n");
    printf("              | |\n");
    printf("              | |\n");
    printf(" \n\n\n\n      WELCOME TO OUT SHELL!!    \n\n\n");
    sleep(2);
    clear();
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
        execvp(input[0], input);
        return errno;
    } else {
        int returnStatus;
        wait(&returnStatus);

        return returnStatus;
    }
}

int executeCommandAndGetOutput(char** input, int inputType) {
    pid_t pid;
    int pipefd[2]; 
    if (pipe(pipefd) < 0) { 
        return -1; 
    } 

    pid = fork();
    if (pid < 0) {
        return -1;
    } else if (pid == 0) {
        execvp(input[0], input);
        printf("Could not find command \"%s\"\n", input[0]);
        return errno;
    } else {
        int returnStatus;
        wait(&returnStatus);

        return returnStatus;
    }

    return 0;
}

void executePipeline(char **input) {
    int fd[2];
    pid_t pid;
    int fdd = 0;

    char* trimmedInput;
    char** command;
    int commandId = 0;
    for (; input[commandId] != NULL; commandId++) {
        if (strlen(input[commandId]) == 0)
            continue;

        trimmedInput = trimInput(input[commandId]);
        command = parseInputBySeparator(trimmedInput, " ");

        pipe(fd);
        pid = fork();
        
        if (pid < 0) {
            perror("fork error: ");
            exit(1);
        } else if (pid == 0) {
            dup2(fdd, 0);
            if (input[commandId + 1] != NULL)
                dup2(fd[1], 1);

            close(fd[0]);
            execvp(command[0], command);
            exit(1);
        } else {
            int status;
            wait(&status);
            close(fd[1]);
            fdd = fd[0];
        }
    }
}

int executePipedCommands(char **input) {
    int commandId = 0;
    char* trimmedInput;
    char** command;
    int inputType = 0;
    for (; input[commandId] != NULL; commandId++) {
        if (strlen(input[commandId]) == 0) 
            continue;

        trimmedInput = trimInput(input[commandId]);
        command = parseInputBySeparator(trimmedInput, " "); 
        if (executeCommandAndGetOutput(command, inputType) != 0) {
            return -1;
        }
        inputType = 1;
        // pipeline(command);
    }
}

int executeChainedCommand(char **parsedInput) {
    int commandId = 0;
    char* trimmedInput;
    char** pipedInput;
    for (; parsedInput[commandId] != NULL; commandId++) {
        if (strlen(parsedInput[commandId]) == 0) 
            continue;
        
        trimmedInput = trimInput(parsedInput[commandId]);
        pipedInput = parseInputBySeparator(trimmedInput, "|"); 

        // if (executePipedCommands(pipedInput) != 0) {
        //     return -1;
        // }

        executePipeline(pipedInput);
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
            // printf("Command not found\n");
            continue;
        }
    }

    return 0;
}