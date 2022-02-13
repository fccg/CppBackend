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
#include <map>
#include <memory>


#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"
#include "CellTask.hpp"


#pragma comment(lib,"ws2_32.lib")

#ifndef RECV_BUFF_SIZE
// 接收缓冲区最小单元大小
#define RECV_BUFF_SIZE 10240
#endif
#ifndef SEND_BUFF_SIZE
// 发送缓冲区最小单元大小
#define SEND_BUFF_SIZE 10240
#endif


using namespace std;
 
// #define _CELLSERVER_THREAD_COUNT_ 6


// 客户端数据类型
class ClientSocket{

public:
    ClientSocket(SOCKET sockfd = INVALID_SOCKET){
        _sockfd = sockfd;

        memset(_szMsgbuf,0,sizeof(RECV_BUFF_SIZE));
        _lastPos = 0;

        memset(_szSendbuf,0,sizeof(SEND_BUFF_SIZE));
        _lastSendPos = 0;
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
    int SendData(DataHeader* header){

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

};


// 预声明
class CellServer;

// 网络事件接口
class INetEvent
{
private:
    /* data */
public:
    // 客户端加入事件
    virtual void onNetJoin(std::shared_ptr<ClientSocket>& pClient) = 0;

    // 客户端退出事件
    virtual void onNetLeave(std::shared_ptr<ClientSocket>& pClient) = 0;

    //客户端发送消息事件
    virtual void onNetMsg(CellServer* pCellServer,std::shared_ptr<ClientSocket>& pClient,DataHeader* header) = 0;

    //recv事件
    virtual void onNetRecv(std::shared_ptr<ClientSocket>& pClient) = 0;

};


class sendMsg2Client:public CellTask
{
private:
    std::shared_ptr<ClientSocket> _pClient;
    DataHeader* _pHeader;
public:
    sendMsg2Client(std::shared_ptr<ClientSocket> pClient,DataHeader* header):_pClient(pClient),_pHeader(header){

    }
    

    void doTask(){
        _pClient->SendData(_pHeader); 
        delete _pHeader;
    }
};



// 网络消息接受处理服务类
class CellServer{

public:

    CellServer(SOCKET sock = INVALID_SOCKET){
        _sock = sock;
       
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
            for (auto iter:_clients)
            {
                closesocket(iter.second->sockfd());
                // delete iter.second;
            }

            closesocket(_sock);
            // 清除环境在EasyTcpServer中做了
            // WSACleanup();
            printf("mission over\n");
            _clients.clear();
        }
        
    }

    //接收缓冲区
    // char _szRecv[RECV_BUFF_SIZE] = {};
    // 接收数据 处理粘包 拆分包
    int RecvData(std::shared_ptr<ClientSocket> pClient){

        //5接收客户端请求数据
        // 缓冲区
        char* szRecv = pClient->msgBuf() + pClient->getLast();
        int nlen = (int)recv(pClient->sockfd(),szRecv,RECV_BUFF_SIZE-pClient->getLast(),0);
        _pNetEvent->onNetRecv(pClient);

        if(nlen <= 0){
            //  
            // printf("client <Socket=%d> exit,mission finish \n",pClient->sockfd());
            return -1;
        }
        // 将收取的数据copy到消息缓冲区
        // memcpy(pClient->msgBuf()+pClient->getLast(),_szRecv,nlen);

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
                onNetMsg(pClient,header);
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
    // 备份socket fd_set
    fd_set _fdRead_bak;
    // 客户端是否有变化
    bool _clients_Change;
    SOCKET _maxSock;
    bool OnRun(){

        _clients_Change = true;
        while(isRun()){

            if(_clientsBuff.size() > 0){
                // 从缓冲队列中取出数据
                std::lock_guard<std::mutex> mymutex(_mutex);
                for(auto pClient:_clientsBuff){
                    _clients[pClient->sockfd()] = pClient;
                }
                _clientsBuff.clear();
                _clients_Change = true;
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
            if (_clients_Change)
            {   
                _clients_Change = false;
                _maxSock = _clients.begin()->second->sockfd();
                // 因为要调函数，所以用减减，这样能减少调用的次数
                for (auto iter:_clients)
                {
                    FD_SET(iter.second->sockfd(),&fdRead);
                    if(_maxSock <iter.second->sockfd()){
                        _maxSock = iter.second->sockfd();
                    }
                }

                memcpy(&_fdRead_bak,&fdRead,sizeof(fd_set));
            }else{

                memcpy(&fdRead,&_fdRead_bak,sizeof(fd_set));

            }
            
            
            
            
            // nfds是整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量
            // 即是所有文件描述符最大值+1，在windows中这个参数可以为0

            // timeval tv = {1,0};
            // int ret = select(maxSock+1,&fdRead,&fdWrite,&fdExp,&tv);
            int ret = select(_maxSock+1,&fdRead,nullptr,nullptr,nullptr);

            if(ret < 0){
                printf("End select\n");
                Close();
                return false;
            }else if (ret == 0)
            {
                continue;
            }
            
            // // 判断描述符（sock）是否在集合中
            // if (FD_ISSET(_sock,&fdRead)){
            //     FD_CLR(_sock,&fdRead);
            //     Acccept();
            //     return true;
            // }
            

            for (int i = 0; i < fdRead.fd_count; i++)
            {
                auto iter = _clients.find(fdRead.fd_array[i]);
                if (iter != _clients.end())
                {
                    if(-1 == RecvData(iter->second)){
                        
                        if (_pNetEvent)
                        {
                            _pNetEvent->onNetLeave(iter->second);
                        }
                        _clients_Change = true;
                        // delete _clients[i];
                        _clients.erase(iter->first);
                    }
                }else {
					    printf("error. if (iter != _clients.end())\n");
				}
            }
        }
    }

    // 响应网络消息
    virtual void onNetMsg(std::shared_ptr<ClientSocket> pClient,DataHeader* header){
        
        // auto t1 = _tTimer.getElapsedSecond();
        // if(t1 >= 1.0){
        //     printf("time<%1f>,socket<%d>,clients<%d>,recvCount<%d>\n",t1,_sock,_clients.size(),_recvCount);
        //     _recvCount =0;
        //     _tTimer.update();
        // }
        _pNetEvent->onNetMsg(this,pClient, header);

    }

    void addClient(std::shared_ptr<ClientSocket> pClient) {

        std::lock_guard<std::mutex> mymutex(_mutex);
        // _mutex.lock();
        _clientsBuff.push_back(pClient);
        // _mutex.unlock();

    }

    void Start(){
        _Thread = thread(std::mem_fun(&CellServer::OnRun),this);
        _taskServer.Start();
    }

    size_t getClientCount(){
        return _clients.size() + _clientsBuff.size();
    }

    void addSendTask(std::shared_ptr<ClientSocket> pClient,DataHeader* header){

        auto task = std::make_shared<sendMsg2Client>(pClient,header);
        _taskServer.addTask(task);
    }


private:
    SOCKET _sock;
    //正式客户队列
    std::map<SOCKET,std::shared_ptr<ClientSocket>> _clients;
    //客户缓冲队列
    std::vector<std::shared_ptr<ClientSocket>> _clientsBuff;
    // 缓冲队列锁
    std::mutex _mutex;
    std::thread _Thread;
    // 网络事件对象
    INetEvent* _pNetEvent;
    // 
    CellTaskServer _taskServer;
};



class EasyTcpServer : public INetEvent
{
private:
    SOCKET _sock;
    // 只管理EasyTcpServer线程，客户端全部交给cellserver
    // std::vector<ClientSocket*> _clients;
    // 消息处理对象容器
    std::vector<std::shared_ptr<CellServer>> _cellservers;
    // 为统计每秒消息而存在的计时器
    CELLTimestamp _tTimer;
protected:
    // Recv计数
    std::atomic<int> _msgCount;
    // 消息计数
    std::atomic<int> _recvCount;
    // 客户端计数
    std::atomic<int> _clientCount;
    
    
    
public:
    EasyTcpServer(){
        _sock = INVALID_SOCKET;
        _msgCount = 0;
        _recvCount = 0;
        _clientCount = 0;
        
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
            // 将客户端分给任务最少的cellserver 
            
            addClient2CellServer(std::make_shared<ClientSocket>(cSock));
            // 客户端IP地址：inet_ntoa(clientAddr.sin_addr)
            // printf("<socket=%d> new client:socket = %d,IP = %s \n",_sock,(int)cSock, );
        }

        return cSock;
    }

    void addClient2CellServer(std::shared_ptr<ClientSocket> pClient){

        // _clients.push_back(pClient);
        auto minCellServer = _cellservers[0];
        // 找客户数量最少的CellServer消息处理线程
        for (auto pCellServer : _cellservers)
        {
            if(pCellServer->getClientCount() < minCellServer->getClientCount()){
                minCellServer = pCellServer;
            }
        }
        minCellServer->addClient(pClient);
        onNetJoin(pClient);
    }


    void Start(int nCellServer){
        for (size_t i = 0; i < nCellServer; i++)
        {
            auto ser = std::make_shared<CellServer>(_sock);
            _cellservers.push_back(ser);
            // 注册网络事件接收对象
            ser->setEventObj(this);
            // 启动消息处理线程
            ser->Start();
        }
        
    }

    // 关闭socket
    void Close(){

        if (_sock != INVALID_SOCKET){
            // 在cellserver里面做掉了
            // for (int i = _clients.size()-1; i >= 0; i--)
            // {
            //     closesocket(_clients[i]->sockfd());
            //     delete _clients[i];
            // }

            closesocket(_sock);
            WSACleanup();
            printf("mission over\n");
            // _clients.clear();
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
                printf("select Acccept mission finish\n");
                Close();
                return false;
            }

            // 判断描述符（sock）是否在集合中
            if (FD_ISSET(_sock,&fdRead)){
                FD_CLR(_sock,&fdRead);
                Acccept();
                return true;
            }
            return true;
        }
        return false;
    }

    

    // 计算并输出每秒收到的网络消息
    void msgPerSec(){
        auto t1 = _tTimer.getElapsedSecond();
        if(t1 >= 1.0){
            printf("thread of cell<%d>,time<%lf>,socket<%d>,clientsCount<%d>,recvCount<%d>,msgCount<%d>\n",_cellservers.size(),t1,_sock,static_cast<int>(_clientCount),static_cast<int>(_recvCount/t1),static_cast<int>(_msgCount/t1));
            // 为什么TMD这个就对了
            // printf("thread<%d>,time<%lf>,socket<%d>,clients<%d>,recvCount<%d>\n", _cellservers.size(), t1, _sock,(int)_clientCount, (int)(_recvCount/ t1));
            _recvCount = 0;
            _msgCount = 0;
            _tTimer.update();

        }
    }

    // 只会被接收线程调用，线程安全
    virtual void onNetJoin(std::shared_ptr<ClientSocket>& pClient){
        ++_clientCount;
    }

    //cellserver*6,线程不安全 
    virtual void onNetLeave(std::shared_ptr<ClientSocket>& pClient){

        --_clientCount;
        // 退出也在cellserver里面做了
        // for (int i = (int)_clients.size()-1; i >= 0 ; i--)
        // {
        //     if (_clients[i] == pClient)
        //     {
        //         auto iter = _clients.begin() + i;
        //         if(iter != _clients.end())
        //         _clients.erase(iter);

        //     }
        //     ;
        // }
    }

    //cellserver*6,线程不安全
    virtual void onNetMsg(CellServer* pCellServer,std::shared_ptr<ClientSocket>& pClient,DataHeader* header){

        _msgCount++;
    
    }


    virtual void onNetRecv(std::shared_ptr<ClientSocket>& pClient){

        _recvCount++;
    
    }

};






#endif