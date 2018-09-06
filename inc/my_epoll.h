#ifndef __MY_EPOLL_H
#define __MY_EPOLL_H

#include <sys/epoll.h>

/* 最大消息长度 */
#define MAX_MESSAGE_SIZE 1024
#ifndef OPEN_MAX
#define OPEN_MAX 256
#endif


extern unsigned int current_connect;

int creatEpollServer(int *servfd,short port);
int epollAccept(int epfd,int servfd);
int epollRead(int epfd,struct epoll_event *events);



#endif