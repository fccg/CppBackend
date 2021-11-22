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


int main()
{
    
    const int cCount = 100;

    EasyTcpClient* client[cCount];
    for(int i = 0;i < cCount;i++){
        if (!g_bRun)
        {
            return 0;
        }
        client[i] = new EasyTcpClient();
        // client[i]->Connect("127.0.0.1",4567);
    }
    for(int i = 0;i < cCount;i++){
        if (!g_bRun)
        {
            return 0;
        }
        // client[i] = new EasyTcpClient();
        client[i]->Connect("127.0.0.1",4567);
        printf("Connect=%d\n",i);
    }


    // 启动线程
    std::thread t1(cmdThread);
    t1.detach();

    Login login;
    strcpy(login.userName,"KKBond");
    strcpy(login.userPassWord,"1234");
    while(g_bRun){
        
        for(int i = 0;i < cCount;i++){
            
            client[i]->SendData(&login);
            // client[i]->OnRun();

        }
        // client1.OnRun();
    }

    for(int i = 0;i < cCount;i++){
        client[i]->Close();
    }
    
    printf("client exits");
    return 0;
}