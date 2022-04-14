#ifndef _Cell_Semaphore_hpp_
#define _Cell_Semaphore_hpp_

#include <chrono>
#include <thread>

#include <condition_variable>


class CellSemaphore
{
private:
    std::mutex _mutex;
    std::condition_variable _cv;
    // 等待计数
    int _wait = 0;
    // 唤醒计数
    int _wakeup = 0;
public:
    void wait(){
        std::unique_lock<std::mutex> lock(_mutex);
        if (--_wait < 0)
        {
            _cv.wait(lock,[this]()->bool{
                return _wakeup > 0;
            });
            --_wakeup;

        }
        
    }



    void wakeup(){
        std::unique_lock<std::mutex> lock(_mutex);
        if (++_wait <= 0)
        {
            ++_wakeup;
            _cv.notify_one();
        }
        
    }
};


















#endif