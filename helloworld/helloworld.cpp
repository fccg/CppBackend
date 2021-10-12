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


int main()
{
    EasyTcpServer server1;
    // server1.InitSocket();
    server1.Bind(nullptr,4567);
    server1.Listen(5);


    // 启动线程
    std::thread t1(cmdThread);
    t1.detach();

    // EasyTcpServer server2;
    // // server.InitSocket();
    // server2.Bind(nullptr,4568);
    // server2.Listen(5);
   
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