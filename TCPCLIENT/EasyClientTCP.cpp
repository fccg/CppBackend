#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS


// #pragma comment(lib,"ws2_32.lib")
#include <thread>
#include <atomic>
#include <iostream>

#include "EasyTcpClient.hpp"
#include "CELLTimestamp.hpp"


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
const int cCount = 6666;
// 线程数量
const int tcount = 6;
//客户端数组
EasyTcpClient* client[cCount];
// 发送计数
std::atomic_int sendCount(0);
// 线程技术计数
std::atomic_int readyCount(0);



void sendThread(int id){
    printf("thread<%d>,start\n", id);

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
        
    }

    printf("thread<%d>,Connect<beigin=%d,end=%d>\n", id,begin,end);


     
    // 等待所有线程准备好发数据
    std::chrono::milliseconds dua(10);
    std::this_thread::sleep_for(dua); 
    

    Login login[10];

    for (size_t i = 0; i < 10; i++)
    {
        strcpy(login[i].userName,"KKBond");
        strcpy(login[i].userPassWord,"1234");
    }
    
    const int nLen = sizeof(login);
    while(g_bRun){
        
        for(int i = begin;i < end;i++){
            
            if(SOCKET_ERROR != client[i]->SendData(login,nLen)){
                sendCount++;
            }
            client[i]->OnRun();

        }
    }

    for(int i = begin;i < end;i++){
        client[i]->Close();
        delete client[i];
    }
    printf("thread<%d>,Exit<beigin=%d,end=%d>\n", id,begin,end);

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


    CELLTimestamp tTime;
    
    while (g_bRun)
    {
        auto t= tTime.getElapsedSecond();
        if (t >= 1.0)
        {
            // printf("thead<%d>,clients<%d>,time<%1f>,sendCount<%d>\n",tcount,cCount,t,static_cast<int>(sendCount));
            cout<< "thead: " << "<" << tcount << ">" << "clients: " << "<" << cCount << ">" << "time: " << "<" << t << ">" <<"sendCount: " << "<" << static_cast<int>(sendCount/t) << ">" << endl;
            sendCount = 0;
            tTime.update();

        }
        Sleep(1);
    }
    
    
    
    printf("client exits");
    return 0;
}