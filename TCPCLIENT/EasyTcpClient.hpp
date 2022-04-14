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
#include "CellNetWork.hpp"
#include "MessageHeader.hpp"
#include "CellClient.hpp"
#include "Logger.hpp"

using namespace std;


class EasyTcpClient
{

    
protected:
    bool _isConnect = false;
    CellClient* pClient = nullptr;

public:
    EasyTcpClient(/* args */){
        
        _isConnect = false;
    }
    // 虚析构函数
    virtual ~EasyTcpClient(){
        Close();
    }

    // 初始化socket
    void InitSocket(){
        //启动win sock 2.x的环境 
        CellNetWork::Init();

        if(pClient){
            Logger::Info("<socket=%d> close old connect",pClient->sockfd());
            Close();
        }

        // 1建立一个socket套接字
        SOCKET _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if (INVALID_SOCKET == _sock){
            Logger::Info("construct error\n");
        }else{
            pClient = new CellClient(_sock);
            // Logger::Info("construct <socket=%d> success\n",_sock);
        }


    }
    // 连接服务器
    int Connect(const char* ip,unsigned short port){
        if(!pClient){
            InitSocket();
        }

        // 2连接服务器
        sockaddr_in _sin = {};
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port);
        _sin.sin_addr.S_un.S_addr = inet_addr(ip);
        int ret = connect(pClient->sockfd(),(sockaddr*)&_sin,sizeof(sockaddr_in));
        if(SOCKET_ERROR == ret){
            Logger::Info("<socket=%d>connect to server<%s:%d> error\n",pClient->sockfd(),ip,port);
        }else{
            _isConnect = true;
            // Logger::Info("<socket=%d>connect to server success\n",_sock);
        }
        return ret;


    }
    // 关闭socket
    void Close(){
        //启动win sock 2.x的环境
        if (pClient){
            // 7关闭
            delete pClient;
            pClient = nullptr;
        }
        _isConnect = false;

    }

    // 发送数据

    // 接收数据

    // 监听网络消息
    bool OnRun(){

        if (isRun())
        {
            SOCKET _sock = pClient->sockfd();

            fd_set fdRead;
            FD_ZERO(&fdRead);
            FD_SET(_sock,&fdRead);
            fd_set fdWrite;
            FD_ZERO(&fdWrite);

            int ret = 0;
            timeval tv = {0,1};

            if (pClient->readyWrite())
            {
                
                FD_SET(_sock,&fdWrite);
                ret = select(_sock+1,&fdRead,&fdWrite,nullptr,&tv);
            }else{
                ret = select(_sock+1,&fdRead,nullptr,nullptr,&tv);
            }
            
            

            
            
            if (ret<0){
                Logger::Info("<socket=%d>OnRun.select misssion over1",_sock);
                Close();
                return false;
            }

            if (FD_ISSET(_sock,&fdRead)){
                // FD_CLR(_sock,&fdRead);

                if (-1 == RecvData(_sock)){
                    Logger::Info("<socket=%d>OnRun.select RecvData over \n",_sock);
                    Close();
                    return false;
                } 
            }

            if (FD_ISSET(_sock,&fdWrite)){
                // FD_CLR(_sock,&fdRead);

                if (-1 == pClient->SendDataIM()){
                    Logger::Info("<socket=%d>OnRun.select SendDataIM over \n",_sock);
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
        return pClient && _isConnect;
    }

#ifndef RECV_BUFF_SIZE
// 缓冲区最小单元大小
#define RECV_BUFF_SIZE 10240
#endif


    // 接收数据 之后要处理粘包，拆分包
    int RecvData(SOCKET cSock){

        //5接收客户端请求数据
        // 缓冲区
        int nlen = pClient->RecvData();
        
        if(nlen > 0){
            // 循环判断是否有数据需要处理
            while(pClient->hasMsg()){

                // 处理网络消息
                onNetMsg(pClient->front_msg());
                // 将处理完的消息（最前面的一条数据）移除缓冲区
                pClient->pop_front_msg();

            }
        }

        return nlen;
        
           
    }

    // 处理网络消息
    virtual void onNetMsg(netmsg_DataHeader* header) = 0;

    // 发送数据
    int SendData(netmsg_DataHeader* header){

        
        return pClient->SendData(header);;
        
    }

};


#endif

