#ifndef _CELLOBJECTPOLL_HPP_
#define _CELLOBJECTPOLL_HPP_

#include <stdlib.h>
#include <assert.h>
#include <mutex>


template<typename T,size_t nPoolSize>
class CellobjectPool
{
private:
    class NodeHeader
    {
    private:
        /* data */
    public:

        // 内存块编号
        int nID;
        // 引用次数
        char nRef;
        // 下一块位置
        NodeHeader* pNext;
        // 是否在内存池中；
        bool pBool;
    };
    
public:
    CellobjectPool(/* args */);
    ~CellobjectPool();


    void initPool(){

        assert(nullptr == _pBuf);
        if(_pBuf){
            return;
        }

        // 计算对象池大小
        size_t n = nPoolSize*(sizeof(T) + sizeof(NodeHeader));
        // 向内存池申请内存
        _pBuf = new char[n];

        // 初始化内存池
        _pHeader = (NodeHeader*)_pBuf;
        _pHeader->pBool = true;
        _pHeader->nID = 0;
        _pHeader->nRef = 0;
        _pHeader->pNext = nullptr; 

        NodeHeader* Temp = _pHeader;
        for (size_t i = 1; i < nPoolSize; i++)
        {
            NodeHeader* pTemp = (NodeHeader*)(_pBuf+(i*(sizeof(T)+sizeof(NodeHeader)))); 
            pTemp->pBool = true;
            pTemp->nID = i;
            pTemp->nRef = 0;
            pTemp->pNext = nullptr;
            Temp->pNext = pTemp; 
            Temp = pTemp;
        }

    }


private:
    //内存池地址 
    char* _pBuf;
    // 头部内存单元
    NodeHeader* _pHeader;
    std::mutex _mutex; 

};



template<typename T>
class ObjectPoolBase
{
private:
    /* data */
public:
    ObjectPoolBase(/* args */);
    ~ObjectPoolBase();

    void* operator new(size_t nsize){
        return malloc(nsize);
    }


    void operator delete(void * p){
        free(p);
    }

    // 可变参数  
    template<typename ...Args>
    static T* createObject(Args ... args){

        T* obj = new T(args ...);
        return obj;

    }

    static void deleteObject(T* obj){

        delete obj;

    }


};





#endif