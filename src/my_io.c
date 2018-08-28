#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <fcntl.h>
#include "my_io.h"
#include "rsa.h"

/* 工作目录 */
#define WORK_PATH "/home/weChart"
static const char const *public_key = "rsa_public"
static const char const *private_key = "rsa_public"
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
 * @brief   保存公钥或私钥
 * @format  
 *      前面两个字节用小端模式表示长度,后跟具体数字
 * @retval  成功返回0,失败返回-1
 */
int save_key(const char *filename,bignum e,bignum n){
    /* 不存在则创建文件 */
    int fd = open(filename,O_WRONLY | O_TRUNC | FILE_MODE);
    if(fd == -1){
        perror("open file");
    }
}
