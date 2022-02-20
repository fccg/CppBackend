#ifndef _MessageHeader_hpp_
#define _MessageHeader_hpp_


enum CMD{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_USER_JOIN,
    CMD_ERROR
};

// 消息头
struct netmsg_DataHeader
{
    netmsg_DataHeader(){
        dataLength = sizeof(netmsg_DataHeader);
        cmd = CMD_ERROR;
    }
    short dataLength;
    short cmd;
};

// DataPackage
struct netmsg_Login: public netmsg_DataHeader
{
    netmsg_Login(){
        dataLength = sizeof(netmsg_Login);
        cmd = CMD_LOGIN;
    }
    char userName[32];
    char userPassWord[32];
    char data[32];
};

struct netmsg_LoginResult: public netmsg_DataHeader
{
    netmsg_LoginResult(){
        dataLength = sizeof(netmsg_LoginResult);
        cmd = CMD_LOGIN_RESULT;
        result = 666;
    }
    int result;
    char data[92];
};

struct netmsg_NewUserJoin: public netmsg_DataHeader
{
    netmsg_NewUserJoin(){
        dataLength = sizeof(netmsg_NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
        sock = 0;
    }
    int sock;
};

struct netmsg_Logout: public netmsg_DataHeader
{
    netmsg_Logout(){
        dataLength = sizeof(netmsg_Logout);
        cmd = CMD_LOGOUT;
    }
    char userName[32];
};

struct netmsg_LogOutResult: public netmsg_DataHeader
{
    netmsg_LogOutResult(){
        dataLength = sizeof(netmsg_LogOutResult);
        cmd = CMD_LOGOUT_RESULT;
        result = 666;
    }
    int result;
};

#endif