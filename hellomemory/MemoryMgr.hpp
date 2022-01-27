#ifndef _MEMORYMGR_HPP_
#define _MEMORYMGR_HPP_


#include <stdlib.h>
#include <assert.h>


class MemoryAlloc;

// 内存块 最小单元
class MemoryBlock
{
private:
    /* data */
public:
    MemoryBlock(/* args */);
    ~MemoryBlock();

    // 内存块编号
    int nID;
    // 引用次数
    int nRef;
    // 所属内存池
    MemoryAlloc* pAlloc;
    // 下一块位置
    MemoryAlloc* pNext;
    // 是否在内存池中；
    bool pBool;

};



// 内存池
class MemoryAlloc
{
private:
    //内存池地址 
    char* _pBuf;
    // 头部内存单元
    MemoryBlock* _pHeader;
    // 内存单元大小；
    size_t _nSize;
    // 内存块数量；
    size_t _nBlockCnt;


public:
    MemoryAlloc(){

        _pbuf = nullptr;
        _pHeader = nullptr;
        _nSize = 0;
        _nBlockCnt = 0;

    }
    ~MemoryAlloc();

    // 申请内存
    void* allocMemory(size_t nSize){

        if(!_pBuf){

            initMemory();

        }

        MemoryBlock* pReturn = nullptr;
        if(nullptr == _pHeader){

            _pHeader = (MemoryBlock*)malloc(nSize+sizeof(MemoryBlock));
            pReturn->pBool = false;
            pReturn->nID = -1;
            pReturn->nRef = 0;
            pReturn->pAlloc = this;
            pReturn->pNext = nullptr;
        }else{
            pReturn = _pHeader;
            _pHeader = _pHeader->pNext;
            assert(0 == pReturn->nRef);
            pReturn->nRef = 1;
        }
        return pReturn;
    }


    void freeMem(void* p){
        free(p);
    }


    void initMemory(){
        
        assert(nullptr == _pBuf);
        if(!_pBuf){
            return;
        }

        // 计算内存池大小
        size_t bufSize = _nSize*_nBlockCnt;
        // 申请内存
        _pBuf = (char *)malloc(bufSize);   

        // 初始化内存池
        _pHeader = (MemoryBlock*)_pBuf;
        _pHeader->pBool = true;
        _pHeader->nID = 0;
        _pHeader->nRef = 0;
        _pHeader->pAlloc = this;
        _pHeader->pNext = nullptr; 

        MemoryBlock* Temp = _pHeader;
        for (size_t i = 1; i < _nBlockCnt; i++)
        {
            MemoryBlock* pTemp = (MemoryBlock*)(_pBuf+(i*_nSize)); 
            pTemp->pBool = true;
            pTemp->nID = 0;
            pTemp->nRef = 0;
            pTemp->pAlloc = this;
            pTemp->pNext = nullptr;
            Temp->pNext = pTemp; 
            Temp = pTemp;
        }

    }

    
    
};



// 内存池管理工具
class MemoryMgr
{
private:
    /* data */
private:
    MemoryMgr(/* args */);
    ~MemoryMgr();

public:

    static MemoryMgr& instance(){
        static MemoryMgr mgr;
        return mgr;
    }

    void* allocMem(size_t nSize){
        return malloc(nSize);
    }

    void freeMem(void* p){
        free(p);
    }
};



#endif