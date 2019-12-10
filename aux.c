#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <readline/readline.h> 
#include <readline/history.h> 
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

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

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}
 
void clearDisplay() {
    clear();
}

char* getUsernameDisplay() {
    // clearDisplay();
    // printf(“\033[0;31m”);
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
    sleep(1);
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
    int fdd = 0;
    char shm_name[] = "myshm";
    int shm_fd;
    shm_fd = shm_open(shm_name, O_CREAT| O_RDWR, S_IRUSR| S_IWUSR);
    if (shm_fd < 0) {
    	perror(NULL);
    	exit(1);
    }
    size_t shm_size = getpagesize();
    if (ftruncate(shm_fd, shm_size) == -1) {
    	perror(NULL);
    	shm_unlink(shm_name);
    	exit(1);
    }
    int *shm_ptr = mmap(0, shm_size, PROT_READ| PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
    	perror(NULL);
    	shm_unlink(shm_name);
    	exit(1);
    }

    pid_t pid, pid2;	

    char* trimmedInput;
    char** command;
    int commandId = 0;
    for (; input[commandId] != NULL; commandId++) {
        if (strlen(input[commandId]) == 0)
            continue;



        trimmedInput = trimInput(input[commandId]);
        command = parseInputBySeparator(trimmedInput, " ");
        printf("%d %s", strlen(command[0]), command[0]);
        pipe(fd);
        pid = fork();
        
        if (pid < 0) {
            perror("fork error: ");
            exit(1);
        } else if (pid == 0) {
        	shm_ptr[0] = 1;
        	if (strcmp(command[0], "help") == 0 && command[1] == NULL) {
        		openHelp();
        	} else if (strcmp(command[0], "cd") == 0) {
        		printf("cdcdcdccdc!!!!!!!\n");
                changeDirectory(command[1]);
            } else {
                dup2(fdd, 0);
                if (input[commandId + 1] != NULL)
                    dup2(fd[1], STDOUT_FILENO);

                close(fd[0]);
                execvp(command[0], command);
            }
            printf("Could not find command\n");
            exit(0);
        } else {

        	char ch;
        	pid2 = fork();
        	if (pid2 == 0) {

        		while(shm_ptr[0] == 1) {
        			if (kbhit()) {
        				ch = getchar();
        				if (ch == 27) { // 27 = escape
        					kill(pid, SIGKILL);
        				}
        			}
        		}
        		exit(1);

    		} 
    		else {
	            int status;
	            wait(NULL);
	            shm_ptr[0] = 2;
	            wait(NULL);
	            close(fd[1]);
	            fdd = fd[0];
	            munmap(shm_ptr, shm_size);
        	}

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
        // printf("%s \n", pipedInpput);
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