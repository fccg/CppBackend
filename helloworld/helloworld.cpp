#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <thread>
#include <vector>
#include <algorithm>
#include "Alloctor.h"
#include "EasyTcpServer.hpp"

#pragma comment(lib,"ws2_32.lib")


using namespace std;

bool g_bRun = true;
void cmdThread(){

    while (true)
    {
        char cmdBuf[256] = {};
        scanf("%s",cmdBuf);

        if (0 == strcmp(cmdBuf,"exit")){
            g_bRun =false;
            std::printf("client exit thread \n");
            break;
        }else
        {
            std::printf("%s\n",cmdBuf);
            std::printf("unsurport command \n");
        }
        // }else if (0 == strcmp(cmdBuf,"login"))
        // {
        //     netmsg_Login login;
        //     strcpy(login.userName,"KKBond");
        //     strcpy(login.userPassWord,"1234");
        //     client->SendData(&login);
        // }else if (0 == strcmp(cmdBuf,"logout"))
        // {
        //     netmsg_Logout logout;
        //     strcpy(logout.userName,"KKBond");
        //     client->SendData(&logout);
        // }
        
    }

}

class MyServer : public EasyTcpServer
{
private:
    /* data */
public:
   // 客户端加入事件
    virtual void onNetJoin(std::shared_ptr<CellClient>& pClient){
        EasyTcpServer::onNetJoin(pClient);
        // printf("client <%d> join\n",pClient->sockfd());
    }

    // 客户端退出事件
    virtual void onNetLeave(std::shared_ptr<CellClient>& pClient){
        EasyTcpServer::onNetLeave(pClient);
        // printf("client <%d> leave\n",pClient->sockfd());              
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
                    // printf("client <Socket=%d> message:CMD_LOGIN,message length:%d,userName:%s,passWord: %s \n",cSock,login->dataLength, login->userName,login->userPassWord);
                    // 暂时忽略判断用户名密码正确与否
                    netmsg_LoginResult ret;
                    pClient->SendData(&ret);
                    // netmsg_LoginResult* ret = new netmsg_LoginResult();
                    // pCellServer->addSendTask(pClient,ret);
                }
                break;
            case CMD_LOGOUT:
                {
                    // netmsg_Logout logout;
                    netmsg_Logout* logout = (netmsg_Logout*) header;
                    // printf("client <Socket=%d> message:CMD_LOGOUT,message length:%d,userName:%s \n",cSock,logout->dataLength, logout->userName);
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
                    printf("server <Socket=%d> unknown message,message length:%d \n",pClient->sockfd(),header->dataLength);
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
    MyServer server1;
    server1.InitSocket();
    server1.Bind(nullptr,4567);
    server1.Listen(5);
    server1.Start(6);


    // 启动线程
    std::thread t1(cmdThread);
    t1.detach();

   
    while (g_bRun)
    {

        server1.OnRun();
        // server2.OnRun();

    }

    server1.Close();
    // server1.Close();
    getchar();
    return 0;
}