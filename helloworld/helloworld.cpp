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
        //     Login login;
        //     strcpy(login.userName,"KKBond");
        //     strcpy(login.userPassWord,"1234");
        //     client->SendData(&login);
        // }else if (0 == strcmp(cmdBuf,"logout"))
        // {
        //     Logout logout;
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
    virtual void onNetJoin(ClientSocket* pClient){
        ++_clientCount;
        // printf("client <%d> join\n",pClient->sockfd());
    }

    // 客户端退出事件
    virtual void onNetLeave(ClientSocket* pClient){
        --_clientCount;
        // printf("client <%d> leave\n",pClient->sockfd());              
    }

    //客户端发送消息事件
    virtual void onNetMsg(ClientSocket* pClient,DataHeader* header){
        ++_msgCount;
        switch (header->cmd)
            {
            case CMD_LOGIN:
                {
                    Login* login = (Login*) header;
                    // printf("client <Socket=%d> message:CMD_LOGIN,message length:%d,userName:%s,passWord: %s \n",cSock,login->dataLength, login->userName,login->userPassWord);
                    // 暂时忽略判断用户名密码正确与否
                    LoginResult ret;
                    pClient->SendData(&ret);
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
                    printf("server <Socket=%d> unknown message,message length:%d \n",pClient->sockfd(),header->dataLength);
                    // DataHeader ret;
                    // SendData(cSock,&ret);
                }
            
                break;
            }

    }

     // 客户端退出事件
    virtual void onNetRecv(ClientSocket* pClient){
        ++_recvCount;
        // printf("client <%d> leave\n",pClient->sockfd());              
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