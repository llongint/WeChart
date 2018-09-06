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

#include "my_socket.h"
#include "my_io.h"
#include "format.h"
#include "my_epoll.h"
#include "login.h"

//服务器端ip
const char const *servIp = "139.199.172.253";

/** 
 * @brief  与服务器建立TCP连接
 * @note   
 * @param  *ip: 
 * @param  port: 
 * @retval 成功返回套接字描述符.失败返回-1
 */
int connect_serv(const char const *ip,short port)
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    assert(sockfd != -1);

    struct sockaddr_in clientAddr;
    socklen_t addrlen = sizeof(clientAddr);
    bzero(&clientAddr,addrlen);
    clientAddr.sin_family=AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = 0;
    int err = bind(sockfd,(struct sockaddr*)&clientAddr,addrlen);//不绑定也行的
    assert(err != -1);

    clientAddr.sin_addr.s_addr = inet_addr(ip);
    clientAddr.sin_port = htons(port);
    err = connect(sockfd,(struct sockaddr*)&clientAddr,addrlen);
    assert(err != -1);

    return sockfd;
}
/** 
 * @brief  处理客户端标准输入
 * @note   
 * @param  sock: 套接字描述符
 * @param  *cmd: 命令
 * @retval 成功返回0,失败返回一个负数
 */
int delClientInput(int sock,char *cmd)
{
    if (strncmp(cmd, "regist", strlen("regist")) == 0){
		sendRegisterCmd(sock, cmd);
	}
    if (strncmp(cmd, "login", strlen("login")) == 0){
		sendLoginCmd(sock, cmd);
	}if (strncmp(cmd, "logout", strlen("logout")) == 0){
		sendLogoutCmd(sock, cmd);
	}else{
        //没有检查到关键词
        char send_cmd[MAX_MESSAGE_SIZE];
        head_package(send_cmd,e_msgDebug,strlen(cmd)+1);        //加上结束符
        snprintf((char *)send_cmd+Msg.m_msgLen,MAX_MESSAGE_SIZE-Msg.m_msgLen-1,"%s",cmd);
        write(sock, send_cmd, Msg.m_msgLen+strlen(cmd)+1);//发送结束符
    }
    return 0;
}
/** 
 * @brief  处理客户端接收的套接字数据
 * @note   
 * @param  sock: 
 * @retval 成功返回0,失败返回一个负数
 */
int redClientRecv(int sockfd)
{
    assert(sockfd != -1);

    char buf[MAX_MESSAGE_SIZE];
    int count=0;
    bzero(buf,MAX_MESSAGE_SIZE);

    /* 1.读取出错 */
    if ( (count = read(sockfd, buf, Msg.m_msgLen)) < 0) {
        close(sockfd);
        perror("read sockfd");
        exit(0);
        return -2;
    }

    /* 2.服务器发送FIN */
    else if (count == 0){
        printf("server closed!\n");
        exit(0);
        return -3;
    }
    /* 3.没有读到6个字节 */
    else if(count < 6){
        exit(0);
        return -1;
    }

    /* 4.正常读数据 */
    unsigned short cmd_num = 0;
    unsigned int packet_len = 0;
    head_analyze(buf,&cmd_num,&packet_len);
    count = Read(sockfd, buf, packet_len);
    if(count < packet_len){
        printf("read failed!\n");
        exit(0);
        return -1;
    }
    int err = delClientRecv(cmd_num,packet_len,buf);
    
    return err;
}
int delClientRecv(unsigned short cmd,unsigned int packet_len,char *buf)
{
    //signed int err = 0;
    printf("cmd num = %d\n",cmd);
    switch (cmd){
        case e_msgDebug:
            printf("recv: %s",buf);
            break;
        default:
            printf("unknow cmd\n");
            return -1;
    }
    return 0;
}
/** 
 * @brief  
 * @note   
 * @param  sock: 
 * @param  *cmd:    用户名+密码+验证码(暂时先用固定的)，换行符分隔，最长32个字符
 *      @example    hzq\n!$@*%&\n142857\n\0
 * @retval None
 */
void sendRegisterCmd(int sock, char *cmd)
{
    char send_cmd[MAX_MESSAGE_SIZE];
    bzero(send_cmd,MAX_MESSAGE_SIZE);
    
    printf("user name: ");
    fgets(cmd,max_string_len,stdin);
    printf("paswd: ");
    fgets(cmd+strlen(cmd),max_string_len,stdin);
    printf("identifying code: ");
    fgets(cmd+strlen(cmd),max_string_len,stdin);
    
    head_package(send_cmd,e_msgRegist,strlen(cmd)+1);        //加上结束符
    snprintf((char *)send_cmd+Msg.m_msgLen,MAX_MESSAGE_SIZE-Msg.m_msgLen-1,"%s",cmd);
    write(sock, send_cmd, Msg.m_msgLen+strlen(cmd)+1);//发送结束符
}
/**
 * @brief  发送登录命令
 * @note   举例：       hzq\n123\n\0
 * @param  sock: 
 * @param  *cmd: 
 * @retval None
 */
