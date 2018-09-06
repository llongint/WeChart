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
#include <sys/time.h>


#include "my_thread_pool.h"
#include "my_socket.h"
#include "my_signal.h"
#include "my_epoll.h"
#include "rsa.h"

int main(int argc,char *argv[]){

    if(argc<2){
        printf("./%s [ip] [port]\n",argv[0]);
        exit(0);
    }
    int sockfd = connect_serv(servIp,atoi(argv[1]));
    assert(sockfd != -1);

    char buf[MAX_MESSAGE_SIZE];
    int maxfdp1;
    fd_set rset;
    FD_ZERO(&rset);
    setbuf(stdout,NULL);
    while(1){
        printf("\r$ ");
        bzero(buf,MAX_MESSAGE_SIZE);
        FD_SET(fileno(stdin),&rset);
        FD_SET(sockfd,&rset);

        maxfdp1=MAX(fileno(stdin),sockfd)+1;
        select(maxfdp1,&rset,NULL,NULL,NULL);

        if(FD_ISSET(fileno(stdin),&rset)){
            fgets(buf,MAX_MESSAGE_SIZE,(stdin));
            //printf("get command: %s\n",buf);
            delClientInput(sockfd,buf);
        }

        if(FD_ISSET(sockfd,&rset)){
            redClientRecv(sockfd);
        }
    }

    return 0;
}
