#ifndef _MEMORYMGR_HPP_
#define _MEMORYMGR_HPP_


#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <stdio.h>



#define MAX_MEMORY_SIZE 64

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
    MemoryBlock* pNext;
    // 是否在内存池中；
    bool pBool;

};



// 内存池
class MemoryAlloc
{
protected:
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

        _pBuf = nullptr;
        _pHeader = nullptr;
        _nSize = 0;
        _nBlockCnt = 0;

    }
    ~MemoryAlloc(){
        if (_pBuf)
        {
            free(_pBuf);
        }
        
    }

    // 申请内存
    void* allocMemory(size_t nSize){

        if(!_pBuf){

            initMemory();

        }

        MemoryBlock* pReturn = nullptr;
        // 没有现成的内存单元可分配
        if(nullptr == _pHeader){

            pReturn = (MemoryBlock*)malloc(nSize+sizeof(MemoryBlock));
            pReturn->pBool = false;
            pReturn->nID = -1;
            pReturn->nRef = 1;
            pReturn->pAlloc = nullptr;
            pReturn->pNext = nullptr;
        }else{
            pReturn = _pHeader;
            _pHeader = _pHeader->pNext;
            assert(0 == pReturn->nRef);
            pReturn->nRef = 1;
        }
        printf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
        // std::cout << "allocMem   " << pReturn << "   id   " << pReturn->nID << "   size   " << nSize << std::endl;
        return ((char*)pReturn + sizeof(MemoryBlock));
    }


    void freeMemory(void* pMem){

        MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
        assert(1 == pBlock->nRef);
        if(--pBlock->nRef != 0){
            return;
        }

        if (pBlock->pBool)
        {
            pBlock->pNext = _pHeader;
            _pHeader = pBlock;
        }
        else{
            free(pBlock);
        }
        
    }


    void initMemory(){
        
        assert(nullptr == _pBuf);
        if(_pBuf){
            return;
        }

        // 计算内存池大小
        size_t bufSize = (_nSize+sizeof(MemoryBlock))*_nBlockCnt;
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
            MemoryBlock* pTemp = (MemoryBlock*)(_pBuf+(i*(_nSize+sizeof(MemoryBlock)))); 
            pTemp->pBool = true;
            pTemp->nID = i;
            pTemp->nRef = 0;
            pTemp->pAlloc = this;
            pTemp->pNext = nullptr;
            Temp->pNext = pTemp; 
            Temp = pTemp;
        }

    }

    
    
};

// 便于声明类成员变量时初始化MemoryAlloc的成员数据
template<size_t nSize,size_t nBlockCnt>
class MemoryAlloctor : public MemoryAlloc{

public:
    MemoryAlloctor(){

        const size_t n = sizeof(void*);

        _nSize = (nSize/n)*n + (nSize%n ? n : 0);
        
        _nBlockCnt = nBlockCnt;
    }


};



// 内存池管理工具
class MemoryMgr
{

private:
    MemoryMgr(){
        init(0,64,&_mem64);
    }
    ~MemoryMgr(){

    }


    // 初始化内存池映射数组
    void init(int nbegin,int nEnd,MemoryAlloc* pMem){
        for (int i = nbegin; i <= nEnd; i++)
        {
            _szAlloc[i] = pMem;
        }
        
    }

public:

    static MemoryMgr& instance(){
        static MemoryMgr mgr;
        return mgr;
    }

    void* allocMem(size_t nSize){

        if (nSize <= MAX_MEMORY_SIZE)
        {
            return _szAlloc[nSize]->allocMemory(nSize);
        }
        else{
            MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize+sizeof(MemoryBlock));
            pReturn->pBool = false;
            pReturn->nID = -1;
            pReturn->nRef = 1;
            pReturn->pAlloc = nullptr;
            pReturn->pNext = nullptr;
            printf("allocMem: %llx, id=%d, size=%d\n", pReturn, pReturn->nID, nSize);
            // std::cout << "allocMem  " << pReturn << "    id   " << pReturn->nID << "    size  " << nSize << std::endl;
            return ((char*)pReturn + sizeof(MemoryBlock));
        }
        
        
    }
    // 释放内存
    void freeMem(void* pMem){
        MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
        printf("allocMem: %llx, id=%d\n", pBlock, pBlock->nID);
        // std::cout << "freeMem    " << pBlock << "   id   " << pBlock->nID << std::endl;
        if (pBlock->pBool)
        {
            pBlock->pAlloc->freeMemory(pMem);
        }
        else{
            if (--pBlock->nRef == 0)
            {
                free(pBlock);
            }
            
        }
        
    }
    // 增加内存块的引用计数
    void* addRef(void* pMem){
        MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
        ++pBlock->nRef;
    }

private:
    MemoryAlloctor<64,10> _mem64;
    MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE+1]; 
};



#endif