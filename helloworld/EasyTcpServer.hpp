#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
// #define FD_SETSIZE 100

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

#pragma comment(lib,"ws2_32.lib")

#ifndef RECV_BUFF_SIZE
// 缓冲区最小单元大小
#define RECV_BUFF_SIZE 10240
#endif


using namespace std;



class ClientSocket{

public:
    ClientSocket(SOCKET sockfd = INVALID_SOCKET){
        _sockfd = sockfd;
        memset(_szMsgbuf,0,sizeof(_szMsgbuf));
        _lastPos = 0;
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
    
private:
//fd_set file desc set
    SOCKET _sockfd;
    // 第二消息缓冲区
    char _szMsgbuf[RECV_BUFF_SIZE*10];
    // 消息缓冲区数据尾部位置
    int _lastPos = 0;

};


class EasyTcpServer
{
private:
    SOCKET _sock;
    std::vector<ClientSocket*> _clients;
    CELLTimestamp _tTimer;
    int _recvCount;

public:
    EasyTcpServer(/* args */){
        _sock = INVALID_SOCKET;
        _recvCount = 0;
    }
    ~EasyTcpServer(){
        Close();
    }


    // 初始化socket
SOCKET InitSocket(){
    
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
    _sock = socket(AF_INET,SOCK_STREAM,0);
    if (INVALID_SOCKET == _sock){
        printf("construct error\n");
    }else{
        printf("construct <socket=%d> success\n",_sock);
    }

    return _sock;
}

// 绑定IP和端口
int Bind(const char* ip,unsigned short port){

     if(INVALID_SOCKET == _sock){
            InitSocket();
        }
     sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(port);
    if(ip){
        _sin.sin_addr.S_un.S_addr = inet_addr(ip);
    }else{
        _sin.sin_addr.S_un.S_addr = INADDR_ANY;
    }
    
    int ret = bind(_sock,(sockaddr*)&_sin,sizeof(sockaddr_in));
    if(SOCKET_ERROR == ret){
        printf("bind <port=%d> ERROR\n",port);
    }else{
        printf("bind <port=%d> success\n",port);
    }
    return ret;
}

// 监听端口
int Listen(int n){
    // 3监听网络
    int ret = listen(_sock,n);
    if(SOCKET_ERROR == ret){
        printf("<socket=%d>listen ERROR\n",_sock);
    }else{
        printf("<socket=%d>listen success\n",_sock);
    }
    return ret;
}

// 接收客户端连接

SOCKET Acccept(){
    // 4等待客户端连接
    sockaddr_in clientAddr = {};
    int nAddLen = sizeof(sockaddr_in);
    SOCKET cSock = INVALID_SOCKET;
    

    cSock = accept(_sock,(sockaddr*)&clientAddr,&nAddLen);
    if(INVALID_SOCKET == cSock){
        printf("<socket=%d>ERROR client sock\n",_sock);
    }else{
        NewUserJoin userJoin;
        SendData2ALL(&userJoin);
        _clients.push_back(new ClientSocket(cSock));
        printf("<socket=%d> new client:socket = %d,IP = %s \n",_sock,(int)cSock, inet_ntoa(clientAddr.sin_addr));
    }

    return cSock;
}
// 关闭socket
void Close(){

    if (_sock != INVALID_SOCKET){
        for (size_t i = _clients.size()-1; i >= 0; i--)
        {
            closesocket(_clients[i]->sockfd());
            delete _clients[i];
        }

        closesocket(_sock);
        WSACleanup();
        printf("mission over\n");
        _clients.clear();
    }
    
}
// 处理网络消息
bool OnRun(){

    if(isRun()){
        // 伯克利套接字
        fd_set fdRead;
        fd_set fdWrite;
        fd_set fdExp;
        // 清理集合
        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExp);
        // 将描述符（socket）加入集合
        FD_SET(_sock,&fdRead);
        FD_SET(_sock,&fdWrite);
        FD_SET(_sock,&fdExp);
        
        SOCKET maxSock = _sock;
        // 因为要调函数，所以用减减，这样能减少调用的次数
        for (int i = (int)_clients.size()-1; i >= 0 ; i--)
        {
            FD_SET(_clients[i]->sockfd(),&fdRead);
            if(maxSock < _clients[i]->sockfd()){
                maxSock = _clients[i]->sockfd();
            }
        }
        
        
        // nfds是整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量
        // 即是所有文件描述符最大值+1，在windows中这个参数可以为0

        timeval tv = {1,0};
        int ret = select(maxSock+1,&fdRead,&fdWrite,&fdExp,&tv);

        if(ret < 0){
            printf("End select\n");
            Close();
            return false;
        }
        // 判断描述符（sock）是否在集合中
        if (FD_ISSET(_sock,&fdRead)){
            FD_CLR(_sock,&fdRead);
            Acccept();
        }

        for (int i = (int)_clients.size()-1; i >= 0; i--)
        {
            if (FD_ISSET(_clients[i]->sockfd(),&fdRead))
            {
               if(-1 == RecvData(_clients[i])){
                auto iter = _clients.begin() + i;
                if(iter != _clients.end()){
                    delete _clients[i];
                    _clients.erase(iter);
                }
                }
            }
            
        }
        return true;
    }
    return false;

     
}


// 是否在工作中
bool isRun(){
        return _sock != INVALID_SOCKET;
    }

char _szRecv[RECV_BUFF_SIZE] = {};

// 接收数据 处理粘包 拆分包
int RecvData(ClientSocket* pClient){

    //5接收客户端请求数据
    // 缓冲区
    int nlen = recv(pClient->sockfd(),_szRecv,RECV_BUFF_SIZE,0);
   
    if(nlen <= 0){
        printf("client <Socket=%d> exit,mission over \n",pClient->sockfd());
        return -1;
    }
    // 将收取的数据copy到消息缓冲区
    memcpy(pClient->msgBuf()+pClient->getLast(),_szRecv,nlen);

    // 消息缓冲区的数据尾部后移
    pClient->setLast(pClient->getLast() + nlen);
    //判断消息缓冲区的数据长度大于消息头长度 
    // 此时就可以知道当前消息的长度
    while(pClient->getLast() >= sizeof(DataHeader)){
        DataHeader* header = (DataHeader *)pClient->msgBuf();
        // 判断消息缓冲区的数据长度大于消息长度
        if(pClient->getLast() >= header->dataLength){
            // 剩余消息的长度
            int nSize = pClient->getLast()-header->dataLength;
            // 处理网络消息
            onNetMsg(pClient->sockfd(),header);
            // 将未处理数据迁移
            memcpy(pClient->msgBuf(),pClient->msgBuf()+header->dataLength,nSize);
            // 消息缓冲区尾部位置前移
            pClient->setLast(nSize);
        }else{
            // 消息缓冲区剩余数据不够完整消息
            break;
        }
    }

    return 0;
    
}

// 响应网络消息

virtual void onNetMsg(SOCKET cSock,DataHeader* header){
    ++_recvCount;
    auto t1 = _tTimer.getElapsedSecond();
    if(t1 >= 1.0){
        printf("time<%1f>,socket<%d>,clients<%d>,recvCount<%d>\n",t1,_sock,_clients.size(),_recvCount);
        ++_recvCount =0;
        _tTimer.update();
    }
    switch (header->cmd)
        {
        case CMD_LOGIN:
            {
                Login* login = (Login*) header;
                // printf("client <Socket=%d> message:CMD_LOGIN,message length:%d,userName:%s,passWord: %s \n",cSock,login->dataLength, login->userName,login->userPassWord);
                // 暂时忽略判断用户名密码正确与否
                LoginResult ret;
                SendData(cSock,&ret);
            }
            break;
        case CMD_LOGOUT:
            {
                // Logout logout;
                Logout* logout = (Logout*) header;
                // printf("client <Socket=%d> message:CMD_LOGOUT,message length:%d,userName:%s \n",cSock,logout->dataLength, logout->userName);
                // 暂时忽略判断用户名密码正确与否
                LogOutResult ret;
                SendData(cSock,&ret);
            }
            break;
        default:
            {
                printf("server <Socket=%d> unknown message,message length:%d \n",cSock,header->dataLength);
                // DataHeader ret;
                // SendData(cSock,&ret);
            }
        
            break;
        }

}

// 发送指定Socket数据
int SendData(SOCKET cSock,DataHeader* header){

        if (isRun() && header)
        {
            return send(cSock,(const char*)header,header->dataLength,0);
        }

        return SOCKET_ERROR;
        
    }

void SendData2ALL(DataHeader* header){
        
        for (int i = (int)_clients.size()-1; i >= 0 ; i--)
        {
            SendData(_clients[i]->sockfd(),header);
        }
        
    }

};






#endif