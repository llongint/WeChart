#ifndef __LOGIN_H
#define __LOGIN_H

#include <pthread.h>

//最大在线用户
#define MAX_ONLINE 1024
#pragma pack(4)

struct User_onLine  //线程池头结点
{
    unsigned int online_count;      //@ref file_init(...)
	pthread_mutex_t lock;           //互斥锁，用来保护这个这个结构
	struct User *user[MAX_ONLINE];  //指针数组
};

extern struct User_onLine user_online;

int user_confirmation(int sockfd,char *buf);
int send_session(int sockfd,char *p);
int add_onlineUser(int sockfd,struct User *p);
int del_onlineUser(struct User *p);
void print_onlineUser();
struct User *findUserBysockfd(int sockfd);








#endif // !__LOGIN_H
