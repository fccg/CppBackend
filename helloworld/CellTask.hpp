#ifndef _CELL_TASK_HPP_
#define _CELL_TASK_HPP_

 
#include <thread>
#include <mutex>
#include <list>





class CellTask
{
private:
    /* data */
public:
    CellTask(/* args */){

    }
    virtual ~CellTask(){

    }
    // 执行任务
    virtual void doTask(){

    }
};


class CellTaskServer
{
private:

    // 任务数据
    std::list<std::shared_ptr<CellTask>> _tasks;
    // 任务数据缓冲区
    std::list<std::shared_ptr<CellTask>> _tasksBuf;
    // 改变数据缓冲区时需要加锁
    std::mutex _mutex;

 public:
    CellTaskServer(/* args */){
   
    }

    ~CellTaskServer(){

    }
    // 添加任务
    void addTask(std::shared_ptr<CellTask> task){

        std::unique_lock<std::mutex> lock(_mutex);
        _tasksBuf.push_back(task);
        

    }
    // 启动服务 
    void Start(){

        std::thread t(std::mem_fun(&CellTaskServer::onRun),this);
        t.detach();

    }

protected:
    // 工作函数
    void onRun(){
        

        while(true){

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
                pTask->doTask();
                // delete pTask;
            }

            // 清空任务
            _tasks.clear();

        } 
        
    }



};






#endif