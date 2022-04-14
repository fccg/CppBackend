#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_



#include "ShareLib.hpp"


class Logger
{
private:

    Logger(/* args */)
    {
        _taskServer.Start();
    }
    ~Logger()
    {
        _taskServer.Close();
        if (_logFile)
        {
            Info("Logger fclose _logFile\n");
            fclose(_logFile);
            _logFile = nullptr;
        }
    }

public:
    static Logger& Instance()
    {
        static Logger slog;
        return slog;
    }

    // log等级Info，以此为例子
    static void Info(const char* pStr)
    {
        Logger* pLog = &Instance();
        pLog->_taskServer.addTask([pLog,pStr](){
            if (pLog->_logFile)
            {
                auto t = system_clock::now();
                auto tNow = system_clock::to_time_t(t);
                
                // fprintf(pLog->_logFile,"%s",ctime(&tNow));
                std::tm* now = std::gmtime(&tNow);
                fprintf(pLog->_logFile,"[%d-%d-%d %d:%d:%d] INFO: ",now->tm_year+1900,now->tm_mon+1,now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec);
                fprintf(pLog->_logFile,"%s",pStr);
                fflush(pLog->_logFile);
            }
            printf(pStr);
        });
        
    }

    // log等级Info，以此为例子
    template<typename ...Args>
    static void Info(const char* pformat,Args... args)
    {
        Logger* pLog = &Instance();
        pLog->_taskServer.addTask([pLog,pformat,args...](){
            if (pLog->_logFile)
            {
                auto t = system_clock::now();
                auto tNow = system_clock::to_time_t(t);
                
                // fprintf(pLog->_logFile,"%s",ctime(&tNow));
                std::tm* now = std::gmtime(&tNow);
                fprintf(pLog->_logFile,"[%d-%d-%d %d:%d:%d] INFO: ",now->tm_year+1900,now->tm_mon+1,now->tm_mday,now->tm_hour,now->tm_min,now->tm_sec);
                fprintf(pLog->_logFile,pformat,args...);
                fflush(pLog->_logFile);
            }
            printf(pformat,args...);
        });
        
    }

    void setLogPath(const char* logPath,const char* mode)
    {
        if (_logFile)
        {
            Info("Logger::setLogPath fclose\n");
            fclose(_logFile);
            _logFile = nullptr;
        }

        _logFile = fopen(logPath,mode);
        if (_logFile)
        {
            Info("Logger::setLogPath success,<%s,%s>\n",logPath,mode);
        }
        else{
            Info("Logger::setLogPath failed,<%s,%s>\n",logPath,mode);
        }
        
        
    }

private:

    FILE* _logFile = nullptr;
    CellTaskServer _taskServer;



};










#endif