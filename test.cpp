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

#pragma pack(8)
class ClassA
{
//public:
//	int function1(){
     //   return 1+1;
   // }      //      1
	//virtual void function2();   //      8
	// void function3();           //      8
	//virtual void function4();   //      8
    //virtual void function5();   //      8
	short a;     //  16
	int b;      //  16
	char c;     //  24
	long d;     //  32
};
  /*return 1 : little-endian, return 0:big-endian*/
int main(){

    printf("sizeof(class ClassA) = %d\n",sizeof(class ClassA));
    
    //printf("sizeof(char *) = %d\n",sizeof(char *));
    return 0;
}


