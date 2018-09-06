/*
 * @Author: hzq 
 * @Date: 2018-08-27 21:23:40 
 * @Last Modified by: hzq
 * @Last Modified time: 2018-08-29 16:29:53
 */
#include <limits.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "login.h"
#include "my_epoll.h"
#include "my_io.h"
#include "my_thread_pool.h"

static thread_pool *s_pool=NULL;

/** 
 * @brief  
 * @note   
 * @param  argc: 
 * @param  *argv[]: 
 * @retval 
 */
int main(int argc,char *argv[]){

    int i=0;
    if(argc!=2){
        printf("%s <port>\n",argv[0]);
        exit(0);
    }
    if( file_init() == -1){
        return -1;
    }
    s_pool = malloc(sizeof(thread_pool));
	init_pool(s_pool,OPEN_MAX);
    int servfd;

    /* 1.创建服务 */
    int epfd = creatEpollServer(&servfd,atoi(argv[1]));
    assert("epfd != -1");

    /* 2.创建事件 */
    struct epoll_event events[OPEN_MAX],ev;

    /* 3.等待事件 */
    while(1){
        int nfd = epoll_wait(epfd,events,current_connect+1,-1);
        for(i=0;i<nfd;i++){
            /* 如果是servfd可读，即有新的连接，服务器处于SYN_RCVD状态 */
            if(events[i].data.fd == servfd){
                /* 接受连接，服务器状态变为：ESTABLISHED */
                epollAccept(epfd,servfd);
            }else if(events[i].events&EPOLLIN){     //如果是可读事件
                add_task(s_pool,epollRead,epfd,&events[i]);
            }else if(events[i].events&EPOLLRDHUP){  //如果对端关闭连接
                printf("a user closed\n");
                epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&ev);
                del_onlineUser(findUserBysockfd(events[i].data.fd));
                close(events[i].data.fd);
                current_connect--;
                events->data.fd = -1;
            }else{
                printf("fd = %d\n",events[i].data.fd);
                printf("没有理由运行到这里！\n");
            }
        }
    }
    destroy_pool(s_pool);
    return 0;
}