#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <winsock2.h>
// #pragma comment(lib,"ws2_32.lib")
#include <stdio.h>


using namespace std;

struct DataPackage
{
    int age;
    char name[32];
};


int main()
{
    // vector<string> msg {"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};
    // int i = 0;
    // for (const string& word : msg)
    // {
    //     cout << word << " ";
    //     ++i;
    // }
    // cout << endl;



    WORD ver = MAKEWORD(2,2);
    WSADATA dat;
    WSAStartup(ver,&dat);

    // 1建立一个socket套接字
    SOCKET _sock = socket(AF_INET,SOCK_STREAM,0);
    if (INVALID_SOCKET == _sock){
        printf("construct error\n");
    }else{
        printf("construct success\n");
    }

    // 2连接服务器
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int ret = connect(_sock,(sockaddr*)&_sin,sizeof(sockaddr_in));
    if(SOCKET_ERROR == ret){
        printf("connect to server error\n");
    }else{
        printf("connect to server success\n");
    }

    
    while(true){
        //输入请求
        char cmdBuf[128] = {};
        printf("Please enter a command:");
        scanf("%s",cmdBuf);
        // 4处理请求
        if(0 == strcmp(cmdBuf,"exit")){
            printf("The command entered this time is exit\n");
            break;
        }else{
            // 5将命令发送给服务器
            send(_sock,cmdBuf,strlen(cmdBuf)+1,0);
        }

        // 6接收服务器信息
        char recvBuf[128] = {};
        int nlen = recv(_sock,recvBuf,128,0);
        if(nlen > 0 ){
            DataPackage* dp = (DataPackage*)recvBuf;
            printf("server data:age = %d,name = %s\n",dp->age,dp->name);
        }
    }

    // 7关闭
    closesocket(_sock);

    WSACleanup();
    printf("mission over\n");
    getchar();

    return 0;
}