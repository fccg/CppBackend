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
    CellobjectPool(/* args */){

        _pBuf = nullptr;
        initPool();
    }
    ~CellobjectPool(){
        if(_pBuf)
            delete[] _pBuf;
    }
    

    // 释放对象内存
    void freeObjMemory(void* pMem){

        NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));
        assert(1 == pBlock->nRef);
        if(--pBlock->nRef != 0){
            return;
        }

        if (pBlock->pBool)
        {
            std::lock_guard<std::mutex> lg(_mutex);
            if(--pBlock->nRef != 0){
                return;
            }
            pBlock->pNext = _pHeader;
            _pHeader = pBlock;
        }
        else{
            if (--pBlock->nRef != 0)
            {
                return;
            }
            
            delete[] pBlock;
        }
        
    }

    // 申请对象内存
    void* allocObjMemory(size_t nSize){

        std::lock_guard<std::mutex> lg(_mutex);

        NodeHeader* pReturn = nullptr;
        // 没有现成的内存单元可分配
        if(nullptr == _pHeader){

            pReturn = (NodeHeader*)new char[sizeof(T)+sizeof(NodeHeader)];
            pReturn->pBool = false;
            pReturn->nID = -1;
            pReturn->nRef = 1;
            pReturn->pNext = nullptr;
        }else{
            pReturn = _pHeader;
            _pHeader = _pHeader->pNext;
            assert(0 == pReturn->nRef);
            pReturn->nRef = 1;
        }
        // printf("allocObjMemory: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
        // std::cout << "allocMem   " << pReturn << "   id   " << pReturn->nID << "   size   " << nSize << std::endl;
        return ((char*)pReturn + sizeof(NodeHeader));
    }

private:
    // 初始化对象池
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



template<typename T,size_t nPoolSize>
class ObjectPoolBase
{
private:
    /* data */
public:
    ObjectPoolBase(/* args */) = default;
    ~ObjectPoolBase() = default;

    void* operator new(size_t nsize){
        return objectPool().allocObjMemory(nsize);
    }


    void operator delete(void * p){
        objectPool().freeObjMemory(p);
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

private:
    using ClassTPool = CellobjectPool<T,nPoolSize>;

    static ClassTPool& objectPool(){
        // 静态
        static ClassTPool Tpool;
        return Tpool;
    }


};





#endif