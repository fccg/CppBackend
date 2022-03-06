#ifndef _CELL_TASK_HPP_
#define _CELL_TASK_HPP_

 
#include <thread>
#include <mutex>
#include <list>
#include <functional>


#include "CellThread.hpp"



class CellTaskServer
{
private:

    using CellTask = std::function<void()>;

    // 任务数据
    std::list<CellTask> _tasks;
    // 任务数据缓冲区
    std::list<CellTask> _tasksBuf;
    // 改变数据缓冲区时需要加锁
    std::mutex _mutex;

    CellThread _thread;


 public:
    CellTaskServer(/* args */) =default;

    ~CellTaskServer() = default;
    // 添加任务
    void addTask(CellTask task){

        std::unique_lock<std::mutex> lock(_mutex);
        _tasksBuf.push_back(task);
        

    }
    // 启动服务 
    void Start(){
        // _isRun = true;
        // std::thread t(std::mem_fun(&CellTaskServer::onRun),this);
        // t.detach();

        _thread.Start(nullptr,[this](CellThread* t){
            onRun(t);
        },nullptr);
    }

    void Close(){

        printf("CellTaskServer close1\n");
        // if (_isRun)
        // {
        //     _isRun = false;
        //     _sem.wait();
        // }
        _thread.Close();
        printf("CellTaskServer close2\n");
        
        

    }

protected:
    // 工作函数
    void onRun(CellThread* t){
        

        while(t->isRun()){

            // 从缓冲区取出任务
            if (!_tasksBuf.empty())
            {
                std::unique_lock<std::mutex> lock(_mutex);
                for (auto pTask:_tasksBuf)
                {
                    _tasks.push_back(pTask);
                }
                _tasksBuf.clear();
            }  

            // 如果没有任务
            if (_tasks.empty())
            {
                std::chrono::milliseconds dua(1);
                std::this_thread::sleep_for(dua);
                continue;
            }

            // 处理任务
            for(auto pTask:_tasks){
                pTask();
                // delete pTask;
            }

            // 清空任务
            _tasks.clear();

        } 

        
    }



};






#endif