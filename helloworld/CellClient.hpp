#ifndef _CELL_ClIENT_HPP_
#define _CELL_ClIENT_HPP_


#include "ShareLib.hpp"

// 客户端心跳检测时间间隔
#define CLIENT_HEART_DEAD_TIME 60000
// 发送间隔
#define CLIENT_SEND_IMMED_TIME 200


// 客户端数据类型
class CellClient : public ObjectPoolBase<CellClient,6666>
{

public:
    int id = -1;

public:
    CellClient(SOCKET sockfd = INVALID_SOCKET){

        static int num = 1;
        id = num++;

        _sockfd = sockfd;

        memset(_szMsgbuf,0,sizeof(RECV_BUFF_SIZE));
        _lastPos = 0;

        memset(_szSendbuf,0,sizeof(SEND_BUFF_SIZE));
        _lastSendPos = 0;

        resetDTHeart();
        resetDTSend();
    }

    ~CellClient(){

        printf("CellClient.OnRun%d close1\n",id);
        if (INVALID_SOCKET != _sockfd)
        {
            closesocket(_sockfd);
            _sockfd = INVALID_SOCKET;
            
        }
        printf("CellClient.OnRun%d close2\n",id);

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

        
        if(_lastSendPos+nSendLen <= SEND_BUFF_SIZE){
            // 待发送数据拷贝到发送缓冲区尾部
            memcpy(_szSendbuf+_lastSendPos,pSendData,nSendLen);
            // 更新尾部位置
            _lastSendPos += nSendLen;

            if (_lastSendPos == SEND_BUFF_SIZE)
            {
                _sendBuffFull++;
            }
            return nSendLen;
            
        }else{
             _sendBuffFull++;
        }
        
    return ret;
       
 }

void resetDTHeart(){
    _dtHeart = 0;
}

void resetDTSend(){
    _dtSend = 0;
}



int SendDataIM(){

    int ret = 0;

    if (_lastSendPos > 0 && INVALID_SOCKET != _sockfd)
    {
        // 发送数据
        ret = send(_sockfd,_szSendbuf,_lastSendPos,0);   
        // 发送完数据清零
        _lastSendPos = 0;
        _sendBuffFull = 0;

        resetDTSend();
    }
    return ret;
}

// 定时发送
bool checkSend(time_t dt){
    _dtSend += dt;
    if(_dtSend >= CLIENT_SEND_IMMED_TIME){
        // printf("checkSend:socket=%d,time=%d\n",_sockfd,_dtSend);
        // 立刻发送数据
        SendDataIM();
        // 重置发送
        resetDTSend();
        return true;
    }
    return false;
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
    // 上次发送时间
    time_t _dtSend;
    //发送缓冲区写满 
    int _sendBuffFull = 0;
};




#endif