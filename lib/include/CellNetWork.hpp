#ifndef _CELL_NETWORK_HPP_
#define _CELL_NETWORK_HPP_


#include "ShareLib.hpp"

class CellNetWork
{
private:

    CellNetWork(/* args */)
    {
        #ifdef _WIN32
            WORD ver = MAKEWORD(2,2);
            WSADATA dat;
            WSAStartup(ver,&dat);
        #endif
    }
    ~CellNetWork()
    {
        WSACleanup();

    }



public:

    static void Init()
    {
        static CellNetWork NW;
    }





};









#endif
