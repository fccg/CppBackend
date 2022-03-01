#ifndef _EasyTcpServer_hpp_

#define _EasyTcpServer_hpp_



#include "ShareLib.hpp"
#include "CellClient.hpp"
#include "CellServer.hpp"
#include "INetEvent.hpp"


#include <iostream>
#include <string>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <memory>



using namespace std;
 
// #define _CELLSERVER_THREAD_COUNT_ 6



class EasyTcpServer : public INetEvent
{
private:
    SOCKET _sock;
    // 只管理EasyTcpServer线程，客户端全部交给cellserver
    // std::vector<CellClient*> _clients;
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
            std::shared_ptr<CellClient> c(new CellClient(cSock));
            addClient2CellServer(c);
            // 客户端IP地址：inet_ntoa(clientAddr.sin_addr)
            // printf("<socket=%d> new client:socket = %d,IP = %s \n",_sock,(int)cSock, );
        }

        return cSock;
    }

    void addClient2CellServer(std::shared_ptr<CellClient> pClient){

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
        
    }


    void Start(int nCellServer){
        for (size_t i = 0; i < nCellServer; i++)
        {
            auto ser = std::make_shared<CellServer>(i+1);
            _cellservers.push_back(ser);
            // 注册网络事件接收对象
            ser->setEventObj(this);
            // 启动消息处理线程
            ser->Start();
        }
        
    }

    // 关闭socket
    void Close(){

        printf("EasyTcpServer close1\n");
        if (_sock != INVALID_SOCKET){
            
            // 智能指针
            // for (auto ser:_cellservers)
            // {
            //     delete ser;
            // }
            _cellservers.clear();
            

            closesocket(_sock);
            WSACleanup();
            _sock = INVALID_SOCKET;
            
            // _clients.clear();
        }
        printf("EasyTcpServer close2\n");
        
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
    virtual void onNetJoin(std::shared_ptr<CellClient>& pClient){
        ++_clientCount;
    }

    //cellserver*6,线程不安全 
    virtual void onNetLeave(std::shared_ptr<CellClient>& pClient){

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
    virtual void onNetMsg(CellServer* pCellServer,std::shared_ptr<CellClient>& pClient,netmsg_DataHeader* header){

        _msgCount++;
    
    }


    virtual void onNetRecv(std::shared_ptr<CellClient>& pClient){

        _recvCount++;
    
    }

};






#endif