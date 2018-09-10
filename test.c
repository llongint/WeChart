/*************************************************************************
    > File Name: test.c
    > Author: hzq
    > Mail: 1593409937@qq.com 
    > Created Time: Sat 25 Aug 2018 11:06:28 AM CST
    > function:
 ************************************************************************/
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include "my_socket.h"

int main()
{
    char str[100]={0};
    // create_rand_num(20,str);
    // str[20]=0;
    // printf("str = %s\n",str);
    sendAddFriend(fileno(stdout),str);
    return 0;
}


//zc 88652751513