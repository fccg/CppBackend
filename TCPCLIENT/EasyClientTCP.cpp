#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


// #pragma comment(lib,"ws2_32.lib")
#include <thread>
#include "EasyTcpClient.hpp"


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
            std::printf("%s\n",cmdBuf);
            std::printf("unsurport command \n");
        }
    }

}

// 客户端数量
const int cCount = 120;
// 线程数量
const int tcount = 6;
//客户端数组
EasyTcpClient* client[cCount];

void sendThread(int id){

    int cnt = cCount / tcount;
    int begin = (id-1)*cnt;
    int end = id*cnt;

    for(int i = begin;i < end;i++){

        client[i] = new EasyTcpClient();
        // client[i]->Connect("127.0.0.1",4567);
    }
    for(int i = begin;i < end;i++){
    
        // client[i] = new EasyTcpClient();
        client[i]->Connect("127.0.0.1",4567);
        printf("thread<%d>,Connect=%d\n", id,i);
    }


    

    Login login;
    strcpy(login.userName,"KKBond");
    strcpy(login.userPassWord,"1234");
    while(g_bRun){
        
        for(int i = begin;i < end;i++){
            
            client[i]->SendData(&login);
            // client[i]->OnRun();

        }
        // client1.OnRun();
    }

    for(int i = begin;i < end;i++){
        client[i]->Close();
    }

}


int main()
{

    // 启动UI线程
    std::thread t1(cmdThread);
    t1.detach();

    

    //启动发送线程
    for (size_t i = 0; i < tcount; i++)
    {
        std::thread t1(sendThread,i+1);
        t1.detach();
    }
    
    while (g_bRun)
    {
        Sleep(1);
    }
    
    
    
    printf("client exits");
    return 0;
}