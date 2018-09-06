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
#include <assert.h>

#include "my_epoll.h"
#include "my_io.h"
#include "format.h"
#include "login.h"
#include "my_socket.h"

unsigned int current_connect=0;
static const int s_listenEq = 1024;//最大等待连接队列
/**
 * @brief   接收一个新的连接,加入epfd监听的队列
 * @param
 *      epfd    需要加入的epoll描述符
 *      serfd   监听描述符
 * @retval
 *      成功返回客户端描述符,失败返回-1
 */
int epollAccept(int epfd,int servfd){

    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    bzero(&clientAddr,addrlen);
    struct epoll_event ev;

    int clientfd = accept(servfd,(struct sockaddr *)&clientAddr,&addrlen);
    if(clientfd == -1){
        perror("accept");
        return -1;
    }else{
        char *str = inet_ntoa(clientAddr.sin_addr);
        printf("new connection:%s,port = %d,fd = %d!\n",str,ntohs(clientAddr.sin_port),clientfd);
        current_connect+=1;
        
        ev.data.fd = clientfd;
        ev.events = EPOLLRDHUP |EPOLLIN | EPOLLET;
        epoll_ctl(epfd,EPOLL_CTL_ADD,clientfd,&ev);
    }
    return clientfd;
}
/**
 * @brief 创建套接字并加入epoll
 * @param 
 *      servfd  作为结果参数，返回接听套接字描述符
 *      port    需要监听的端口号
 * @retval 
 *      成功返回 epoll描述符
 *      失败返回-1
 */
int creatEpollServer(int *servfd,short port){
    assert(servfd!=NULL);
    /* 1.创建套接字 */
    *servfd=socket(AF_INET,SOCK_STREAM,0);
    if(*servfd == -1){
        perror("socket");
        return -1;
    }
    /* 2.设置服务端地址 */
    struct sockaddr_in servAddr;
    socklen_t addrlen = sizeof(servAddr);
    bzero(&servAddr,addrlen);
    //bzero(&clientAddr,addrlen);
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);

    /* 3.bind地址 */
    int err = bind(*servfd,(struct sockaddr *)&servAddr,addrlen);
    if(err == -1){
        perror("bind");
        return -1;
    }

    /* 4.listen,服务器变为LISTEN状态 */
    err = listen(*servfd,s_listenEq);
    if(err == -1){
        perror("listen");
        return -1;
    }
    /* 5.创建epfd描述符 */
    int epfd = epoll_create(1);
    if(epfd == -1){
        perror("epoll_create");
        return -1;
    }
    /* 6.添加事件 */
    struct epoll_event ev;
    bzero(&ev,sizeof(ev));
    ev.data.fd = *servfd;
    ev.events = EPOLLIN | EPOLLET;//可读事件(新的连接),边沿触发

    err = epoll_ctl(epfd,EPOLL_CTL_ADD,*servfd,&ev);
    if(err == -1){
        perror("epoll_ctrl");
        return -1;
    }
    return epfd;
}
/**
 * @function:处理用户的数据
 * @param
 *      epfd    epoll描述符
 *      events  描述事件结构体
 * @retval  
 *      成功返回0
 *      失败返回一个负数
 */
int epollRead(int epfd,struct epoll_event *events){
    
    struct epoll_event ev;
    char buf[MAX_MESSAGE_SIZE];
    int count=0;
    int sockfd = -1;
    
    if( (sockfd = events->data.fd) < 0)
        return -1;
    
    bzero(buf,MAX_MESSAGE_SIZE);
    
    /* 1.对端发送FIN后，还向这个套接字写会受到RST */
    if ( (count = read(sockfd, buf, Msg.m_msgLen)) < 0) {
        if (errno == ECONNRESET){
            epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,&ev);
            del_onlineUser(findUserBysockfd(sockfd));
            close(sockfd);
            current_connect--;
            events->data.fd = -1;
        }
        perror("read sockfd");
        return -2;
    }
    /* 2.对方发送了FIN,服务器读会返回0，应答后处于CLOSE_WAIT状态 */
    else if (count == 0){
        printf("a user closed\n");
        epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,&ev);
        del_onlineUser(findUserBysockfd(sockfd));
        close(sockfd);
        current_connect--;
        events->data.fd = -1;
        return -3;
    }
    /* 3.没有读到6个字节 */
    else if(count < 6){
        return -1;
    }

    /* 4.正常读数据 */
    unsigned short cmd_num = 0;
    unsigned int packet_len = 0;
    head_analyze(buf,&cmd_num,&packet_len);
    count = Read(events->data.fd, buf, packet_len);
    if(count < packet_len){
        printf("read failed!\n");
        return -1;
    }
    delServerRecv(events->data.fd,cmd_num,packet_len,buf);
    
    return 0;
}












