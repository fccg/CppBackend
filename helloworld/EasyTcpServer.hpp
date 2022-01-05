#ifndef _EasyTcpServer_hpp_

#define _EasyTcpServer_hpp_
// #define FD_SETSIZE 1000

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <stdio.h>

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <atomic>


#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"

#pragma comment(lib,"ws2_32.lib")

#ifndef RECV_BUFF_SIZE
// 缓冲区最小单元大小
#define RECV_BUFF_SIZE 10240
#endif


using namespace std;

#define _CELLSERVER_THREAD_COUNT_ 6

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
    int _lastPos;

};


class INetEvent
{
private:
    /* data */
public:

    // 客户端退出事件
    virtual void onLeave(ClientSocket* pClient) = 0;

};



class CellServer{
public:
    CellServer(SOCKET sock = INVALID_SOCKET){
        _sock = sock;
        _pThread = nullptr;
        _recvCount = 0;
        _pNetEvent = nullptr;
    }
    ~CellServer(){

        Close();
        _sock = INVALID_SOCKET;
    }

    void setEventObj(INetEvent* event){
        _pNetEvent = event;
    }

    // 是否在工作中
    bool isRun(){
        return _sock != INVALID_SOCKET;
    }


    // 关闭socket
    void Close(){

        if (_sock != INVALID_SOCKET){
            for (int i = _clients.size()-1; i >= 0; i--)
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

    //接收缓冲区
    char _szRecv[RECV_BUFF_SIZE] = {};
    // 接收数据 处理粘包 拆分包
    int RecvData(ClientSocket* pClient){

        //5接收客户端请求数据
        // 缓冲区
        int nlen = (int)recv(pClient->sockfd(),_szRecv,RECV_BUFF_SIZE,0);
    
        if(nlen <= 0){
            printf("client <Socket=%d> exit,mission finish \n",pClient->sockfd());
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
    
    // 处理网络消息
    // int _nCount = 0;
    bool OnRun(){

        while(isRun()){

            if(_clientsBuff.size() > 0){
                // 从缓冲队列中取出数据
                std::lock_guard<std::mutex> mymutex(_mutex);
                for(auto pClient:_clientsBuff){
                    _clients.push_back(pClient);
                }
                _clientsBuff.clear();
            }
            // 如果没有要处理的客户端就跳过
            if (_clients.empty())
            {
                std::chrono::milliseconds dua(1);
                std::this_thread::sleep_for(dua);
                continue;
            }
            

            // 伯克利套接字
            fd_set fdRead;
            // fd_set fdWrite;
            // fd_set fdExp;
            // 清理集合
            FD_ZERO(&fdRead);
            // FD_ZERO(&fdWrite);
            // FD_ZERO(&fdExp);
            
            // 主线程不需要在此轮询
            // FD_SET(_sock,&fdRead);
            // FD_SET(_sock,&fdWrite);
            // FD_SET(_sock,&fdExp);
            
            // 将描述符（socket）加入集合
            SOCKET maxSock = _clients[0]->sockfd();
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

            // timeval tv = {1,0};
            // int ret = select(maxSock+1,&fdRead,&fdWrite,&fdExp,&tv);
            int ret = select(maxSock+1,&fdRead,nullptr,nullptr,nullptr);

            if(ret < 0){
                printf("End select\n");
                Close();
                return false;
            }
            // // 判断描述符（sock）是否在集合中
            // if (FD_ISSET(_sock,&fdRead)){
            //     FD_CLR(_sock,&fdRead);
            //     Acccept();
            //     return true;
            // }

            for (int i = (int)_clients.size()-1; i >= 0; i--)
            {
                if (FD_ISSET(_clients[i]->sockfd(),&fdRead))
                {
                if(-1 == RecvData(_clients[i])){
                    auto iter = _clients.begin() + i;
                    if(iter != _clients.end()){
                        if (_pNetEvent)
                        {
                            _pNetEvent->onLeave(_clients[i]);
                        }
                        
                        delete _clients[i];
                        _clients.erase(iter);
                    }
                    }
                }
            }
            // return true;
        }
        // return false;
    }

    // 响应网络消息
    virtual void onNetMsg(SOCKET cSock,DataHeader* header){
        _recvCount++;
        // auto t1 = _tTimer.getElapsedSecond();
        // if(t1 >= 1.0){
        //     printf("time<%1f>,socket<%d>,clients<%d>,recvCount<%d>\n",t1,_sock,_clients.size(),_recvCount);
        //     _recvCount =0;
        //     _tTimer.update();
        // }
        switch (header->cmd)
            {
            case CMD_LOGIN:
                {
                    Login* login = (Login*) header;
                    // printf("client <Socket=%d> message:CMD_LOGIN,message length:%d,userName:%s,passWord: %s \n",cSock,login->dataLength, login->userName,login->userPassWord);
                    // 暂时忽略判断用户名密码正确与否
                    // LoginResult ret;
                    // SendData(cSock,&ret);
                }
                break;
            case CMD_LOGOUT:
                {
                    // Logout logout;
                    Logout* logout = (Logout*) header;
                    // printf("client <Socket=%d> message:CMD_LOGOUT,message length:%d,userName:%s \n",cSock,logout->dataLength, logout->userName);
                    // 暂时忽略判断用户名密码正确与否
                    // LogOutResult ret;
                    // SendData(cSock,&ret);
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

    void addClient(ClientSocket* pClient) {

        std::lock_guard<std::mutex> mymutex(_mutex);
        // _mutex.lock();
        _clientsBuff.push_back(pClient);
        // _mutex.unlock();

    }

    void Start(){
        _pThread = new thread(std::mem_fun(&CellServer::OnRun),this);
    }

    size_t getClientCount(){
        return _clients.size() + _clientsBuff.size();
    }


private:
    SOCKET _sock;
    //正式客户队列
    std::vector<ClientSocket*> _clients;
    //客户缓冲队列
    std::vector<ClientSocket*> _clientsBuff;
    std::mutex _mutex;
    std::thread* _pThread;
    INetEvent* _pNetEvent;
public:
    std::atomic<int> _recvCount;

};



class EasyTcpServer : public INetEvent
{
private:
    SOCKET _sock;
    std::vector<ClientSocket*> _clients;
    std::vector<CellServer*> _cellservers;
    CELLTimestamp _tTimer;

public:
    EasyTcpServer(/* args */){
        _sock = INVALID_SOCKET;
        // _recvCount = 0;
    }
    virtual ~EasyTcpServer(){
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
            printf("<socket=%d>close old connect",(int)_sock);
            Close();
        }

        // 1建立一个socket套接字
        _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if (INVALID_SOCKET == _sock){
            printf("construct error\n");
        }else{
            printf("construct <socket=%d> success\n",(int)_sock);
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
            printf("<socket=%d>ERROR client sock\n",(int)_sock);
        }else{
            // NewUserJoin userJoin;
            // SendData2ALL(&userJoin);
            addClient2CellServer(new ClientSocket(cSock));
            // printf("<socket=%d> new client:socket = %d,IP = %s \n",_sock,(int)cSock, inet_ntoa(clientAddr.sin_addr));
        }

        return cSock;
    }

    void addClient2CellServer(ClientSocket* pClient){

        _clients.push_back(pClient);
        auto minCellServer = _cellservers[0];
        // 找客户数量最少的CellServer消息处理线程
        for (auto pCellServer : _cellservers)
        {
            if(pCellServer->getClientCount() < minCellServer->getClientCount()){
                minCellServer = pCellServer;
            }
        }
        minCellServer->addClient(pClient);
        
    }


    void Start(){
        for (size_t i = 0; i < _CELLSERVER_THREAD_COUNT_; i++)
        {
            auto ser = new CellServer(_sock);
            _cellservers.push_back(ser);
            ser->setEventObj(this);
            ser->Start();
        }
        
    }

    // 关闭socket
    void Close(){

        if (_sock != INVALID_SOCKET){
            for (int i = _clients.size()-1; i >= 0; i--)
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


    // 是否在工作中
    bool isRun(){
        return _sock != INVALID_SOCKET;
    }



    // 处理网络消息
    // int _nCount = 0;
    bool OnRun(){

        if(isRun()){

            msgPerSec();

            // 伯克利套接字
            fd_set fdRead;
            // fd_set fdWrite;
            // fd_set fdExp;
            // 清理集合
            FD_ZERO(&fdRead);
            // FD_ZERO(&fdWrite);
            // FD_ZERO(&fdExp);
            // 将描述符（socket）加入集合
            FD_SET(_sock,&fdRead);
            // FD_SET(_sock,&fdWrite);
            // FD_SET(_sock,&fdExp);
            
            // SOCKET maxSock = _sock;
            // 因为要调函数，所以用减减，这样能减少调用的次数
            // for (int i = (int)_clients.size()-1; i >= 0 ; i--)
            // {
            //     FD_SET(_clients[i]->sockfd(),&fdRead);
            //     if(maxSock < _clients[i]->sockfd()){
            //         maxSock = _clients[i]->sockfd();
            //     }
            // }
            
            
            // nfds是整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量
            // 即是所有文件描述符最大值+1，在windows中这个参数可以为0

            timeval tv = {0,10};
            // int ret = select(_sock+1,&fdRead,&fdWrite,&fdExp,&tv);
            int ret = select(_sock+1,&fdRead,nullptr,nullptr,&tv);

            if(ret < 0){
                printf("select mission finish\n");
                Close();
                return false;
            }

            // 判断描述符（sock）是否在集合中
            if (FD_ISSET(_sock,&fdRead)){
                FD_CLR(_sock,&fdRead);
                Acccept();
                return true;
            }

            // for (int i = (int)_clients.size()-1; i >= 0; i--)
            // {
            //     if (FD_ISSET(_clients[i]->sockfd(),&fdRead))
            //     {
            //     if(-1 == RecvData(_clients[i])){
            //         auto iter = _clients.begin() + i;
            //         if(iter != _clients.end()){
            //             delete _clients[i];
            //             _clients.erase(iter);
            //         }
            //         }
            //     }
                
            // }
            return true;
        }
        return false;
    }

    

    // 响应网络消息
    void msgPerSec(){
        auto t1 = _tTimer.getElapsedSecond();
        if(t1 >= 1.0){
            int recvCount = 0;
            for (auto ser:_cellservers)
            {
                recvCount += ser->_recvCount;
                ser->_recvCount = 0;
            }
            printf("cellcount<%d>,time<%1f>,socket<%d>,clients<%d>,recvCount<%d>\n",_cellservers.size(),t1,_sock,static_cast<int>(_clients.size()),static_cast<int>(recvCount/t1));
            _tTimer.update();
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


    void onLeave(ClientSocket* pClient){

        for (int i = (int)_clients.size()-1; i >= 0 ; i--)
        {
            if (_clients[i] == pClient)
            {
                auto iter = _clients.begin() + i;
                if(iter != _clients.end())
                _clients.erase(iter);

            }
            ;
        }
    }

};






#endif