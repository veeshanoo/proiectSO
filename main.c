#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <readline/readline.h> 
#include <readline/history.h> 
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#define USER_SIZE 1000
#define PARSER_SIZE 1000
#define ARGUMENTS_SIZE 100
#define PIPES_SIZE 100 
#define clear() printf("\033[H\033[J") 

/*
    To do:
        *cd -- done
        *pipes -- done 
        *help command
        *exit (ctrl + c de genu)
        *free memory -- important
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
    printf("%s\n", cwd);
    free(username);
}

void greetUser() {
    printf("\033[0;32m");
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
    printf("\033[0;35m");
    printf("              | |\n");
    printf("              | |\n");
    printf("              | |\n");
    printf("              | |\n");
    printf("              | |\n");
    printf("\033[01;33m");
    printf(" \n\n\n\n      WELCOME TO OUR SHELL!!    \n\n\n");
    printf("\033[0m"); 
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

void openHelp() {
	puts("\n***WELCOME TO MY SHELL HELP***"
        "\nCopyright © mihaiciv, flibia and francurichard"
        "\n-You can use supported linux shell commands and the above ones:"
        "\n - CTRL-O for interupting the current program"
        "\n - help"
        "\n - closecomputer for closing your PC");
}

int changeDirectory(char *pth) {
    char path[1000];
    
    strcpy(path, pth);
    if (path[0] != '/') {
        char cwd[1000];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror(NULL);
            return errno;
        }
        strcat(cwd, "/");
        strcat(cwd, path);
        if(chdir(cwd)) {
            perror(NULL);
            return errno;
        }
    } else {
        if(chdir(path)) {
            perror(NULL);
            return errno;
        }
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
        free(trimmedInput);
        pipe(fd);
        
        if (strcmp(command[0], "help") == 0 && command[1] == NULL) {
            openHelp();
        } else if (strcmp(command[0], "cd") == 0) {
            changeDirectory(command[1]);
        } else {
            pid = fork();
            if (pid < 0) {
                perror("fork error: ");
                exit(1);
            } else if (pid == 0) {
                signal(SIGINT, SIG_DFL);
            	
                dup2(fdd, 0);
                if (input[commandId + 1] != NULL)
                    dup2(fd[1], STDOUT_FILENO);

                close(fd[0]);
                execvp(command[0], command);
                printf("Could not find command \"%s\"\n", input[0]);  
            } else {
                int status;
                signal(SIGINT, SIG_IGN);
                wait(&status);
                signal(SIGINT, SIG_DFL);
                close(fd[1]);
                fdd = fd[0];
            }
        }


    }
}

void executeChainedCommand(char **parsedInput) {
    int commandId = 0;
    char* trimmedInput;
    char** pipedInput;
    for (; parsedInput[commandId] != NULL; commandId++) {
        if (strlen(parsedInput[commandId]) == 0) 
            continue;
        
        trimmedInput = trimInput(parsedInput[commandId]);
        pipedInput = parseInputBySeparator(trimmedInput, "|"); 

        executePipeline(pipedInput);

        free(trimmedInput);
        int size = sizeof(pipedInput) / sizeof(char*);
        for (int i = 0; i < size; i++)
            free(pipedInput[i]);
                
        free(pipedInput);
    }
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

        executeChainedCommand(parsedInput);
        
        // clearing stuff 
        free(currentInput);
        free(trimmedInput);
        int size = sizeof(parsedInput) / sizeof(char*);
        for (int i = 0; i < size; i++)
            free(parsedInput[i]);

        free(parsedInput);
    }

    return 0;
}