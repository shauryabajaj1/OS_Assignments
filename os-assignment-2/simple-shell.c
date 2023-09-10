#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMAND_LENGTH 1000
#define MAX_ARGS 100

volatile sig_atomic_t ctrlc = 0;

char* command_history[1000];

double execution_times[1000];

pid_t process_IDs[1000];

int commandcounter = 0;

void print_cwd(){
    char directory[1000];
    getcwd(directory, sizeof(directory));
    printf("%s", directory);
}

void ctrlc_handler(int signumber){
    ctrlc = 1;
}

void get_command(char* command){
    int status = 0;
    printf("simple-shell~$ ");
    status = 1;
    fgets(command, MAX_COMMAND_LENGTH, stdin);
    command[strcspn(command, "\n")] = '\0';
    if (strlen(command) != 0 && commandcounter < 1000 && strcmp("history", command) != 0) {
        command_history[commandcounter] = strdup(command);
        commandcounter++;
    }

}

void get_args(char* command, char** tokencommands)
{
    int i;
  
    for (i = 0; i < MAX_ARGS; i++) {
        tokencommands[i] = strsep(&command, " ");
  
        if (tokencommands[i] == NULL)
            break;
        if (strlen(tokencommands[i]) == 0)
            i--;
    }
}

void execute_system_command(char** tokens){
    pid_t newpid = fork();
    int status;
    int i = 0;
    if (newpid  == -1){
        printf("\nFailed to fork child process.");
        return;
    }
    else if (newpid == 0){
        process_IDs[commandcounter] = newpid;
        if (execvp(tokens[0], tokens) < 0) {
            printf("Couldn't execute that command\n");
        }
        exit(0);
    }
    else {
        process_IDs[commandcounter] = newpid;
        wait(NULL);
        return;
    }

}

void execute_piped_commands(char** tokens, char** piped_tokens) {
    int p1;
    int p2;
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
            printf("Couldn't execute that command\n");
            exit(EXIT_SUCCESS);
        }
    }
    else {
        p2 = fork();
        if (p2 == 0) {
            close(pipefds[1]);
            dup2(pipefds[0], STDIN_FILENO);
            if (execvp(piped_tokens[0], piped_tokens) == -1) {
                printf("Couldn't execute that command\n");
                exit(EXIT_SUCCESS);
            }
        }
        else {
            commandcounter++;
            wait(NULL);
        }
    }
}

int check_pipe(char* command, char** piped_tokens)
{
    int i;
    for (i = 0; i < 2; i++) {
        piped_tokens[i] = strsep(&command, "|");
        if (piped_tokens[i] == NULL)
            break;
    }
  
    if (piped_tokens[1] == NULL){
        return 0;
    }
    else {
        return 1;
    }
}

void get_piped_args(char* command, char** tokens, char** piped_tokens) {
    char* piped_command[2];
    int status = 0;
    get_args(piped_command[0], tokens);
    get_args(piped_command[1], piped_tokens);
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
            for (int i = 0; i < commandcounter; i++) {
                printf("Command: %s\n", command_history[i]);
                printf("PID: %d\n", process_IDs[i]);
                printf("Time taken to execute: %.2f seconds\n\n", execution_times[i]);
            }
            break;
        }

        if (strcmp("cd", command) == 0) {
            command_history[commandcounter] = "cd";
            process_IDs[commandcounter] = getpid();
            chdir(tokens[1]);
            continue;
        }

        if (tokens[0] != NULL && strcmp("echo", tokens[0]) == 0) {
            command_history[commandcounter] = command;
            process_IDs[commandcounter] = getpid();
            commandcounter++;
            int j = 1;
            while (tokens[j] != NULL) {
                printf("%s ", tokens[j]);
                j++;
            }
            printf("\n");
            continue;
        }

        if (strcmp("history", command) == 0) {
            for (int printcounter = 0; printcounter < commandcounter; printcounter++){
                printf("Command: %s\n", command_history[printcounter]);
            }
        }

        int pipecheck = check_pipe(command, pipedtokens);
        
        if (pipecheck == 1) {
            get_piped_args(command, tokens, pipedtokens);
            clock_t start = clock();
            execute_piped_commands(tokens, pipedtokens);
            clock_t end = clock();
            double duration = (double)(end - start)/CLOCKS_PER_SEC;
            execution_times[commandcounter] = duration;
        }
        else {
            clock_t start = clock();
            execute_system_command(tokens);
            clock_t end = clock();
            double duration = (double)(end - start)/CLOCKS_PER_SEC;
            execution_times[commandcounter] = duration;
        }

    }
    return 0;
}
