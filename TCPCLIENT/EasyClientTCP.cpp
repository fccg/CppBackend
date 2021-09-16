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

enum CMD{
    CMD_LOGIN,
    CMD_LOGOUT,
    CMD_ERROR
};

// 消息头
struct DataHeader
{
    short dataLength;
    short cmd;
};

// DataPackage
struct Login
{
    char userName[32];
    char userPassWord[32];
};

struct LoginResult
{
    int result;
};

struct Logout
{
    char userName[32];
};

struct LogOutResult
{
    int result;
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
        }else if(0 == strcmp(cmdBuf,"login")){
            Login login = {"kkbond","123456"};
            DataHeader dh = {sizeof(Login),CMD_LOGIN};
            // 5将命令发送给服务器
            send(_sock,(const char *)&dh,sizeof(DataHeader),0);
            send(_sock,(const char *)&login,sizeof(Login),0);
            // 接收服务器的返回消息
            DataHeader dh_recv = {};
            LoginResult loginRes = {};
            recv(_sock,(char *)&dh_recv,sizeof(DataHeader),0);
            recv(_sock,(char *)&loginRes,sizeof(DataHeader),0);
            printf("login result:%d \n",loginRes.result);
        }else if(0 == strcmp(cmdBuf,"logout")){
            Logout logout = {"kkbond"};
            DataHeader dh = {sizeof(Logout),CMD_LOGOUT};
            // 5将命令发送给服务器
            send(_sock,(const char *)&dh,sizeof(DataHeader),0);
            send(_sock,(const char *)&logout,sizeof(Logout),0);
            // 接收服务器的返回消息
            DataHeader dh_recv = {};
            LogOutResult logoutRes = {};
            recv(_sock,(char *)&dh_recv,sizeof(DataHeader),0);
            recv(_sock,(char *)&logoutRes,sizeof(LogOutResult),0);
            printf("logout result:%d \n",logoutRes.result);
        }else{
            printf("unsurport commond,plz enter again \n");
        }

        // // 6接收服务器信息
        // char recvBuf[128] = {};
        // int nlen = recv(_sock,recvBuf,128,0);
        // if(nlen > 0 ){
        //     DataPackage* dp = (DataPackage*)recvBuf;
        //     printf("server data:age = %d,name = %s\n",dp->age,dp->name);
        // }
    }

    // 7关闭
    closesocket(_sock);

    WSACleanup();
    printf("mission over\n");
    getchar();

    return 0;
}