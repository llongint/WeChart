#ifndef __MY_SOCKET_H
#define __MY_SOCKET_H



//服务器端ip
extern const char const *servIp;


int connect_serv(const char const *ip,short port);
int delClientInput(int sock,char *cmd);
int delServerRecv(int sockfd,unsigned short cmd,unsigned int packet_len,char *buf);
int delClientRecv(unsigned short cmd,unsigned int packet_len,char *buf);
int redClientRecv(int sockfd);
void sendRegisterCmd(int sock, char *cmd);
void sendLoginCmd(int sock, char *cmd);
void sendLogoutCmd(int sock, char *cmd);
void freeback2client(int sockfd,signed int err);
int save_session(char *buf);
void sendAddFriend(int sock, char *cmd);

#endif