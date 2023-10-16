#include "headers.h"

volatile sig_atomic_t ctrlc = 0;
char* command_history[1000];
double execution_times[1000];
pid_t process_IDs[1000];
int commandcounter = 0;
int start_flag = 0;
int toggle = 1;
int ncpu;

typedef struct {
    int pid;           
    int state;         
    int priority;      
    char name[256];
    int execTime;
    int waitTime;
} Process;


typedef struct {
    Process PCB[MAX_PROs];
    sem_t semaphore;
    int numProcs;
} SharedMem;

int fd;
SharedMem* shared_mem;

void print_cwd(){
    char directory[1000];
    getcwd(directory, sizeof(directory));
    printf("%s", directory);
}

void ctrlc_handler(int signumber){
    if (signumber == SIGINT) {
        for (int i = 0; i < commandcounter; i++) {
        printf("Command: %s\n", command_history[i]);
        printf("PID: %d\n", process_IDs[i]);
        printf("Time taken to execute: %.2f seconds\n\n", execution_times[i]);
        }
    }
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
int tslice;
int cycles;
void SimpleScheduler(){
    cycles++;
    sem_wait(&shared_mem->semaphore);
    int check = shared_mem->numProcs;
    sem_post(&shared_mem->semaphore);
    
if(toggle == 1){
        sem_wait(&shared_mem->semaphore);
        Process initial = shared_mem->PCB[0];

        for (int i = 0; i < check - 1; i++) {
        shared_mem->PCB[i] = shared_mem->PCB[i + 1]; }
        shared_mem->PCB[check-1] = initial;
        sem_post(&shared_mem->semaphore);
        kill(initial.pid,SIGCONT);
        toggle = 0;
    }

if(check>1){
    for (int j = 0; j < ncpu; j++) {
    sem_wait(&shared_mem->semaphore);
    Process curr = shared_mem->PCB[0];

    for (int i = 0; i < check - 1; i++) {
        shared_mem->PCB[i] = shared_mem->PCB[i + 1];
    }
    shared_mem->PCB[check-1] = curr;
    Process next = shared_mem->PCB[0];
    sem_post(&shared_mem->semaphore);

    kill(curr.pid, SIGSTOP);
    curr.state = READY;
    kill(next.pid,SIGCONT);
    next.state = RUNNING;
    }
    }
}

void timeCalc(){
    sem_wait(&shared_mem->semaphore);
    int check = shared_mem->numProcs;
    sem_post(&shared_mem->semaphore);

    for(int i = 0; i<shared_mem->numProcs; i++){
        sem_wait(&shared_mem->semaphore);
        if (shared_mem->PCB[i].state == RUNNING) {
            shared_mem->PCB[i].execTime += tslice;
        }
        if (shared_mem->PCB[i].state == READY) {
        shared_mem->PCB[i].waitTime += tslice*(shared_mem->numProcs);
        }
        sem_post(&shared_mem->semaphore);
    }
}

void display(){
    for(int i = 0;i < shared_mem->numProcs ;i++){
        printf("File Path : %s\n",shared_mem->PCB[i].name);
        printf("Pid : %d\n",shared_mem->PCB[i].pid);
        printf("Pid : %d\n",shared_mem->PCB[i].priority);
    }
}

void timerHandler(int signum){
    if(start_flag){
        // printf("TIMER HERE\n");
        SimpleScheduler();
        timeCalc();
    }
    }

int compPrio(const void* p1, const void* p2) {
    return ((Process*)p1)->priority - ((Process*)p2)->priority;
}

int main(){
    // Shared Memory Initialization
    fd = shm_open("sharedMem", O_CREAT | O_RDWR, 0666);
    
    if (fd == -1) {
    perror("shm_open"); //Error Handling
    exit(1); 
}
    ftruncate(fd, sizeof(SharedMem));

//     if (ftruncate(fd, sizeof(SharedMem)) == -1) {
//     perror("ftruncate");
//     exit(1); 
// }

    void* shared_memory = mmap(NULL, sizeof(SharedMem), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    shared_mem = (SharedMem*)shared_memory;
    sem_init(&shared_mem->semaphore, 1, 1);

    sem_wait(&shared_mem->semaphore);
    shared_mem->numProcs = 0;
    sem_post(&shared_mem->semaphore);

    printf("Enter the number of CPUs: ");
    scanf("%d", &ncpu);
    printf("Enter the time slice (in ms): ");
    scanf("%d", &tslice);

    struct sigaction sig1;    
    sig1.sa_handler = timerHandler;
    sig1.sa_flags = SA_RESTART;
    int c1 = sigaction(SIGALRM, &sig1, NULL);
    
    if (c1 == -1) {
    perror("sigaction"); //Error Handling
    exit(1); 
}
        
    struct itimerval tSlicing;
    tSlicing.it_value.tv_sec = 0;           
    tSlicing.it_value.tv_usec = (tslice)*1000;       
    tSlicing.it_interval = tSlicing.it_value;
    setitimer(ITIMER_REAL, &tSlicing, NULL);    

    
    char command[MAX_COMMAND_LENGTH];
    char* tokens[MAX_ARGS];
    char* pipedtokens[MAX_ARGS];
    
    
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = ctrlc_handler;
    int c2 = sigaction(SIGINT, &sig, NULL);
    
    if (c2 == -1) {
    perror("sigaction"); //Error Handling
    exit(1); 
}

    print_cwd();
    printf("/");
    get_command(command);
    get_args(command, tokens);
    
    while (1) {
        print_cwd();
        printf("/");
        get_command(command);
        get_args(command, tokens);

        if (strcmp("exit", command) == 0) {
            for (int i = 0;i < shared_mem->numProcs;i++) {
                printf("Process: %s\n", shared_mem->PCB[i].name);
                printf("PID: %d\n", shared_mem->PCB[i].pid);
                printf("Time taken to execute: %.d milliseconds\n", shared_mem->PCB[i].execTime/(shared_mem->numProcs+1));
                printf("Wait time: %d milliseconds\n\n", shared_mem->PCB[i].waitTime);
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

        if (strcmp("start", tokens[0]) == 0) {
            qsort(shared_mem->PCB, shared_mem->numProcs, sizeof(Process), compPrio);
            start_flag = 1;
        }

        if (strcmp("display", tokens[0]) == 0) {
            qsort(shared_mem->PCB, shared_mem->numProcs, sizeof(Process), compPrio);
            display();
        }
    
    if (strcmp("submit", tokens[0]) == 0) {
        Process p1;
        p1.waitTime = 0;
        strcpy(p1.name, tokens[1]);
        if (tokens[2] != NULL) {
        p1.priority = atoi(tokens[2]);
        }
        else {
            p1.priority = 1;
        }
        p1.state = READY;
        if (tokens[3] != NULL && atoi(tokens[3]) == 1) {
            start_flag = 0;
        }
        int sc_pid = fork();
        if (sc_pid == 0) {
            execlp(tokens[1],tokens[1],NULL);
            perror("execlp");
            exit(1);
        } else if (sc_pid > 0) {
            p1.pid = sc_pid;
            sem_wait(&shared_mem->semaphore);
            shared_mem->PCB[shared_mem->numProcs] = p1;
            shared_mem->numProcs++;
            sem_post(&shared_mem->semaphore);
            kill(p1.pid,SIGSTOP);
        }
    }
    
}
    return 0;
}
