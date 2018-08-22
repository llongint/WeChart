#include "my_epoll.h"


int main(int argc,char *argv[]){

    int i=0;
    if(argc!=2){
        printf("%s <port>\n",argv[0]);
        exit(0);
    }
    /* 1.创建套接字 */
    int servfd=socket(AF_INET,SOCK_STREAM,0);
    if(servfd == -1){
        perror("socket");
        exit(0);
    }
    
    /* 2.设置套接字地址 */
    struct sockaddr_in servAddr,clientAddr;
    socklen_t addrlen = sizeof(servAddr);
    bzero(&servAddr,addrlen);
    bzero(&clientAddr,addrlen);
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(atoi(argv[1]));

    /* 3.bind地址 */
    int err = bind(servfd,(struct sockaddr *)&servAddr,addrlen);
    if(err == -1){
        perror("bind");
    }

    /* 4/listen,服务器变为LISTEN状态 */
    err = listen(servfd,LISTENQ);
    if(err == -1){
        perror("listen");
    }
    
    /* 5.创建epfd描述符 */
    int epfd = epoll_create(1);
    
    /* 6.添加事件 */
    struct epoll_event ev;
    bzero(&ev,sizeof(ev));
    ev.data.fd = servfd;
    ev.events = EPOLLIN | EPOLLET;//可读事件(新的连接),边沿触发
    
    err = epoll_ctl(epfd,EPOLL_CTL_ADD,servfd,&ev);
    if(err == -1){
        perror("epoll_ctrl");
        exit(0);
    }
    
    /* 7.创建事件 */
    struct epoll_event events[OPEN_MAX];

    /* 8.等待事件 */
    while(1){
        int nfd = epoll_wait(epfd,events,current_connect+1,-1);
        //printf("nfd = %d\n",nfd);
        for(i=0;i<nfd;i++){
            /* 如果是servfd可读，即有新的连接，服务器处于SYN_RCVD状态 */
            //printf("now is event[%d]\n",i);
            if(events[i].data.fd == servfd){
                /* 接受连接，服务器状态变为：ESTABLISHED */
                epollAccept(epfd,servfd,&clientAddr,&addrlen);
        
            }else if(events[i].events&EPOLLIN){//如果是可读事件
                epollClient(epfd,&events[i]);
            }else{
                printf("fd = %d\n",events[i].data.fd);
                printf("没有理由运行到这里啊！\n");
            }
        }
    }
    
    return 0;
}