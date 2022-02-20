#ifndef _CELL_ClIENT_HPP_
#define _CELL_ClIENT_HPP_


#include "ShareLib.hpp"

// 客户端心跳检测时间间隔
#define CLIENT_HEART_DEAD_TIME 5000


// 客户端数据类型
class CellClient : public ObjectPoolBase<CellClient,6666>
{

public:
    CellClient(SOCKET sockfd = INVALID_SOCKET){
        _sockfd = sockfd;

        memset(_szMsgbuf,0,sizeof(RECV_BUFF_SIZE));
        _lastPos = 0;

        memset(_szSendbuf,0,sizeof(SEND_BUFF_SIZE));
        _lastSendPos = 0;

        resetDTHeart();
    }

    SOCKET sockfd(){
        return _sockfd;
    }

    char* msgBuf(){
        return _szMsgbuf;
    }

    int getLast(){
        return _lastPos;
    }

    void setLast(int pos){
        _lastPos = pos;
    }

    // 发送指定Socket数据
    int SendData(netmsg_DataHeader* header){

        int ret = SOCKET_ERROR;
        // 待发送数据长度
        int nSendLen = header->dataLength;
        // 待发送数据
        const char* pSendData = (const char*)header;

        while(true){
            if(_lastSendPos+nSendLen >= SEND_BUFF_SIZE){
            // 剩余可拷贝数据长度
            int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
            // 拷贝数据
            memcpy(_szSendbuf+_lastSendPos,pSendData,nCopyLen);
            // 计算缓冲区尾部位置
            pSendData += nCopyLen;
            // 计算消息剩余长度
            nSendLen -= nCopyLen;
            // 发送数据
            ret = send(_sockfd,_szSendbuf,SEND_BUFF_SIZE,0);
            _lastSendPos = 0;

            if (SOCKET_ERROR == ret)
            {
                return ret;
            }
            

        }else{

            memcpy(_szSendbuf+_lastSendPos,pSendData,nSendLen);
            _lastSendPos += nSendLen;
            break;
        }
    }
        
    return ret;
    
            
 }

void resetDTHeart(){
    _dtHeart = 0;
}

// 检测心跳
bool checkHeart(time_t dt){
    _dtHeart += dt;
    if(_dtHeart >= CLIENT_HEART_DEAD_TIME){
        printf("checkheart death:socket=%d,time=%d\n",_sockfd,_dtHeart);
        return true;
    }
    return false;
}

    
private:
//fd_set file desc set
    SOCKET _sockfd;
    // 第二消息缓冲区
    char _szMsgbuf[RECV_BUFF_SIZE];
    // 消息缓冲区数据尾部位置
    int _lastPos;
    // 发送缓冲区
    char _szSendbuf[SEND_BUFF_SIZE];
    // 消息缓冲区数据尾部位置
    int _lastSendPos;
    // 心跳计时
    time_t _dtHeart;
};




#endif