#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <winsock2.h>
// #pragma comment(lib,"ws2_32.lib")
#include <stdio.h>
#include <thread>
#include "EasyTcpClient.hpp"


using namespace std;




void cmdThread(EasyTcpClient* client){

    while (true)
    {
        char cmdBuf[256] = {};
        scanf("%s",cmdBuf);

        if (0 == strcmp(cmdBuf,"exit")){
            client->Close();
            std::printf("client exit thread \n");
            break;
        }else if (0 == strcmp(cmdBuf,"login"))
        {
            Login login;
            strcpy(login.userName,"KKBond");
            strcpy(login.userPassWord,"adad");
            client->SendData(&login);
        }else if (0 == strcmp(cmdBuf,"logout"))
        {
            Logout logout;
            strcpy(logout.userName,"KKBond");
            client->SendData(&logout);
        }else
        {
            std::printf("unsurport cmmond");
        }
    }

}

int main()
{
    
    EasyTcpClient client1;
    client1.Connect("127.0.0.1",4567);

    EasyTcpClient client2;
    client2.Connect("127.0.0.1",4567);


    // 启动线程
    std::thread t1(cmdThread,&client1);
    t1.detach();

    std::thread t2(cmdThread,&client2);
    t2.detach();

    while(client1.isRun() || client2.isRun()){
        client1.OnRun();
        client2.OnRun();
    }
    client1.Close();
    client2.Close();
    printf("client exits");

    getchar();

    return 0;
}