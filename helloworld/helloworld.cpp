#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <winsock2.h>
#include <stdio.h>

// #pragma comment(lib,"ws2_32.lib")


using namespace std;

enum CMD{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_ERROR
};

// 消息头
struct DataHeader
{
    short dataLength;
    short cmd;
};

// DataPackage
struct Login: public DataHeader
{
    Login(){
        dataLength = sizeof(Login);
        cmd = CMD_LOGIN;
    }
    char userName[32];
    char userPassWord[32];
};

struct LoginResult: public DataHeader
{
    LoginResult(){
        dataLength = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
        result = 666;
    }
    int result;
};

struct Logout: public DataHeader
{
    Logout(){
        dataLength = sizeof(Logout);
        cmd = CMD_LOGOUT;
    }
    char userName[32];
};

struct LogOutResult: public DataHeader
{
    LogOutResult(){
        dataLength = sizeof(LogOutResult);
        cmd = CMD_LOGOUT_RESULT;
        result = 666;
    }
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
    SOCKET _sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    // 2绑定端口
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = INADDR_ANY;
    if(bind(_sock,(sockaddr*)&_sin,sizeof(sockaddr_in)) == SOCKET_ERROR){
        printf("bind ERROR\n");
    }else{
        printf("bind success\n");
    }
    // 3监听网络
    if(listen(_sock,5) == SOCKET_ERROR){
        printf("listen ERROR\n");
    }else{
        printf("listen success\n");
    }

    // 4等待客户端连接
    sockaddr_in clientAddr = {};
    int nAddLen = sizeof(sockaddr_in);
    SOCKET _cSock = INVALID_SOCKET;
    

    _cSock = accept(_sock,(sockaddr*)&clientAddr,&nAddLen);
    if(INVALID_SOCKET == _cSock){
        printf("ERROR client sock");
    }
    printf("new client:socket = %d,IP = %s \n",(int)_cSock, inet_ntoa(clientAddr.sin_addr));

    // char _recvBuf [128] = {};
    while (true)
    {   
        //5接收客户端请求数据
        DataHeader header = {};
        int nlen = recv(_cSock,(char*)&header,sizeof(DataHeader),0);
        if(nlen <= 0){
            printf("client exit,mission over\n");
            break;
        }
        // printf("client message:%d,message length:%d \n",header.cmd,header.dataLength);

        switch (header.cmd)
        {
        case CMD_LOGIN:
            {
                Login login = {};
                recv(_cSock,(char*)&login+sizeof(DataHeader),sizeof(Login)-sizeof(DataHeader),0);
                printf("client message:CMD_LOGIN,message length:%d,userName:%s,passWord: %s \n",login.dataLength, login.userName,login.userPassWord);
                // 暂时忽略判断用户名密码正确与否
                LoginResult ret;
                send(_cSock,(char*)&ret,sizeof(LoginResult),0);
            }
            break;
        case CMD_LOGOUT:
            {
                Logout logout = {};
                recv(_cSock,(char*)&logout+sizeof(DataHeader),sizeof(Logout)-sizeof(DataHeader),0);
                printf("client message:CMD_LOGOUT,message length:%d,userName:%s \n",logout.dataLength, logout.userName);
                // 暂时忽略判断用户名密码正确与否
                LogOutResult ret;
                send(_cSock,(char*)&ret,sizeof(LogOutResult),0);
            }
            break;
        default:
        header.cmd = CMD_ERROR;
        header.dataLength = 0;
        send(_cSock,(char*)&header,sizeof(DataHeader),0);
            break;
        }
        // //6 处理请求
        // if(0 == strcmp(_recvBuf,"getInfo")){
        //     DataPackage dp = {80,"kkbond"};
        //     // 7向客户端发送一条数据
        //     send(_cSock,(const char *)&dp,sizeof(DataPackage),0);

        // }else{
        //     char msgBuf [] = "surprise motherfucker";
        //     //7 向客户端发数据
        //     // 加上结尾符
        //     send(_cSock,msgBuf,strlen(msgBuf)+1,0);
        // }
        
        
    }
 
    // 8关闭
    closesocket(_sock);

    WSACleanup();
    printf("mission over\n");
    getchar();
    return 0;
}