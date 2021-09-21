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

std::vector<SOCKET> g_clents;


int processor(SOCKET _cSock){

    //5接收客户端请求数据
    // 缓冲区
    char szRecv[1024] = {};
    int nlen = recv(_cSock,szRecv,sizeof(DataHeader),0);
    DataHeader* header = (DataHeader *)szRecv;
    if(nlen <= 0){
        printf("client exit,mission over \n");
        return -1;
    }


    switch (header->cmd)
        {
        case CMD_LOGIN:
            {
                recv(_cSock,szRecv + sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
                Login* login = (Login*) szRecv;
                printf("client message:CMD_LOGIN,message length:%d,userName:%s,passWord: %s \n",login->dataLength, login->userName,login->userPassWord);
                // 暂时忽略判断用户名密码正确与否
                LoginResult ret;
                send(_cSock,(char*)&ret,sizeof(LoginResult),0);
            }
            break;
        case CMD_LOGOUT:
            {
                // Logout logout;
                recv(_cSock,szRecv + sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
                Logout* logout = (Logout*) szRecv;
                printf("client message:CMD_LOGOUT,message length:%d,userName:%s \n",logout->dataLength, logout->userName);
                // 暂时忽略判断用户名密码正确与否
                LogOutResult ret;
                send(_cSock,(char*)&ret,sizeof(LogOutResult),0);
            }
            break;
        default:
            {
                DataHeader header = {0,CMD_ERROR};
                send(_cSock,(char*)&header,sizeof(DataHeader),0);
            }
        
            break;
        }


}


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
    // sockaddr_in clientAddr = {};
    // int nAddLen = sizeof(sockaddr_in);
    // SOCKET _cSock = INVALID_SOCKET;
    

    // _cSock = accept(_sock,(sockaddr*)&clientAddr,&nAddLen);
    // if(INVALID_SOCKET == _cSock){
    //     printf("ERROR client sock\n");
    // }
    // printf("new client:socket = %d,IP = %s \n",(int)_cSock, inet_ntoa(clientAddr.sin_addr));

    // char _recvBuf [128] = {};
    while (true)
    {   
        fd_set fdRead;
        fd_set fdWrite;
        fd_set fdExp;

        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExp);

        FD_SET(_sock,&fdRead);
        FD_SET(_sock,&fdWrite);
        FD_SET(_sock,&fdExp);
        
        // 因为要调函数，所以用减减，这样能减少调用的次数
        for (int i = (int)g_clents.size()-1; i >= 0 ; i--)
        {
            FD_SET(g_clents[i],&fdRead);
        }
        
        
        // nfds是整数值，是指fd_set集合中所有描述符（socket）的范围，而不是数量
        // 即是所有文件描述符最大值+1，在windows中这个参数可以为0

        timeval tv = {0,0};
        int ret = select(_sock+1,&fdRead,&fdWrite,&fdExp,&tv);

        if(ret < 0){
            printf("End select\n");
            break;
        }

        if (FD_ISSET(_sock,&fdRead)){
            FD_CLR(_sock,&fdRead);
            // 4等待客户端连接
            sockaddr_in clientAddr = {};
            int nAddLen = sizeof(sockaddr_in);
            SOCKET _cSock = INVALID_SOCKET;
            

            _cSock = accept(_sock,(sockaddr*)&clientAddr,&nAddLen);
            if(INVALID_SOCKET == _cSock){
                printf("ERROR client sock\n");
            }
            g_clents.push_back(_cSock);
            printf("new client:socket = %d,IP = %s \n",(int)_cSock, inet_ntoa(clientAddr.sin_addr));
        }

        for (size_t i = 0; i < fdRead.fd_count; i++)
        {
            if(-1 == processor(fdRead.fd_array[i])){
                auto iter = find(g_clents.begin(),g_clents.end(),fdRead.fd_array[i]);
                if(iter != g_clents.end()){
                    g_clents.erase(iter);
                }
            }
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

    for (size_t i = g_clents.size()-1; i >= 0; i--)
    {
        closesocket(g_clents[i]);
    }
    
    
    closesocket(_sock);
    WSACleanup();
    printf("mission over\n");
    getchar();
    return 0;
}