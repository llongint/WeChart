#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <assert.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "login.h"
#include "my_io.h"
#include "format.h"
#include "my_epoll.h"
#include "my_thread_pool.h"

struct User_onLine user_online;
/**
 * @brief  验证登录信息
 * @note   hzq\n123\n\0
 * @param  *buf: 
 * @retval 
 */
int user_confirmation(int sockfd,char *buf)
{
    printf("start of user_confirmation\n");
    assert(g_userdata != NULL);
    char *p1 = NULL,*p2 = NULL;
    struct User *p = g_userdata;
    p1 = strstr((const char *)buf,(const char *)"\n");
    p2 = strstr((const char *)p1+1,  (const char *)"\n");
    if(p1==NULL || p2==NULL || p1-buf >=32 || p2-p1 >= 32){
        printf("login format error\n");
        return e_formatErr;
    }
    *p1 = '\0';
    *p2 = '\0';
    printf("buf = %s\n",buf);
    printf("p1 = %s\n",p1);
    while(p != NULL){
        if(strlen(p->m_name) == strlen(buf) &&
          strlen(p->m_passwd) ==strlen(p1+1)&& 
          memcmp(p->m_name,buf,strlen(buf)) == 0 && 
          memcmp(p->m_passwd,p1+1,strlen(p1+1)) == 0){
            int err = add_onlineUser(sockfd,p);          
            return err;
        }
        p=p->next;
    }
    printf("end of user_confirmation\n");
    return e_UserOrPasswdWrong;
}
/**
 * @brief  发送session
 * @note   
 * @param  sockfd: 
 * @param  *p: 
 * @retval 
 */
int send_session(int sockfd,char *p)
{
    printf("start of send_session\n");
    char send_cmd[MAX_MESSAGE_SIZE];
    bzero(send_cmd,MAX_MESSAGE_SIZE);

    head_package(send_cmd,e_msgSession,max_string_len);        //加上结束符
    memcpy((char *)send_cmd+Msg.m_msgLen,p,max_string_len);
    Write(sockfd, send_cmd, Msg.m_msgLen+max_string_len);      //发送结束符

    printf("end of send_session\n");
    return 0;
}

/**
 * @brief  添加在线用户
 * @note   
 * @param  *p: 
 * @retval 
 */
int add_onlineUser(int sockfd,struct User *p)
{
    printf("start of add_onlineUser\n");
    pthread_cleanup_push(handler, (void *)&user_online.lock);
	pthread_mutex_lock(&user_online.lock);
    int i=0;
    /* 1.从已经注册的用户中查找 */
    printf("user_online.online_count = %d\n",user_online.online_count);
    for(i = 0;i<user_online.online_count;i++){
        if(user_online.user[i] == p){
            printf("user is already inline\n");
            pthread_mutex_unlock(&user_online.lock);
            return e_userOnline;
        }
    }
    
    /* 2.添加到登录用户列表 */
    user_online.user[user_online.online_count++]=p;//记录在线用户信息节点

    /* 3.给该用户生成session */
    int err = create_rand_string(31,p->m_session);
    assert(err == 0);
    err = send_session(sockfd,p->m_session);
    assert(err == 0);

    /* 4.记录该用的IP和端口 */
    struct sockaddr_in sa;
    socklen_t len = sizeof(sa);   
    if(!getpeername(sockfd, (struct sockaddr *)&sa, &len)){
        strcpy(p->m_ip,inet_ntoa(sa.sin_addr));
        p->m_port = ntohs(sa.sin_port);
    }
    p->m_sockfd = sockfd;


    pthread_mutex_unlock(&user_online.lock);
	pthread_cleanup_pop(0);

    print_onlineUser();

    printf("login success,session has created\n"); 
    printf("end of add_onlineUser\n");
    return e_loginSuccess;
}
/**
 * @brief   从在线列表中删除某个用户
 * @note   
 * @param  *p: 
 * @retval 
 */
int del_onlineUser(struct User *p)
{
    printf("start of del_onlineUser\n");
    pthread_cleanup_push(handler, (void *)&user_online.lock);
	pthread_mutex_lock(&user_online.lock);
    int i=0;
    for(i = 0;i<user_online.online_count;i++){
        printf("i=%d,user_online.online_count=%d\n",i,user_online.online_count);
        if(user_online.user[i] == p){
            bzero(p->m_session,max_string_len);
            bzero(p->m_ip,max_string_len);
            p->m_port = -1;
            p->m_sockfd = -1;
            user_online.user[i]=user_online.user[--user_online.online_count];//记录在线用户信息节点
            
            pthread_mutex_unlock(&user_online.lock);
            printf("a user quit\n");
            return 0;
        }
    }
    pthread_mutex_unlock(&user_online.lock);
	pthread_cleanup_pop(0);
    printf("end of del_onlineUser\n");
    return 0;
}
/**
 * @brief 
 * @note   
 * @param  sockfd: 
 * @retval 
 */
struct User *findUserBysockfd(int sockfd)
{
    struct User *p = g_userdata;
    while(p!=NULL){
        if(p->m_sockfd == sockfd){
            return p;
        }
        p=p->next;
    }
    printf("not find such sockfd!");
    return NULL;
}
void print_onlineUser()
{
    printf("start of print_onlineUser\n");
    pthread_cleanup_push(handler, (void *)&user_online.lock);
	pthread_mutex_lock(&user_online.lock);
    printf("------------------------online user -----------------------------\n");
    struct User *p = NULL;
    int i=0;
    for(i = 0;i<user_online.online_count;i++){
        p = user_online.user[i];
        printf("user name:   %s\n",p->m_name);
        printf("user passwd: %s\n",p->m_passwd);
        printf("user ident..:%s\n",p->m_identification);
        printf("user sess..: %s\n",p->m_session);
        printf("user ip:     %s\n",p->m_ip);
        printf("user port:   %d\n",p->m_port);
        printf("user sockfd  %d\n",p->m_sockfd);
        printf("------------------------------------------\n");
    }
    printf("---------------------------------------end of the information-----\n");
    pthread_mutex_unlock(&user_online.lock);
	pthread_cleanup_pop(0);
    printf("end of print_onlineUser\n");
}