#ifndef __FORMAT_H
#define __FORMAT_H


/**
 * 协议总述：使用最常见的TLV格式数据
 *    T:2个字节，L:4字节，V:根据L而定
 * 类型字段 T:
 *    见MessageType
 * 注意： 
 *    数据的传输不包含'\0'
 */
struct MsgLen{
    unsigned int m_msgTypeLen;
    unsigned int m_msgLenLen;
    unsigned int m_msgValueLen;
}Msg;
enum MessageType{
    e_msgChart = 0x0001,    //聊天消息
    e_msgLs,                //Ls命令
    e_msgGet,               //文件请求命令
    e_msgFileHead,          //文件头信息
    e_msgFileContent,       //文件内容
    e_msgEndFile,           //文件传输完成
    e_msgSendFile,          //文件上传请求
    e_msgFileLegal,         //文件名合法，可以上传文件
    e_msgFileNonExist,      //文件不存在
    e_msgPwd,               //PWD命令
    e_msgCd                 //cd命令，改变工作路径
}messageType;





#endif