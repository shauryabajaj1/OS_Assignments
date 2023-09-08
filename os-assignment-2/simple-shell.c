#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_COMMAND_LENGTH 1000
#define MAX_ARGS 100

char ** commandhistory[1000];

int commandcounter = 0;

char** nonsystemcommands[2] = {"cd", "exit"};

void print_cwd(){
    char directory[1000];
    getcwd(directory, sizeof(directory));
    printf("%s", directory);
}

void fork_process(char** tokens){
    int newpid = fork();

    if (newpid  == -1){
        printf("Failed to fork a new process.");
    }
    if (newpid == 0){
        if (execvp(tokens[0], tokens) == -1){
            printf("\nCould not execute that command.");
        }
    }
}
int get_input(char* command){
    int status = 0;
    char *temp;
    print_cwd();
    temp = readline("simple-shell~$ ");
    if (strlen(temp) != 0){
        strcpy(command, temp);
        commandhistory[commandcounter] = command;
        commandcounter++;
    }
    else {
        status = 1;
    }
    return status;
}

int number_of_tokens(char* command, char** tokens){
    int token_counter = 0;
    char* token = strtok(command, " ");

    while (token != NULL){
        tokens[token_counter] = token;
        token = strtok(NULL, " ");
    }
    return token_counter;
}

void get_args(char *command, char** tokens){
    for (int token = 0; token < MAX_ARGS; token++){
        tokens[token] = strsep(&command," ");
        if (tokens[token] == NULL){
            break;
        }
    }

}

int execute_ns_commands(char** tokens){
    int status = 0;
    int commandtoexecute = 0;
    for (int i = 0; i < 2; i++){
        if (strcmp(nonsystemcommands[i], tokens[0]) == 0){
            commandtoexecute = i;
            break;
        }

    }
    switch(commandtoexecute){
        case 0:
            exit(0);
        case 1:
            chdir(tokens[1]);
            status = 1;
    }
    return status;
}




