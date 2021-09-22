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


using namespace std;

enum CMD{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
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

struct NewUserJoin: public DataHeader
{
    NewUserJoin(){
        dataLength = sizeof(NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        sock = 0;
    }
    int sock;
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


int processor(SOCKET _cSock){

    //5接收客户端请求数据
    // 缓冲区
    char szRecv[1024] = {};
    int nlen = recv(_cSock,szRecv,sizeof(DataHeader),0);
    DataHeader* header = (DataHeader *)szRecv;
    if(nlen <= 0){
        std::printf("server disconnect, mission over \n",_cSock);
        return -1;
    }


    switch (header->cmd)
        {
        case CMD_LOGIN_RESULT:
            {
                recv(_cSock,szRecv + sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
                LoginResult* login = (LoginResult*) szRecv;
                std::printf("server <Socket=%d> message:CMD_LOGIN_RESULT,message length:%d \n",_cSock,header->dataLength);
                // 暂时忽略判断用户名密码正确与否
                // LoginResult ret;
                // send(_cSock,(char*)&ret,sizeof(LoginResult),0);
            }
            break;
        case CMD_LOGOUT_RESULT:
            {
                // Logout logout;
                recv(_cSock,szRecv + sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
                LogOutResult* logout = (LogOutResult*) szRecv;
                std::printf("server <Socket=%d> message:CMD_LOGOUT_RESULT,message length:%d \n",_cSock,header->dataLength);
                // 暂时忽略判断用户名密码正确与否
                // LogOutResult ret;
                // send(_cSock,(char*)&ret,sizeof(LogOutResult),0);
            }
            break;
        case CMD_NEW_USER_JOIN:
            {
                // Logout logout;
                recv(_cSock,szRecv + sizeof(DataHeader),header->dataLength-sizeof(DataHeader),0);
                NewUserJoin* userJoin = (NewUserJoin*) szRecv;
                std::printf("server <Socket=%d> message:CMD_NEW_USER_JOIN,message length:%d \n",_cSock,header->dataLength);
                // 暂时忽略判断用户名密码正确与否
                // LogOutResult ret;b
                // send(_cSock,(char*)&ret,sizeof(LogOutResult),0);
            }
            break;
        }
}

bool g_bRun = true;

void cmdThread(SOCKET sock){

    while (true)
    {
        char cmdBuf[256] = {};
        std::printf("Please enter a command:");
        scanf("%s",cmdBuf);

        if (0 == strcmp(cmdBuf,"exit")){
            g_bRun = false;
            std::printf("client exit thread \n");
            break;
        }else if (0 == strcmp(cmdBuf,"login"))
        {
            Login login;
            strcpy(login.userName,"KKBond");
            strcpy(login.userPassWord,"adad");
            send(sock,(const char*)&login,sizeof(Login),0);
        }else if (0 == strcmp(cmdBuf,"logout"))
        {
            Logout logout;
            strcpy(logout.userName,"KKBond");
            send(sock,(const char*)&logout,sizeof(Logout),0);
        }else
        {
            std::printf("unsurport cmmond");
        }
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
    SOCKET _sock = socket(AF_INET,SOCK_STREAM,0);
    if (INVALID_SOCKET == _sock){
        std::printf("construct error\n");
    }else{
        std::printf("construct success\n");
    }

    // 2连接服务器
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    int ret = connect(_sock,(sockaddr*)&_sin,sizeof(sockaddr_in));
    if(SOCKET_ERROR == ret){
        std::printf("connect to server error\n");
    }else{
        std::printf("connect to server success\n");
    }

    // 启动线程
    std::thread t1(cmdThread,_sock);
    t1.detach();

    while(g_bRun){

        fd_set fdReads;
        FD_ZERO(&fdReads);
        FD_SET(_sock,&fdReads);

        timeval tv = {0,0};
        int ret = select(_sock+1,&fdReads,0,0,&tv);
        if (ret<0){
            std::printf("misssion over");
        }

        if (FD_ISSET(_sock,&fdReads)){
            FD_CLR(_sock,&fdReads);

            if (-1 == processor(_sock)){
                std::printf("mission over2 \n");
                break;
            } 
        }

       
        

        //输入请求
        // char cmdBuf[128] = {};
        // printf("Please enter a command:");
        // scanf("%s",cmdBuf);
        // 4处理请求
        // if(0 == strcmp(cmdBuf,"exit")){
        //     printf("The command entered this time is exit\n");
        //     break;
        // }else if(0 == strcmp(cmdBuf,"login")){
        //     Login login;
        //     strcpy(login.userName,"kkbond");
        //     strcpy(login.userPassWord,"1234");
        //     // 5将命令发送给服务器
        //     send(_sock,(const char *)&login,sizeof(Login),0);
        //     // 接收服务器的返回消息
        //     LoginResult loginRes = {};
        //     recv(_sock,(char *)&loginRes,sizeof(LoginResult),0);
        //     printf("login result:%d \n",loginRes.result);
        // }else if(0 == strcmp(cmdBuf,"logout")){
        //     Logout logout;
        //     strcpy(logout.userName,"kkbond");
        //     // 5将命令发送给服务器
        //     send(_sock,(const char *)&logout,sizeof(Logout),0);
        //     // 接收服务器的返回消息
        //     LogOutResult logoutRes = {};
        //     recv(_sock,(char *)&logoutRes,sizeof(LogOutResult),0);
        //     printf("logout result:%d \n",logoutRes.result);
        // }else{
        //     printf("unsurport commond,plz enter again \n");
        // }

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
    std::printf("mission over1 \n");
    getchar();

    return 0;
}