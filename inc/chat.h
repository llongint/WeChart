#ifndef __CHAT_H
#define __CHAT_H


#include "my_io.h"

int changeChat(struct User **g_userdata,char *buf);
struct User *findUserName(struct User *g_userdata,char *name);


#endif // !__CHAT_H

