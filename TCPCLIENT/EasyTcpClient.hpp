#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <windows.h>
    #include <winsock2.h>
    #pragma comment(lib,"ws2_32.lib")  

#endif

#include <stdio.h>
#include "MessageHeader.hpp"

using namespace std;


class EasyTcpClient
{
    SOCKET _sock;
private:
    /* data */

public:
    EasyTcpClient(/* args */){
        _sock = INVALID_SOCKET;
    }
    // 虚析构函数
    virtual ~EasyTcpClient(){
        Close();
    }

    // 初始化socket
    void InitSocket(){
        //启动win sock 2.x的环境 
#ifdef _WIN32
        WORD ver = MAKEWORD(2,2);
        WSADATA dat;
        WSAStartup(ver,&dat);
#endif

        if(INVALID_SOCKET != _sock){
            printf("<socket=%d>close old connect",_sock);
            Close();
        }

        // 1建立一个socket套接字
        _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if (INVALID_SOCKET == _sock){
            printf("construct error\n");
        }else{
            // printf("construct <socket=%d> success\n",_sock);
        }


    }
    // 连接服务器
    int Connect(const char* ip,unsigned short port){
        if(INVALID_SOCKET == _sock){
            InitSocket();
        }

        // 2连接服务器
        sockaddr_in _sin = {};
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port);
        _sin.sin_addr.S_un.S_addr = inet_addr(ip);
        int ret = connect(_sock,(sockaddr*)&_sin,sizeof(sockaddr_in));
        if(SOCKET_ERROR == ret){
            printf("<socket=%d>connect to server error\n",_sock);
        }else{
            // printf("<socket=%d>connect to server success\n",_sock);
        }
        return ret;


    }
    // 关闭socket
    void Close(){
        //启动win sock 2.x的环境
        if (_sock != INVALID_SOCKET){
            // 7关闭
            closesocket(_sock);

            WSACleanup();
            _sock = INVALID_SOCKET;
        }
        

    }

    // 发送数据

    // 接收数据

    // 监听网络消息
    int _nCount = 0;
    bool OnRun(){

        if (isRun())
        {
            fd_set fdReads;
            FD_ZERO(&fdReads);
            FD_SET(_sock,&fdReads);

            timeval tv = {0,0};
            int ret = select(_sock+1,&fdReads,0,0,&tv);
            if (ret<0){
                printf("<socket=%d>misssion over1",_sock);
                Close();
                return false;
            }

            if (FD_ISSET(_sock,&fdReads)){
                FD_CLR(_sock,&fdReads);

                if (-1 == RecvData(_sock)){
                    printf("<socket=%d>mission over2 \n",_sock);
                    Close();
                    return false;
                } 
            }
            return true;
        }
        return false; 
        
    }

    //是否工作中
    bool isRun(){
        return _sock != INVALID_SOCKET;
    }

#ifndef RECV_BUFF_SIZE
// 缓冲区最小单元大小
#define RECV_BUFF_SIZE 10240
#endif

    // 接收缓冲区
    char _szRecv[RECV_BUFF_SIZE] = {};
    // 第二消息缓冲区
    char _szMsgbuf[RECV_BUFF_SIZE*5] = {};
    // 消息缓冲区数据尾部位置
    int _lastPos = 0;

    // 接收数据 之后要处理粘包，拆分包
    int RecvData(SOCKET cSock){

        //5接收客户端请求数据
        int nlen = (int)recv(cSock,_szRecv,RECV_BUFF_SIZE,0);
        // DataHeader* header = (DataHeader *)_szRecv;
        if(nlen <= 0){
            printf("server <Socket=%d> disconnect, mission over \n",cSock);
            return -1;
        }
        // 将收取的数据copy到消息缓冲区
        memcpy(_szMsgbuf+_lastPos,_szRecv,nlen);
        // 消息缓冲区的数据尾部后移
        _lastPos += nlen;
        //判断消息缓冲区的数据长度大于消息头长度 
        // 此时就可以知道当前消息的长度
        while(_lastPos >= sizeof(DataHeader)){
            DataHeader* header = (DataHeader *)_szMsgbuf;
            // 判断消息缓冲区的数据长度大于消息长度
            if(_lastPos >= header->dataLength){
                // 剩余消息的长度
                int nSize = _lastPos-header->dataLength;
                // 处理网络消息
                onNetMsg(header);
                // 将未处理数据迁移
                memcpy(_szMsgbuf,_szMsgbuf+header->dataLength,nSize);
                // 消息缓冲区尾部位置前移
                _lastPos = nSize;
            }else{
                // 消息缓冲区剩余数据不够完整消息
                break;
            }
        }
        // recv(cSock,_szRecv + sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
        
        // onNetMsg(header);
        return 0;     
    }

    // 处理网络消息
    virtual void onNetMsg(DataHeader* header){

        switch (header->cmd)
            {
            case CMD_LOGIN_RESULT:
                {
                    
                    LoginResult* login = (LoginResult*) header;
                    // printf("server <Socket=%d> message:CMD_LOGIN_RESULT,message length:%d \n",_sock,login->dataLength);
                    // 暂时忽略判断用户名密码正确与否
                    // LoginResult ret;
                    // send(_cSock,(char*)&ret,sizeof(LoginResult),0);
                }
                break;
            case CMD_LOGOUT_RESULT:
                {
                    // Logout logout;
                    LogOutResult* logout = (LogOutResult*) header;
                    // printf("server <Socket=%d> message:CMD_LOGOUT_RESULT,message length:%d \n",_sock,logout->dataLength);
                    // 暂时忽略判断用户名密码正确与否
                    // LogOutResult ret;
                    // send(_cSock,(char*)&ret,sizeof(LogOutResult),0);
                }
                break;
            case CMD_NEW_USER_JOIN:
                {
                    // Logout logout;
                    NewUserJoin* userJoin = (NewUserJoin*) header;
                    // printf("server <Socket=%d> message:CMD_NEW_USER_JOIN,message length:%d \n",_sock,userJoin->dataLength);
                    // 暂时忽略判断用户名密码正确与否
                    // LogOutResult ret;b
                    // send(_cSock,(char*)&ret,sizeof(LogOutResult),0);
                }
                break;
                case CMD_ERROR:{
                    printf("server <Socket=%d> message:CMD_ERROR,message length:%d \n",_sock,header->dataLength);
                }
                default:{
                    printf("server <Socket=%d> unknown message,message length:%d \n",_sock,header->dataLength);
                }
            }

    }

    // 发送数据
    int SendData(DataHeader* header,int nLen){

        if (isRun() && header)
        {
            return send(_sock,(const char*)header,nLen,0);
        }

        return SOCKET_ERROR;
        
    }

};


#endif

