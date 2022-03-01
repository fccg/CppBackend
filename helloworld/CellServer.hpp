#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include "ShareLib.hpp"
#include "INetEvent.hpp"
#include "CellClient.hpp"

#include <vector>
#include <map>
#include <thread>



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

        printf("CellServer%d close1\n",_id);
        _taskServer.Close();
        
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
        
        
        printf("CellServer%d close2\n",_id); 
        
        
    }

    //接收缓冲区
    // char _szRecv[RECV_BUFF_SIZE] = {};
    // 接收数据 处理粘包 拆分包
    int RecvData(std::shared_ptr<CellClient> pClient){

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
        while(pClient->getLast() >= sizeof(netmsg_DataHeader)){
            netmsg_DataHeader* header = (netmsg_DataHeader *)pClient->msgBuf();
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
    
    bool OnRun(){

        while(_isRun){

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

            timeval tv = {0,1};
            // int ret = select(maxSock+1,&fdRead,&fdWrite,&fdExp,&tv);
            int ret = select(_maxSock+1,&fdRead,nullptr,nullptr,&tv);

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
            
            ReadData(fdRead);
            CheckTime();
            
        }
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
            iter->second->checkSend(dt);
            iter++;
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
                }else {
					    printf("error. if (iter != _clients.end())\n");
				}
            }
    }

    // 响应网络消息
    virtual void onNetMsg(std::shared_ptr<CellClient> pClient,netmsg_DataHeader* header){
        
        // auto t1 = _tTimer.getElapsedSecond();
        // if(t1 >= 1.0){
        //     printf("time<%1f>,socket<%d>,clients<%d>,recvCount<%d>\n",t1,_sock,_clients.size(),_recvCount);
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

        if(!_isRun){
            _isRun = true;
            std::thread t = std::thread(std::mem_fun(&CellServer::OnRun),this);
            t.detach();
            _taskServer.Start();
        }
        
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
    int _id = -1;
    // 客户端是否有变化
    bool _clients_Change = true;
    // 是否在工作中
    bool _isRun = false;
};

















#endif