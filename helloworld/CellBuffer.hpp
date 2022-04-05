#ifndef _CELL_BUFFER_HPP_
#define _CELL_BUFFER_HPP_

#include "ShareLib.hpp"


class CellBuffer
{
private:
    // 发送缓冲区
    char* _pBuff = nullptr;
    // 消息缓冲区数据尾部位置，即现有数据的长度
    int _lastPos;
    // 缓冲区总空间大小，字节长度
    int _nSize;
    // 缓冲区写满次数计数
    int _BuffFullCount = 0;

public:
    CellBuffer(int nSize = 8192){
        
        _nSize = nSize;
        _pBuff = new char[_nSize];

    }

    ~CellBuffer(){


        if(_pBuff){
            delete[] _pBuff;
            _pBuff = nullptr;
        }
    }



    char* data(){
        return _pBuff;
    }

    // 往缓冲区里面添加数据
    bool push(const char* pData,int nLen){


        if(_lastPos + nLen <= _nSize){
            // 待发送数据拷贝到发送缓冲区尾部
            memcpy(_pBuff+_lastPos,pData,nLen);
            // 更新尾部位置
             _lastPos += nLen;

            if (_lastPos == SEND_BUFF_SIZE)
            {
                ++_BuffFullCount;
            }
            return true;
            
        }else{
             ++_BuffFullCount;
        }

        return false;

    }

    void pop(int nlen){
        
        // 剩余消息长度
        int n = _lastPos - nlen;

        if(n > 0){
            // 用剩余的数据覆盖掉处理完的数据
            memcpy(_pBuff,_pBuff + nlen, n);
            
        }
        _lastPos = n;
        if(_BuffFullCount > 0){
            _BuffFullCount--;
        }
        
    }



    // 立刻发送数据
    int write2socket(SOCKET sockfd){

        int ret = 0;

        if (_lastPos > 0 && INVALID_SOCKET != sockfd)
        {
            // 发送数据
            ret = send(sockfd,_pBuff,_lastPos,0);   
            // 发送完数据清零
            _lastPos = 0;
            _BuffFullCount = 0;

        }
        return ret;
}

    int readfromsocket(SOCKET sockfd){

        if (_nSize - _lastPos > 0)
        {
            // 接收缓冲区
            char* szRecv = _pBuff + _lastPos;
            int nlen = (int)recv(sockfd,szRecv,_nSize-_lastPos,0);

            if(nlen <= 0){
                //  
                // printf("client <Socket=%d> exit,mission finish \n",pClient->sockfd());
                return nlen;
            }
            // 消息缓冲区的数据尾部后移
            _lastPos += nlen;
            return nlen;
        }

        return 0;


    }


    bool hasMsg(){

        //判断消息缓冲区的数据长度大于消息头长度 
        // 此时就可以知道当前消息的长度
        if(_lastPos >= sizeof(netmsg_DataHeader)){
            netmsg_DataHeader* header = (netmsg_DataHeader *)_pBuff;
            // 判断消息缓冲区的数据长度大于消息长度

            return _lastPos >= header->dataLength;
        
        }
        return false;

    }




};




































#endif