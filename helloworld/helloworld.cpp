#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "EasyTcpServer.hpp"

#pragma comment(lib,"ws2_32.lib")


using namespace std;


int main()
{
    EasyTcpServer server1;
    // server.InitSocket();
    server1.Bind(nullptr,4567);
    server1.Listen(5);

    EasyTcpServer server2;
    // server.InitSocket();
    server2.Bind(nullptr,4568);
    server2.Listen(5);
   
    while (server1.isRun() || server2.isRun())
    {

        server1.OnRun();
        server2.OnRun();

    }

    server1.Close();
    server1.Close();
    getchar();
    return 0;
}