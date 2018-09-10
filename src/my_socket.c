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
#include "chat.h"

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
    if(strncmp(cmd, "regist", strlen("regist")) == 0){
		sendRegisterCmd(sock, cmd);
	}else if(strncmp(cmd, "login", strlen("login")) == 0){
		sendLoginCmd(sock, cmd);
	}else if(strncmp(cmd, "logout", strlen("logout")) == 0){
		sendLogoutCmd(sock, cmd);
	}else if(strncmp(cmd, "list", strlen("list")) == 0){
		print_userData(g_cliUserdata);
	}else if(strncmp(cmd, "add f", strlen("add f")) == 0){
		sendAddFriend(sock,cmd);
	}else if(strncmp(cmd, "talk2", strlen("talk2")) == 0){
		changeChat(&g_cliUserdata,cmd+strlen("talk2"));
	}else if(strncmp(cmd, "--->", strlen("--->")) == 0){
		sendChatMessage(sock,cmd+strlen("--->"));
	}else if(strncmp(cmd, "pwd", strlen("pwd")) == 0){
		system("pwd");
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
        print("count = %d,packet_len = %d\n",count,packet_len);
        printf("read failed!\n");
        exit(0);
        return -1;
    }
    int err = delClientRecv(cmd_num,packet_len,buf);
    
    return err;
}
/**
 * @brief  处理客户端接收的数据
 * @note   
 * @param  cmd: 
 * @param  packet_len: 
 * @param  *buf: 
 * @retval 
 */
int delClientRecv(unsigned short cmd,unsigned int packet_len,char *buf)
{
    signed int err = 0;
    printf("cmd num = %d\n",cmd);
    switch (cmd){
        case e_msgDebug:
            printf("recv: %s",buf);
            break;
        case e_msgSession:
            err = save_session(buf);
            assert(err == 0);
            print("login success,session has saved");
            break;
        case e_msgAddFriend:
            err = add_friend(cli_userDataFile,&g_cliUserdata,buf);
            break;
        default:
            printf("unknow cmd\n");
            return -1;
    }
    return 0;
}
/**
 * @brief  解析字符串: hzq\n88652751513\n[˘'e종*\n
 * @note   
 * @param  *buf: 
 * @retval 
 */
int save_session(char *buf)
{
    print("begin of %s\n",__FUNCTION__);
  
    char *p1 = NULL,*p2 = NULL;
    p1 = strstr((const char *)buf,(const char *)"\n");
    p2 = strstr((const char *)p1+1,  (const char *)"\n");
    
    if(p1==NULL || p2==NULL || p1-buf >=32 || p2-p1 >= 32||p2-p1<=1){
        printf("format error\n");
        return e_formatErr;
    }
    assert(g_cliUserdata!=NULL);
    memcpy(g_cliUserdata->m_session,p2+1,max_string_len);


    *p2='\0';
    char path[MAX_MESSAGE_SIZE];
    snprintf(path,MAX_MESSAGE_SIZE,"%s/%s",g_work_path,p1+1);
    print("path: %s\n",path);
    file_init(path,cli_public_key,cli_private_key,&g_cliUserdata,cli_userDataFile);


    *p2='\n';
    add_friend(cli_userDataFile,&g_cliUserdata,buf);
    
    print("end of %s\n",__FUNCTION__);
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
void sendAddFriend(int sock, char *cmd)
{
    char send_cmd[MAX_MESSAGE_SIZE];
    bzero(send_cmd,MAX_MESSAGE_SIZE);
    
    printf("friend's name: ");
    fgets(cmd,max_string_len,stdin);
    printf("friend's QQ: ");
    fgets(cmd+strlen(cmd),max_string_len,stdin);

    printf("waiting server return for add friend:\n");
    head_package(send_cmd,e_msgAddFriend,strlen(cmd)+1);        //加上结束符
    snprintf((char *)send_cmd+Msg.m_msgLen,MAX_MESSAGE_SIZE-Msg.m_msgLen-1,"%s",cmd);
    write(sock, send_cmd, Msg.m_msgLen+strlen(cmd)+1);      //发送结束符
    print("send: %s\n",send_cmd+6);
    print("line:%d,end of the %s\n",__LINE__,__FUNCTION__);
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
            err = save_userData(serv_userDataFile,&g_servUserdata,buf);
            freeback2client(sockfd,err);
            break;
        case e_msgLogin:
            //登录
            printf("login request\n");
            err = user_confirmation(sockfd,&g_servUserdata,buf);    //ref g_userdata
            printf("err = %d\n",err);
            freeback2client(sockfd,err);
            break;
        case e_msgLogout:
            //注销
            printf("logout request\n");
            err = user_logout(sockfd,&g_servUserdata,buf);    //ref g_userdata
            printf("err = %d\n",err);
            freeback2client(sockfd,err);
            break;
        case e_msgAddFriend:
            //添加好友
            printf("add friend request\n");
            err = add_friend_request(sockfd,&g_servUserdata,buf);    //ref g_userdata
            printf("err = %d\n",err);
            freeback2client(sockfd,err);
            break;
        case e_msgChart:
            print("char message:%s\n",buf+max_string_len);

            break;
        default:
            printf("unknow cmd\n");
            return -1;
    }
    return 0;
}
/**
 * @brief  给客户端发送反馈信息
 * @note   
 * @param  sockfd: 连接套接字
 * @param  err: 错误吗
 * @retval None
 */
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
    head_package(send_cmd,e_msgDebug,strlen(cmd)+1);    //加上结束符
    snprintf((char *)send_cmd+Msg.m_msgLen,MAX_MESSAGE_SIZE-Msg.m_msgLen-1,"%s",cmd);
    write(sockfd, send_cmd, Msg.m_msgLen+strlen(cmd)+1);//发送结束符 
}
