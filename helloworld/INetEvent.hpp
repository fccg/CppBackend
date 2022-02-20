#ifndef _I_NET_EVENT_HPP_
#define _I_NET_EVENT_HPP_


#include "ShareLib.hpp"
#include "CellClient.hpp"


// 预声明
class CellServer;

// 网络事件接口
class INetEvent
{
private:
    /* data */
public:
    // 客户端加入事件
    virtual void onNetJoin(std::shared_ptr<CellClient>& pClient) = 0;

    // 客户端退出事件
    virtual void onNetLeave(std::shared_ptr<CellClient>& pClient) = 0;

    //客户端发送消息事件
    virtual void onNetMsg(CellServer* pCellServer,std::shared_ptr<CellClient>& pClient,netmsg_DataHeader* header) = 0;

    //recv事件
    virtual void onNetRecv(std::shared_ptr<CellClient>& pClient) = 0;

};



















#endif