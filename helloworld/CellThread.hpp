#ifndef _CELL_THREAD_HPP_
#define _CELL_THREAD_HPP_

#include "CellSemaphore.hpp"
#include <functional>
#include <mutex>

class CellThread
{
private:
    // 线程是否启动中
    bool _isRun = false;
    // 控制线程终止、退出
    CellSemaphore _sem;
    std::mutex _mutex;

    using Event = std::function<void(CellThread*)>;

public:
    //启动线程 
    void Start(Event onCreate=nullptr,Event onRun=nullptr,Event onStop=nullptr){
        std::unique_lock<std::mutex> lock(_mutex);
        if(!_isRun){
            _isRun = true;

            if (onCreate)
            {
                _onCreate = onCreate;
            }
            if (onRun)
            {
                _onRun = onRun;
            }
            if (onStop)
            {
                _onStop = onStop;
            }

            std::thread t(std::mem_fun(&CellThread::onWork),this);
            t.detach();
            
            
        }
        
        

    }

    // 线程运行状态
    bool isRun(){
        return _isRun;
    }

    // 由关闭其他线程
    void Close(){
        std::unique_lock<std::mutex> lock(_mutex);
        if (_isRun)
        {
            _isRun = false;
            _sem.wait();
        }
    }

    // 主动关闭自身线程
    void SelfExit(){
        std::unique_lock<std::mutex> lock(_mutex);
        if (_isRun)
        {
            _isRun = false;
            
        }
        

    }

protected:
    //线程工作 
    void onWork(){
        if (_onCreate)
            _onCreate(this);
        if (_onRun)
            _onRun(this);
        if (_onStop)
            _onStop(this);

        _sem.wakeup();
    }

private:
    Event _onCreate;
    Event _onRun;
    Event _onStop;


};

















#endif