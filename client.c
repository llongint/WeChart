#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include "my_thread_pool.h"
#include "my_socket.h"
#include "my_signal.h"


int main(int argc,char *argv[]){

    if(argc<3){
        printf("./%s [ip] [port]\n",argv[0]);
        exit(0);
    }


    return 0;
}
