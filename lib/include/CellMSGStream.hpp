#ifndef _CELL_MSG_STREAM_HPP_
#define _CELL_MSG_STREAM_HPP_


#include "MessageHeader.hpp"
#include "CellStream.hpp"


class CellRECVStream : public CellStream
{
private:

public:

    CellRECVStream(netmsg_DataHeader* header)
    :CellStream((char *)header, header->dataLength)
    {
        push(header->dataLength);
        // 预读取消息长度（抛弃）
        readInt16(); 
    }

    // 读取消息类型
    uint16_t getNetCmd(){
        uint16_t cmd = CMD_ERROR;
        Read<uint16_t>(cmd);
        return cmd;
    }


};







class CellSendStream : public CellStream
{
private:

public:


    CellSendStream(char* pData,int Size,bool isDelete = false)
    :CellStream(pData, Size, isDelete)
    {
        // 预先占领消息长度所需空间
        write<uint16_t>(0);
        
    }

    CellSendStream(int nSize = 1024)
    :CellStream(nSize)
    {
        // 预先占领消息长度所需空间
        write<uint16_t>(0);

    }

    void setNetCmd(uint16_t cmd){
        write<uint16_t>(cmd);
    }


    bool WriteString(const char* str,int len){
        return writeArray(str,len);
    }

    bool WriteString(const char* str){

        return writeArray(str,strlen(str));
    }

    bool WriteString(std::string& str){
        
        return writeArray(str.c_str(),str.length());
    }


    void finish(){
        int pos = getWritePos();
        setWritePos(0);
        write<uint16_t>(pos);
        setWritePos(pos);
    }

};















#endif