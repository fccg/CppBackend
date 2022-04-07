#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "Alloctor.h"
#include "EasyTcpServer.hpp"
#include "ShareLib.hpp"

#pragma comment(lib,"ws2_32.lib")


using namespace std;

// bool g_bRun = true;
// void cmdThread(){

//     while (true)
//     {
//         char cmdBuf[256] = {};
//         scanf("%s",cmdBuf);

//         if (0 == strcmp(cmdBuf,"exit")){
//             g_bRun =false;
//             std::Logger::Info("client exit thread \n");
//             break;
//         }else
//         {
//             std::Logger::Info("%s\n",cmdBuf);
//             std::Logger::Info("unsurport command \n");
//         }
        
//     }

// }

class MyServer : public EasyTcpServer
{
private:
    /* data */
public:
   // 客户端加入事件
    virtual void onNetJoin(std::shared_ptr<CellClient>& pClient){
        EasyTcpServer::onNetJoin(pClient);
        // Logger::Info("client <%d> join\n",pClient->sockfd());
    }

    // 客户端退出事件
    virtual void onNetLeave(std::shared_ptr<CellClient>& pClient){
        EasyTcpServer::onNetLeave(pClient);
        // Logger::Info("client <%d> leave\n",pClient->sockfd());              
    }

    //客户端发送消息事件
    virtual void onNetMsg(CellServer* pCellServer,std::shared_ptr<CellClient>& pClient,netmsg_DataHeader* header){

        EasyTcpServer::onNetMsg(pCellServer,pClient,header);
        switch (header->cmd)
            {
            case CMD_LOGIN:
                {
                    pClient->resetDTHeart();
                    netmsg_Login* login = (netmsg_Login*) header;
                    // Logger::Info("client <Socket=%d> message:CMD_LOGIN,message length:%d,userName:%s,passWord: %s \n",cSock,login->dataLength, login->userName,login->userPassWord);
                    // 暂时忽略判断用户名密码正确与否
                    netmsg_LoginResult ret;
                    if(SOCKET_ERROR == pClient->SendData(&ret)){

                        Logger::Info("<Socket %d> send FULL\n",pClient->sockfd());
                        //  放进消息缓冲区
                    }
                    
                    // netmsg_LoginResult* ret = new netmsg_LoginResult();
                    // pCellServer->addSendTask(pClient,ret);
                }
                break;
            case CMD_LOGOUT:
                {
                    // netmsg_Logout logout;
                    netmsg_Logout* logout = (netmsg_Logout*) header;
                    // Logger::Info("client <Socket=%d> message:CMD_LOGOUT,message length:%d,userName:%s \n",cSock,logout->dataLength, logout->userName);
                    // 暂时忽略判断用户名密码正确与否
                    // LogOutResult ret;
                    // SendData(cSock,&ret);
                }
                break;
            case CMD_HEART_BEAT_C2S:{
                pClient->resetDTHeart();
                netmsg_HEART_BEAT_S2C ret;
                pClient->SendData(&ret);
            }
            break;
            default:
                {
                    Logger::Info("server <Socket=%d> unknown message,message length:%d \n",pClient->sockfd(),header->dataLength);
                    // DataHeader ret;
                    // SendData(cSock,&ret);
                }
            
                break;
            }

    }

     // 客户端接收事件
    virtual void onNetRecv(std::shared_ptr<CellClient>& pClient){
        EasyTcpServer::onNetRecv(pClient);            
    }
};




int main()
{
    Logger::Instance().setLogPath("helloServerLog.txt","w");
    MyServer server1;
    server1.InitSocket();
    server1.Bind(nullptr,4567);
    server1.Listen(64);
    server1.Start(6);


    // 启动线程
    // std::thread t1(cmdThread);
    // t1.detach();


    while (true)
    {
        char cmdBuf[256] = {};
        scanf("%s",cmdBuf);

        if (0 == strcmp(cmdBuf,"exit")){
            
            server1.Close();
            Logger::Info("client exit thread \n");
            break;
        }else
        {
            Logger::Info("%s\n",cmdBuf);
            Logger::Info("unsurport command \n");
        }
        
    }


   

    
    // server1.Close();
    // getchar();
    // while (true)
    // {
    //     Sleep(1);
    // }
    
    return 0;
}