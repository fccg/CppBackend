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
struct DataHeader
{
    DataHeader(){
        dataLength = sizeof(DataHeader);
        cmd = CMD_ERROR;
    }
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
    char data[932];
};

struct LoginResult: public DataHeader
{
    LoginResult(){
        dataLength = sizeof(LoginResult);
        cmd = CMD_LOGIN_RESULT;
        result = 666;
    }
    int result;
    char data[992];
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

#endif