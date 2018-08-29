/*
 * @Author: hzq 
 * @Date: 2018-08-29 15:56:14 
 * @Last Modified by: hzq
 * @Last Modified time: 2018-08-29 20:18:06
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "my_io.h"
#include "format.h"
#include "rsa.h"

/* 工作目录 */
#define WORK_PATH "/home/weChart"
const char const *public_key = "rsa.public";
const char const *private_key = "rsa.private";
/**
 * @brief 改变工作路径
 * 
 * @retval 
 *      成功返回0,失败返回-1
 */
int file_init(void){
    
    int err = mkdir(WORK_PATH,DIR_MODE);
    if(err == -1){
        if(errno == EEXIST){
            ;//已经存在就不用做什么
        }
        //如果路径不存在,就是说没有/home目录
        else if(errno == ENOENT){
            mkdir("/home/",DIR_MODE);
            mkdir(WORK_PATH,DIR_MODE);
        }else{
            perror("mkdir");
            return -1;
        }
    }
    err = chdir(WORK_PATH);
    if(err == -1){
        perror("chdir");
    }
    int fd1 = open(public_key,O_WRONLY | O_EXCL | O_CREAT | FILE_MODE);
    int fd2 = open(private_key,O_WRONLY | O_EXCL | O_CREAT | FILE_MODE);
    if(fd1 == -1 && fd2 == -1){
        if(errno == EEXIST){    //如果两个文件都已经存在
            ;                   //说明秘钥文件已经生成
        }else{                  //如果碰到了其他问题
            perror("open");     //打印提示信息
            return -1;          //并退出
        }
    }else{                      //如果缺少秘钥文件
        err = create_key();     //则重新创建
    }

    return err;
}

/** 
 * @brief  保存公钥或私钥
 * @note   前面四个字节用小端模式表示长度,后跟具体数字
 * @param  *filename: 
 * @param  *e: 
 * @param  *n: 
 * @retval 成功返回0,失败返回-1
 */
int save_key(const char *filename,bignum *e,bignum *n)
{    
    int i=0;
    char buf[sizeof(unsigned int)];
    /* 不存在则创建文件 */
    int fd = open(filename,O_WRONLY | O_TRUNC);
    if(fd == -1){
        perror("open file");
        return -1;
    }

    uintToString(buf,(unsigned int)(e->length));
    Write(fd,buf,sizeof(unsigned int));
    uintToString(buf,(unsigned int)(n->length));
    Write(fd,buf,sizeof(unsigned int));

    for(i=0;i<e->length;i++){
        uintToString(buf,e->data[i]);
        Write(fd,buf,sizeof(unsigned int));
    }
    for(i=0;i<n->length;i++){
        uintToString(buf,n->data[i]);
        Write(fd,buf,sizeof(unsigned int));
    }
    close(fd);
    return 0;
}

/** 
 * @brief  读取密钥文件
 * @note   
 * @param  *filename: 密钥文件名
 * @param  *e: 
 * @param  *n: 
 * @retval 
 */
int read_key(const char *filename,bignum *e,bignum *n)
{
    int i = 0;
    char str[sizeof(unsigned int)];
    /* 不存在则创建文件 */
    int fd = open(filename,O_RDONLY);
    if(fd == -1){
        perror("open file");
        return -1;
    }
    Read(fd,str,sizeof(unsigned int));
    stringToUint(str,&(e->length));
    Read(fd,str,sizeof(unsigned int));
    stringToUint(str,&(n->length));
    
    for(i=0;i<e->length;i++){
        Read(fd,str,sizeof(unsigned int));
        stringToUint(str,&(e->data[i]));
    }
    for(i=0;i<e->length;i++){
        Read(fd,str,sizeof(unsigned int));
        stringToUint(str,&(n->data[i]));
    }
    close(fd);
    return 0;
}
void Write(int fd, void *ptr, size_t nbytes){
	int r=write(fd, ptr, nbytes);
    if ( r!= nbytes){
		printf("write error: %d %d\n",r,(int)nbytes);
        exit(-1);
    }
}
ssize_t Read(int fd, void *ptr, size_t nbytes)
{
	ssize_t		n;

	if ( (n = read(fd, ptr, nbytes)) == -1){
		perror("read error");
    }
	return(n);
}