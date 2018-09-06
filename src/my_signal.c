
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#include "my_signal.h"

void sig_chld(int signo){
    pid_t pid;
    int stat;
    pid=wait(&stat);
    printf("child %d terminated\n",pid);
    return ;
}
