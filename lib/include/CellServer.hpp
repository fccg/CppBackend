#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include "ShareLib.hpp"
#include "INetEvent.hpp"
#include "CellClient.hpp"

#include <vector>
#include <map>
#include <thread>

#include "CellSemaphore.hpp"


// 预声明
class CellServer;



// 网络消息接受处理服务类
class CellServer{

public:

    CellServer(int id){
        _id = id;
    
        _pNetEvent = nullptr;
    }
    ~CellServer(){
        Close();
        
    }

    void setEventObj(INetEvent* event){
        _pNetEvent = event;
    }

    
    

    // 关闭socket
    void Close(){


        
        Logger::Info("CellServer%d close1\n",_id);
        _taskServer.Close();
        _thread.Close();
        
        // 不需要，iter.second是智能指针
        // for (auto iter:_clients)
        // {
        //     closesocket(iter.second->sockfd());
        //     // delete iter.second;
        // }
        // 应该由easytcp server去关闭
        // closesocket(_sock);
        _clients.clear();
        _clientsBuff.clear();
        
        
        Logger::Info("CellServer%d close2\n",_id); 
        
        
        
        
    }

    //接收缓冲区
    // char _szRecv[RECV_BUFF_SIZE] = {};
    // 接收数据 处理粘包 拆分包
    int RecvData(std::shared_ptr<CellClient> pClient){

        //5接收客户端请求数据
        // 缓冲区
        int nlen = pClient->RecvData();
        
        if(nlen <= 0){
            //  
            // Logger::Info("client <Socket=%d> exit,mission finish \n",pClient->sockfd());
            return -1;
        }
        // 出发《接收网络数据》事件
        _pNetEvent->onNetRecv(pClient);
        
        // 循环判断是否有数据需要处理
        while(pClient->hasMsg()){

            // 处理网络消息
            onNetMsg(pClient,pClient->front_msg());
            // 将处理完的消息（最前面的一条数据）移除缓冲区
            pClient->pop_front_msg();

        }

        return 0;
        
    }
    
    // 处理网络消息
    
