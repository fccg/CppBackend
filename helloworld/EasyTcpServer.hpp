#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_

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

#pragma comment(lib,"ws2_32.lib")


using namespace std;

class EasyTcpServer
{
private:
    SOCKET _sock;
    std::vector<SOCKET> g_clents;

public:
    EasyTcpServer(/* args */){
        _sock = INVALID_SOCKET;
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
    SOCKET _cSock = INVALID_SOCKET;
    

    _cSock = accept(_sock,(sockaddr*)&clientAddr,&nAddLen);
    if(INVALID_SOCKET == _cSock){
        printf("<socket=%d>ERROR client sock\n",_sock);
    }else{
        NewUserJoin userJoin;
        SendData2ALL(&userJoin);
        g_clents.push_back(_cSock);
        printf("<socket=%d> new client:socket = %d,IP = %s \n",_sock,(int)_cSock, inet_ntoa(clientAddr.sin_addr));
    }

    return _cSock;
}
// 关闭socket
void Close(){

    if (_sock != INVALID_SOCKET){
        for (size_t i = g_clents.size()-1; i >= 0; i--)
        {
            closesocket(g_clents[i]);
        }

        closesocket(_sock);
        WSACleanup();
        printf("mission over\n");
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
        
        // 因为要调函数，所以用减减，这样能减少调用的次数
        for (int i = (int)g_clents.size()-1; i >= 0 ; i--)
        {
            FD_SET(g_clents[i],&fdRead);
        }
        
        
        // nfds是整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量
        // 即是所有文件描述符最大值+1，在windows中这个参数可以为0

        timeval tv = {1,0};
        int ret = select(_sock+1,&fdRead,&fdWrite,&fdExp,NULL);

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

        for (size_t i = 0; i < fdRead.fd_count; i++)
        {
            if(-1 == RecvData(fdRead.fd_array[i])){
                auto iter = find(g_clents.begin(),g_clents.end(),fdRead.fd_array[i]);
                if(iter != g_clents.end()){
                    g_clents.erase(iter);
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


// 接收数据 处理粘包 拆分包
int RecvData(SOCKET _cSock){

    //5接收客户端请求数据
    // 缓冲区
    char szRecv[1024] = {};
    int nlen = recv(_cSock,szRecv,sizeof(DataHeader),0);
    DataHeader* header = (DataHeader *)szRecv;
    if(nlen <= 0){
        printf("client <Socket=%d> exit,mission over \n",_cSock);
        return -1;
    }
    recv(_cSock,szRecv + sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
    onNetMsg(_cSock,header);
    return 0;
    
}

// 响应网络消息

virtual void onNetMsg(SOCKET _cSock,DataHeader* header){
    switch (header->cmd)
        {
        case CMD_LOGIN:
            {
                Login* login = (Login*) header;
                printf("client <Socket=%d> message:CMD_LOGIN,message length:%d,userName:%s,passWord: %s \n",_cSock,login->dataLength, login->userName,login->userPassWord);
                // 暂时忽略判断用户名密码正确与否
                LoginResult ret;
                send(_cSock,(char*)&ret,sizeof(LoginResult),0);
            }
            break;
        case CMD_LOGOUT:
            {
                // Logout logout;
                Logout* logout = (Logout*) header;
                printf("client <Socket=%d> message:CMD_LOGOUT,message length:%d,userName:%s \n",_cSock,logout->dataLength, logout->userName);
                // 暂时忽略判断用户名密码正确与否
                LogOutResult ret;
                send(_cSock,(char*)&ret,sizeof(LogOutResult),0);
            }
            break;
        default:
            {
                DataHeader header = {0,CMD_ERROR};
                send(_cSock,(char*)&header,sizeof(DataHeader),0);
            }
        
            break;
        }

}

// 发送指定Socket数据
int SendData(SOCKET _cSock,DataHeader* header){

        if (isRun() && header)
        {
            return send(_cSock,(const char*)header,header->dataLength,0);
        }

        return SOCKET_ERROR;
        
    }

void SendData2ALL(DataHeader* header){
        
        for (int i = (int)g_clents.size()-1; i >= 0 ; i--)
        {
            SendData(g_clents[i],header);
        }
        
    }

};






#endif