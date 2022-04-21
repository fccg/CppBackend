#ifndef _CELL_STREAM_HPP_
#define _CELL_STREAM_HPP_


#include <cstdint>
#include <stdio.h>
#include <string.h>


class CellStream
{
private:

    // 数据缓冲区
    char* _pBuff = nullptr;
    // 缓冲区总空间大小，字节长度
    int _nSize;

    // 已写数据的尾部位置
    int _WritePos = 0;
    // 已读数据的尾部位置
    int _ReadPos = 0;
    // pBuff如果由外部传入是否需要被释放
    bool _isDelete = true;

public:
    CellStream(char* pData,int Size,bool isDelete = false){

        _nSize = Size;
        _pBuff = pData;
        _isDelete = isDelete;
        
    }
    CellStream(int nSize = 1024){

        _nSize = nSize;
        _pBuff = new char[_nSize];
        _isDelete = true;

    }
    ~CellStream(){

         if(_pBuff){
            delete[] _pBuff;
            _pBuff = nullptr;
        }
    }


    // read
    template<typename T>
    bool Read(T& n,bool offset = true){

        
        // 计算要读取数据的长度
        auto nlen = sizeof(T);
        // 判断是否可读
        if (_ReadPos + nlen <= _nSize)
        {
            // 将待读取数据复制出来
            memcpy(&n,_pBuff + _ReadPos,nlen);
            // 计算已读位置
            if(offset == true){
                _ReadPos += nlen;
            }
            
            return true;
        }
        
        return false;
    }


    template<typename T>
    uint32_t ReadArray(T* pArr,uint32_t len){

        // 读取数组元素个数，对应于先写入数据元素个数，但位置不偏移
        uint32_t n = 0;
        Read(n,false);
        // 判断缓存数组pArr能否放的下
        if (n < len)
        {
            // 计算数组的字节长度
            auto nlen = n * sizeof(T);
            if(_ReadPos + nlen + sizeof(uint32_t) <= _nSize){
                // 计算已读位置+数组长度占有的空间
                _ReadPos += sizeof(uint32_t);
                // 读数据
                memcpy(pArr,_pBuff + _ReadPos,nlen);
                // 计算已读数据位置
                _ReadPos += nlen;
                return n;

            }

        }
        return 0;
        

    }


    int8_t readInt8(){

        int8_t def = 0;
        Read(def);
        return def;
    }
    int16_t readInt16(){
        int16_t def = 0;
        Read(def);
        return def;
    }
    int32_t readInt32(){
        int32_t def = 0;
        Read(def);
        return def;
    }
    float readFloat(){
        float def = 0;
        Read(def);
        return def;
    }
    double readDouble(){
        double def = 0;
        Read(def);
        return def;
    }

    

    // write
    template<typename T>
    bool write(T n){

        // 计算要写入数据的大小
        auto nLen = sizeof(T);
        // 能不能写入
        if (_WritePos + nLen <= _nSize)
        {
            // 写到什么位置
            memcpy(_pBuff+_WritePos,&n,nLen);
            _WritePos += nLen;
            return true;
        }
        return false;
        
    }


    template<typename T>
    bool writeArray(T* pData,uint32_t len){

        // 计算要写入数组的大小
        auto nLen = sizeof(T) * len;
        if (_WritePos + nLen + sizeof(uint32_t) <= _nSize)
        {
            // 写入数据长度，方便读取
            writeInt32(len);
            // 写到什么位置
            memcpy(_pBuff+_WritePos,&pData,nLen);
            _WritePos += nLen;
            return true;
        } 
        return false;

    }  


    bool writeInt8(int8_t n){
        return write(n);
    }
    bool writeInt16(int16_t n){
        return write(n);
    }
    bool writeInt32(int32_t n){

        return write(n);

    }
    
    bool writeFloat(float n){
        return write(n);
    }
    bool writeDouble(double n){
        return write(n);
    }


};















#endif