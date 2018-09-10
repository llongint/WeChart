#include <sys/types.h>
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
#include <math.h>
#include <sys/wait.h>

#include "chat.h"
#include "my_io.h"
#include "format.h"

/**
 * @brief  改变当前聊天的好友，覆盖到链表的头结点
 * @note   
 * @param  *buf: 
 * @retval 
 */
int changeChat(struct User **g_userdata,char *buf)
{
    assert(*g_userdata!=NULL);

    while(*buf==' ')buf++;

    char *p1=strstr((const char *)buf,(const char *)"\n");
    if(p1){
        *p1='\0';
    }
    struct User *p2=findUserName(*g_userdata,buf);
    if(p2){
        bzero((*g_userdata)->m_name,max_string_len);
        bzero((*g_userdata)->m_identification,max_string_len);
        print("p2->m_name = %s\n",p2->m_name);
        print("p2->m_identification = %s\n",p2->m_identification);
        memcpy((*g_userdata)->m_name,p2->m_name,strlen(p2->m_name));
        memcpy((*g_userdata)->m_identification,p2->m_identification,strlen(p2->m_identification));
    }else{
        return e_UserOrPasswdWrong;
    }
    print("end of %s\n",__FUNCTION__);
    return 0;
}
struct User *findUserName(struct User *g_userdata,char *name)
{
    while(g_userdata!=NULL){
        if(memcmp(g_userdata->m_name,name,strlen(name))==0){
            return g_userdata;
        }
        g_userdata = g_userdata->next;
    }
    print("can not find such %s,end of %s\n",name,__FUNCTION__);
    return NULL;
}
int sendChatMessage(int sock,char *cmd)
{
    char send_cmd[MAX_MESSAGE_SIZE];
    bzero(send_cmd,MAX_MESSAGE_SIZE);

    head_package(send_cmd,e_msgChart,strlen(cmd)+1);        //加上结束符
    snprintf((char *)send_cmd+Msg.m_msgLen,MAX_MESSAGE_SIZE-Msg.m_msgLen-1,"%s",cmd);
    write(sock, send_cmd, Msg.m_msgLen+strlen(cmd)+1);      //发送结束符

    print("end of %s\n",__FUNCTION__);
    return 0;
}