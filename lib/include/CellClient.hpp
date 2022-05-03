#ifndef _CELL_ClIENT_HPP_
#define _CELL_ClIENT_HPP_

#include "ShareLib.hpp"
#include "CellBuffer.hpp"

// 客户端心跳检测时间间隔
#define CLIENT_HEART_DEAD_TIME 60000
// 发送间隔
#define CLIENT_SEND_IMMED_TIME 200

// 客户端数据类型
class CellClient : public ObjectPoolBase<CellClient, 6666>
{

public:
    int id = -1;

public:
    CellClient(SOCKET sockfd = INVALID_SOCKET) : _sendBuff(SEND_BUFF_SIZE), _recvBuff(RECV_BUFF_SIZE)
    {

        static int num = 1;
        id = num++;

        _sockfd = sockfd;

        resetDTHeart();
        resetDTSend();
    }

    ~CellClient()
    {

        Logger::Info("CellClient.OnRun%d close1\n", id);
        if (INVALID_SOCKET != _sockfd)
        {
            closesocket(_sockfd);
            _sockfd = INVALID_SOCKET;
        }
        Logger::Info("CellClient.OnRun%d close2\n", id);
    }

    SOCKET sockfd()
    {
        return _sockfd;
    }

    // 发送指定Socket数据
    int SendData(netmsg_DataHeader *header)
    {

        int ret = SOCKET_ERROR;
        // 待发送数据长度
        int nSendLen = header->dataLength;
        // 待发送数据
        const char *pSendData = (const char *)header;

        if (_sendBuff.push(pSendData, nSendLen))
        {

            return nSendLen;
        }

        return ret;
    }


    int SendData(const char* pData,int len)
    {
        
        if (_sendBuff.push(pData, len))
        {

            return len;
        }

        return SOCKET_ERROR;
    }

    void resetDTHeart()
    {
        _dtHeart = 0;
    }

    void resetDTSend()
    {
        _dtSend = 0;
    }

    bool hasMsg(){
        return _recvBuff.hasMsg();
    }

    int RecvData()
    {
        return _recvBuff.readfromsocket(_sockfd);
    }

    int SendDataIM()
    {

        resetDTSend();
        return _sendBuff.write2socket(_sockfd);
    }

    netmsg_DataHeader* front_msg(){

        return (netmsg_DataHeader*)_recvBuff.data();

    }


    void pop_front_msg(){
        
        if(hasMsg()){
            _recvBuff.pop(front_msg()->dataLength);
        }
    }


    bool readyWrite()
    {

        return _sendBuff.readyWrite();

    }



    // 定时发送
    bool checkSend(time_t dt)
    {
        _dtSend += dt;
        if (_dtSend >= CLIENT_SEND_IMMED_TIME)
        {
            // Logger::Info("checkSend:socket=%d,time=%d\n",_sockfd,_dtSend);
            // 立刻发送数据
            SendDataIM();
            // 重置发送
            resetDTSend();
            return true;
        }
        return false;
    }

    // 检测心跳
    bool checkHeart(time_t dt)
    {
        _dtHeart += dt;
        if (_dtHeart >= CLIENT_HEART_DEAD_TIME)
        {
            Logger::Info("checkheart death:socket=%d,time=%d\n", _sockfd, _dtHeart);
            return true;
        }
        return false;
    }

private:
    // fd_set file desc set
    SOCKET _sockfd;
    // 第二接收消息缓冲区
    CellBuffer _recvBuff;
    // 发送缓冲区
    CellBuffer _sendBuff;
    // 心跳计时
    time_t _dtHeart;
    // 上次发送时间
    time_t _dtSend;
    //发送缓冲区写满
    int _sendBuffFull = 0;
};

#endif