void sendLoginCmd(int sock, char *cmd)
{
    char send_cmd[MAX_MESSAGE_SIZE];
    bzero(send_cmd,MAX_MESSAGE_SIZE);
    
    printf("user name: ");
    fgets(cmd,max_string_len,stdin);
    printf("paswd: ");
    fgets(cmd+strlen(cmd),max_string_len,stdin);

    printf("waiting server return for Login:\n");
    head_package(send_cmd,e_msgLogin,strlen(cmd)+1);        //加上结束符
    snprintf((char *)send_cmd+Msg.m_msgLen,MAX_MESSAGE_SIZE-Msg.m_msgLen-1,"%s",cmd);
    write(sock, send_cmd, Msg.m_msgLen+strlen(cmd)+1);      //发送结束符
}
/**
 * @brief  注销账号
 * @note   
 * @param  sock: 
 * @param  *cmd: 
 * @retval None
 */
void sendLogoutCmd(int sock, char *cmd)
{
    char send_cmd[MAX_MESSAGE_SIZE];
    bzero(send_cmd,MAX_MESSAGE_SIZE);
    
    printf("user name: ");
    fgets(cmd,max_string_len,stdin);
    printf("paswd: ");
    fgets(cmd+strlen(cmd),max_string_len,stdin);

    printf("waiting server return for Logout:\n");
    head_package(send_cmd,e_msgLogout,strlen(cmd)+1);        //加上结束符
    snprintf((char *)send_cmd+Msg.m_msgLen,MAX_MESSAGE_SIZE-Msg.m_msgLen-1,"%s",cmd);
    write(sock, send_cmd, Msg.m_msgLen+strlen(cmd)+1);      //发送结束符
}
/** 
 * @brief  处理服务端接收的套接字数据
 * @note   
 * @param  cmd: 
 * @param  packet_len: 
 * @param  *buf: 
 * @retval 
 */
int delServerRecv(int sockfd,unsigned short cmd,unsigned int packet_len,char *buf)
{
    signed int err = 0;
    printf("cmd num = %d\n",cmd);
    switch (cmd){
        case e_msgDebug:
            printf("recv: %s",buf);
            break;
        case e_msgRegist:
            //注册
            printf("regist request\n");
            err = save_userData(userDataFile,buf);
            freeback2client(sockfd,err);
            break;
        case e_msgLogin:
            //登录
            printf("login request\n");
            err = user_confirmation(sockfd,buf);    //ref g_userdata
            printf("err = %d\n",err);
            freeback2client(sockfd,err);
            break;
        case e_msgLogout:
            //注销
            printf("logout request\n");
            err = user_logout(sockfd,buf);    //ref g_userdata
            printf("err = %d\n",err);
            freeback2client(sockfd,err);
            break;
        default:
            printf("unknow cmd\n");
            return -1;
    }
    return 0;
}
void freeback2client(int sockfd,signed int err)
{
    char send_cmd[MAX_MESSAGE_SIZE];
    char cmd[MAX_MESSAGE_SIZE];
    switch(err){
        case e_success:
            snprintf(cmd,MAX_MESSAGE_SIZE,"%s","regist success,now you can login in\n");
            break;
        case e_userExist:
            snprintf(cmd,MAX_MESSAGE_SIZE,"%s","the user name has already exist!\n");
            break;
        case e_wongIdent:
            snprintf(cmd,MAX_MESSAGE_SIZE,"%s","the identification is wrong\n");
            break;
        case e_formatErr:
            snprintf(cmd,MAX_MESSAGE_SIZE,"%s","format error\n");
            break;
        case e_UserOrPasswdWrong:
            snprintf(cmd,MAX_MESSAGE_SIZE,"%s","user name or password is wrong\n");
            break;
        case e_loginSuccess:
            snprintf(cmd,MAX_MESSAGE_SIZE,"%s","login success\n");
            break;
        case e_userOnline:
            snprintf(cmd,MAX_MESSAGE_SIZE,"%s","user is online,you can't login again\n");
            break;
        case e_LogoutSuccess:
            snprintf(cmd,MAX_MESSAGE_SIZE,"%s","Logout success\n");
            break;
        default :
            return ;//其他错误就啥也不输出
    }
    head_package(send_cmd,e_msgDebug,strlen(cmd)+1);        //加上结束符
    snprintf((char *)send_cmd+Msg.m_msgLen,MAX_MESSAGE_SIZE-Msg.m_msgLen-1,"%s",cmd);
    write(sockfd, send_cmd, Msg.m_msgLen+strlen(cmd)+1);//发送结束符 
}
