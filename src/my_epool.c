#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <strings.h>
#include <errno.h>
#include <error.h>
#include <poll.h>

unsigned int current_connect=0;

/**
 * @function:接收一个新的连接,加入epfd监听的队列
 *
 */
int epollAccept(int epfd,int servfd){
    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    struct epoll_event ev;
    int clientfd = accept(servfd,(struct sockaddr *)&clientAddr,&addrlen);
    if(clientfd == -1){
        perror("accept");
    }else{
        char *str = inet_ntoa(clientAddr->sin_addr);
        printf("got a new connection:%s ,fd = %d!\n",str,clientfd);
        current_connect+=1;
        
        ev.data.fd = clientfd;
        ev.events = EPOLLIN | EPOLLET;
        epoll_ctl(epfd,EPOLL_CTL_ADD,clientfd,&ev);
    }
    return clientfd;
}

/**
 *@function:处理用户的数据
 *
 */
int epollClient(int epfd,struct epoll_event *events){
    struct epoll_event ev;
    char buf[MAX_MESSAGE_SIZE];
    int count=0;
    int sockfd = -1;
    
    if( (sockfd = events->data.fd) < 0)
        return -1;
    
    bzero(buf,MAX_MESSAGE_SIZE);
    if ( (count = read(sockfd, buf, MAX_MESSAGE_SIZE)) < 0) {
    /* 1.对端发送FIN后，还向这个套接字写会受到RST */
        if (errno == ECONNRESET) {
            epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,&ev);
            close(sockfd);
            current_connect--;
            events->data.fd = -1;
        }
        perror("read sockfd");
        return -2;
    }
    /* 2.对方发送了FIN,服务器读会返回0，应答后处于CLOSE_WAIT状态 */
    else if (count == 0){
        epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,&ev);
        close(sockfd);
        current_connect--;
        events->data.fd = -1;
        return -3;
    }
    /* 3.正常读数据 */
    buf[count]=0;           //添加结束符
    printf("recv :%s",buf); //打印接收的数据
    
    return 0;
}











