#ifndef __MY_EPOLL_H
#define __MY_EPOLL_H


extern unsigned int current_connect;

int epollAccept(int epfd,int servfd);
int epollClient(int epfd,struct epoll_event *events);



#endif