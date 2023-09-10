#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <time.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1000
#define MAX_ARGS 100

char* command_history[1000];

double execution_times[1000];

pid_t process_IDs[1000];

int commandcounter = 0;

void print_cwd(){
    char directory[1000];
    getcwd(directory, sizeof(directory));
    printf("%s", directory);
}

void get_command(char* command){
    int status = 0;
    printf("simple-shell~$ ");
    status = 1;
    fgets(command, MAX_COMMAND_LENGTH, stdin);
    command[strcspn(command, "\n")] = '\0';
    if (strlen(command) != 0) {
        command_history[commandcounter] = command;
        commandcounter++;
    }

}

void get_args(char* command, char** tokens){
    int token_counter;
    for (token_counter = 0; token_counter < MAX_ARGS; token_counter++) {
        tokens[token_counter] = strsep(&command, " ");
        if (tokens[token_counter] == NULL)
            break;
        if (strlen(tokens[token_counter]) == 0)
            token_counter -= 1;
    }
    
}


void execute_system_command(char** tokens){
    pid_t newpid = fork();
    int status;
    if (newpid  == -1){
        perror("Failed to fork a new process.");
        return;
    }
    else if (newpid == 0){
        if (execvp(tokens[0], tokens) == -1){
            printf("\nCould not execute that command.");
        }
        exit(0);
    }
    else {
        process_IDs[commandcounter] = newpid;
        waitpid(newpid, &status, 0);
    }

}

void execute_piped_commands(char** tokens, char** piped_tokens) {
    pid_t p1;
    pid_t p2;
    int pipefds[2];

    if (pipe(pipefds) == -1) {
        printf("Couldn't form pipe");
        return;
    }
    
    p1 = fork();

    if (p1 == -1) {
        printf("Couldn't fork the process");
        return;
    }

    if (p1 == 0) {
        close(pipefds[0]);
        dup2(pipefds[1], STDOUT_FILENO);
        close(pipefds[1]);
        if (execvp(tokens[0], tokens) == -1) {
            printf("Couldn't execute that command");
            exit(EXIT_SUCCESS);
        }
    }
    else {
        p2 = fork();
        if (p2 == -1) {
            printf("Couldn't fork a new process");
            return;
        }
        if (p2 == 0) {
            close(pipefds[1]);
            dup2(pipefds[0], STDIN_FILENO);
            if (execvp(piped_tokens[0], piped_tokens) == -1) {
                printf("Couldn't execute that command");
                exit(0);
            }
        }
        else {
            wait(NULL);
            wait(NULL);
        }
    }
}

int check_pipe(char* command, char** tokens){
    int counter;
    int status = 0;
    for (counter = 0; counter < 2; counter++){
        tokens[counter] = strsep(&command, "|");
        if (tokens[counter] == NULL){
            break;
        }
    if (tokens[1] != NULL){
        status = 0;
    }
    else {
        status = 1;
    }
    }
    return status;
}

int check_command_type(char* command, char** tokens, char** piped_tokens) {
    char* piped_command[2];
    int ifpiped = 0;
    int status = 0;
    ifpiped = check_pipe(command, piped_command);

    if (ifpiped) {
        get_args(piped_command[0], tokens);
        get_args(piped_command[1], piped_tokens);
        status = 1;
    }
    else {
        get_args(command, tokens);
    }
    return status;
}


int main(){
    char command[MAX_COMMAND_LENGTH];
    char* tokens[MAX_ARGS];
    char* pipedtokens[MAX_ARGS];

    while (1) {
        print_cwd();
        printf("/");
        get_command(command);
        get_args(command, tokens);
        
        if (strcmp("exit", command) == 0) {
            break;
        }

        if (strcmp("cd", command) == 0) {
            chdir(tokens[1]);
            continue;
        }

        if (strcmp("history", command) == 0) {
            for (int i = 0; i < commandcounter; i++){
                printf("Command: %s\n", command_history[i]);
                printf("PID: %d\n", process_IDs[i]);
                printf("Time taken to execute: %.2f seconds\n", execution_times[i]);
            }
        }

        int pipecheck = check_command_type(command, tokens, pipedtokens);
        if (pipecheck) {
            execute_piped_commands(tokens, pipedtokens);
        }
        else {
            execute_system_command(tokens);
        }

    }
    return 0;
}
