#include <stdlib.h>

#include "Alloctor.h"
#include "MemoryMgr.hpp"


void* operator new(size_t size){
    
    return MemoryMgr::instance().allocMem(size); 
}


void operator delete(void* p){

    MemoryMgr::instance().freeMem(p); 
}

void operator delete(void* p,size_t size){

    MemoryMgr::instance().freeMem(p); 
}


void* operator new[](size_t size){

    return MemoryMgr::instance().allocMem(size); 
}

void operator delete[](void* p){

    MemoryMgr::instance().freeMem(p); 
}

void* mem_alloc(size_t size){

    return malloc(size); 

}

void mem_free(void* p){
    
    free(p); 

}