    void OnRun(CellThread* t){

        while(t->isRun()){

            if(_clientsBuff.size() > 0){
                
                // 从缓冲队列中取出数据
                std::lock_guard<std::mutex> mymutex(_mutex);
                for(auto pClient:_clientsBuff){
                    _clients[pClient->sockfd()] = pClient;
                    if(_pNetEvent){
                        _pNetEvent->onNetJoin(pClient);
                    }
                }
                _clientsBuff.clear();
                _clients_Change = true;
                
            }
            // 如果没有要处理的客户端就跳过
            if (_clients.empty())
            {
                std::chrono::milliseconds dua(1);
                std::this_thread::sleep_for(dua);
                // 保持更新旧的时间戳
                time_t _oldTime = Timestick::getNowTimeInMilliSec();
                continue;
            }
            

            // 伯克利套接字
            fd_set fdRead;
            fd_set fdWrite;
            // fd_set fdExp;
               
            
            // 主线程不需要在此轮询
            // FD_SET(_sock,&fdRead);
            // FD_SET(_sock,&fdWrite);
            // FD_SET(_sock,&fdExp);
            
            // 将描述符（socket）加入集合
            if (_clients_Change)
            {   
                _clients_Change = false;

                // 清理集合
                FD_ZERO(&fdRead);
                // FD_ZERO(&fdWrite);
                // FD_ZERO(&fdExp);

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

            memcpy(&fdWrite,&_fdRead_bak,sizeof(fd_set));
            // memcpy(&fdExp,&_fdRead_bak,sizeof(fd_set));
           
            
            // nfds是整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量
            // 即是所有文件描述符最大值+1，在windows中这个参数可以为0

            timeval tv = {0,1};
            int ret = select(_maxSock+1,&fdRead,&fdWrite,nullptr,&tv);

            if(ret < 0){
                Logger::Info("cell server error,End select\n");
                t->SelfExit();
                break;
            }
            // else if (ret == 0)
            // {
            //     continue;
            // }
            
            // // 判断描述符（sock）是否在集合中
            // if (FD_ISSET(_sock,&fdRead)){
            //     FD_CLR(_sock,&fdRead);
            //     Acccept();
            //     return true;
            // }
            
            ReadData(fdRead);
            WriteData(fdWrite);
            // WriteData(fdExp);
            // Logger::Info("CellServer OnRun select:,fdRead=%d,fdWrite=%d\n",fdRead.fd_count,fdWrite.fd_count);
            CheckTime();
            
        }
        Logger::Info("CellServer OnRun exit\n");
        // 放进thread的stop任务中去做了
        // _clients.clear();
        // _clientsBuff.clear();
        
    }

    
    void CheckTime(){

        
        auto tNow = Timestick::getNowTimeInMilliSec();
        auto dt = tNow - _oldTime;
        _oldTime = tNow;
        for (auto iter = _clients.begin(); iter != _clients.end();)
        {
            // 心跳检测
            if(iter->second->checkHeart(dt)){
                if (_pNetEvent)
                {
                    _pNetEvent->onNetLeave(iter->second);
                }
                _clients_Change = true;
                auto iterOld = iter++;
                // delete _clients[i];
                _clients.erase(iterOld);
                continue;
            }
            // 定时发送检测
            // iter->second->checkSend(dt);
            iter++;
        }
        
    }

    void WriteData(fd_set& fdWrite){

        for (int i = 0; i < fdWrite.fd_count; i++)
            {
                auto iter = _clients.find(fdWrite.fd_array[i]);
                if (iter != _clients.end())
                {
                    if(-1 == iter->second->SendDataIM()){
                        
                        if (_pNetEvent)
                        {
                            _pNetEvent->onNetLeave(iter->second);
                        }
                        _clients_Change = true;
                        // closesocket(iter->first);
                        // delete _clients[i];
                        _clients.erase(iter);
                    }
                }
            }
    }




    void ReadData(fd_set& fdRead){

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
                        // closesocket(iter->first);
                        // delete _clients[i];
                        _clients.erase(iter);
                    }
                }
            }
    }

    // 响应网络消息
    virtual void onNetMsg(std::shared_ptr<CellClient> pClient,netmsg_DataHeader* header){
        
        // auto t1 = _tTimer.getElapsedSecond();
        // if(t1 >= 1.0){
        //     Logger::Info("time<%1f>,socket<%d>,clients<%d>,recvCount<%d>\n",t1,_sock,_clients.size(),_recvCount);
        //     _recvCount =0;
        //     _tTimer.update();
        // }
        _pNetEvent->onNetMsg(this,pClient, header);

    }

    void addClient(std::shared_ptr<CellClient> pClient) {

        std::lock_guard<std::mutex> mymutex(_mutex);
        // _mutex.lock();
        _clientsBuff.push_back(pClient);
        // _mutex.unlock();
        

    }

    void Start(){

      
        _taskServer.Start();
       
        _thread.Start(
            // Create
            nullptr
            // Run
            ,[this](CellThread* t){
                OnRun(t);
            }
            // Close
            ,[this](CellThread* t){
                _clients.clear();
                _clientsBuff.clear();
            });
        
    }

    size_t getClientCount(){
        return _clients.size() + _clientsBuff.size();
    }

    // void addSendTask(std::shared_ptr<CellClient> pClient,netmsg_DataHeader* header){

    //     // auto task = std::make_shared<sendMsg2Client>(pClient,header);

    //     _taskServer.addTask([pClient,header](){
    //         pClient->SendData(header); 
    //         delete header;
    //     });
    // }


public:
    int _id = -1;

private:
    //正式客户队列
    std::map<SOCKET,std::shared_ptr<CellClient>> _clients;
    //客户缓冲队列
    std::vector<std::shared_ptr<CellClient>> _clientsBuff;
    // 缓冲队列锁
    std::mutex _mutex;
    // 网络事件对象
    INetEvent* _pNetEvent;
    // 
    CellTaskServer _taskServer;

    // 备份socket fd_set
    fd_set _fdRead_bak;
    
    SOCKET _maxSock;
    // 旧时间戳
    time_t _oldTime = Timestick::getNowTimeInMilliSec();
    
    // 客户端是否有变化
    bool _clients_Change = true;

    CellThread _thread;
    // // 是否在工作中
    // bool _isRun = false;

    // CellSemaphore _sem;
};

















#endif