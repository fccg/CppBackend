#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


// #pragma comment(lib,"ws2_32.lib")
#include <thread>
#include <atomic>
#include <iostream>

#include "EasyTcpClient.hpp"
#include "CELLTimestamp.hpp"
#include "CellStream.hpp"


using namespace std;


class myClient : public EasyTcpClient
{
public:
    // 处理网络消息
    virtual void onNetMsg(netmsg_DataHeader* header){

        switch (header->cmd)
            {
            case CMD_LOGIN_RESULT:
                {
                    
                    netmsg_LoginResult* login = (netmsg_LoginResult*) header;
                    // Logger::Info("server <Socket=%d> message:CMD_LOGIN_RESULT,message length:%d \n",_sock,login->dataLength);
                    // 暂时忽略判断用户名密码正确与否
                    // netmsg_LoginResult ret;
                    // send(_cSock,(char*)&ret,sizeof(netmsg_LoginResult),0);
                }
                break;
            case CMD_LOGOUT_RESULT:
                {
                    // Logout logout;
                    netmsg_LogOutResult* logout = (netmsg_LogOutResult*) header;
                    // Logger::Info("server <Socket=%d> message:CMD_LOGOUT_RESULT,message length:%d \n",_sock,logout->dataLength);
                    // 暂时忽略判断用户名密码正确与否
                    // netmsg_LogOutResult ret;
                    // send(_cSock,(char*)&ret,sizeof(netmsg_LogOutResult),0);
                }
                break;
            case CMD_NEW_USER_JOIN:
                {
                    // Logout logout;
                    netmsg_NewUserJoin* userJoin = (netmsg_NewUserJoin*) header;
                    // Logger::Info("server <Socket=%d> message:CMD_NEW_USER_JOIN,message length:%d \n",_sock,userJoin->dataLength);
                    // 暂时忽略判断用户名密码正确与否
                    // netmsg_LogOutResult ret;b
                    // send(_cSock,(char*)&ret,sizeof(netmsg_LogOutResult),0);
                }
                break;
                case CMD_ERROR:{
                    Logger::Info("server <Socket=%d> message:CMD_ERROR,message length:%d \n",pClient->sockfd(),header->dataLength);
                }
                default:{
                    Logger::Info("server <Socket=%d> unknown message,message length:%d \n",pClient->sockfd(),header->dataLength);
                }
            }

    }
};












bool g_bRun = true;

void cmdThread(){

    while (true)
    {
        char cmdBuf[256] = {};
        scanf("%s",cmdBuf);

        if (0 == strcmp(cmdBuf,"exit")){
            g_bRun =false;
            Logger::Info("client exit thread \n");
            break;
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
        else
        {
            Logger::Info("%s\n",cmdBuf);
            Logger::Info("unsurport command \n");
        }
    }

}

// 客户端数量
const int cCount = 12;
// 线程数量
const int tcount = 1;
//客户端数组
EasyTcpClient* client[cCount];
// 发送计数
std::atomic_int sendCount(0);
// 线程技术计数
std::atomic_int readyCount(0);


// 发送线程
void recvThread(int begin,int end){

    while(g_bRun){
        
        for(int i = begin;i < end;i++){
            
            client[i]->OnRun();

        }
    }
}



// 发送线程
void sendThread(int id){
    Logger::Info("thread<%d>,start\n", id);

    int cnt = cCount / tcount;
    int begin = (id-1)*cnt;
    int end = id*cnt;

    for(int i = begin;i < end;i++){

        client[i] = new myClient();
        // client[i]->Connect("127.0.0.1",4567);
    }
    for(int i = begin;i < end;i++){
    
        // client[i] = new EasyTcpClient();
        client[i]->Connect("127.0.0.1",4567);
        
    }

    Logger::Info("thread<%d>,Connect<beigin=%d,end=%d>\n", id,begin,end);


     
    // 等待所有线程准备好发数据
    std::chrono::milliseconds dua(40);
    std::this_thread::sleep_for(dua); 

    // 启动接收线程
    std::thread t1(recvThread,begin,end);
    t1.detach();
    

    netmsg_Login login[1];

    for (size_t i = 0; i < 10; i++)
    {
        strcpy(login[i].userName,"KKBond");
        strcpy(login[i].userPassWord,"1234");
    }
    
    const int nLen = sizeof(login);
    while(g_bRun){
        
        for(int i = begin;i < end;i++){
            
            if(SOCKET_ERROR != client[i]->SendData(login)){
                sendCount++;
            }

        }
        // std::chrono::milliseconds dua(200);
        // std::this_thread::sleep_for(dua);
    }

    for(int i = begin;i < end;i++){
        client[i]->Close();
        delete client[i];
    }
    Logger::Info("thread<%d>,Exit<beigin=%d,end=%d>\n", id,begin,end);

}


int main()
{

    Logger::Instance().setLogPath("helloClientLog.txt","w");

    CellStream s;
    s.writeInt8(66);
    s.writeInt16(66);
    s.writeInt32(66);
    s.writeFloat(66);
    s.writeDouble(66);
    char* str = "hello";
    s.writeArray(str,strlen(str));
    char a[] = "world";
    s.writeArray(a,strlen(a));
    int b[] = {1,2,3,4,5};
    s.writeArray(b,5);







    // 启动UI线程
    // std::thread t1(cmdThread);
    // t1.detach();

    

    // //启动发送线程
    // for (size_t i = 0; i < tcount; i++)
    // {
    //     std::thread t1(sendThread,i+1);
    //     t1.detach();
    // }


    // CELLTimestamp tTime;
    
    // while (g_bRun)
    // {
    //     auto t= tTime.getElapsedSecond();
    //     if (t >= 1.0)
    //     {
    //         // Logger::Info("thead<%d>,clients<%d>,time<%1f>,sendCount<%d>\n",tcount,cCount,t,static_cast<int>(sendCount));
    //         cout<< "thead: " << "<" << tcount << ">" << "clients: " << "<" << cCount << ">" << "time: " << "<" << t << ">" <<"sendCount: " << "<" << static_cast<int>(sendCount/t) << ">" << endl;
    //         sendCount = 0;
    //         tTime.update();

    //     }
    //     Sleep(1);
    // }
    
    
    
    Logger::Info("client exits");
    return 0;
}