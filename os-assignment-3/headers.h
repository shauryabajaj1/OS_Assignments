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
#include <sys/signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/time.h>



#define MAX_PROs 100
#define MAX_COMMAND_LENGTH 1000
#define MAX_ARGS 100
#define READY 0
#define RUNNING 1
