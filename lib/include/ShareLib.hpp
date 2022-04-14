#ifndef _SHARELIB_HPP_
#define _SHARELIB_HPP_


#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>


#include "MessageHeader.hpp"
#include "CELLTimestamp.hpp"
#include "CellTask.hpp"
#include "CellobjectPool.hpp"
#include "Logger.hpp"


#include <stdio.h>


#pragma comment(lib,"ws2_32.lib")

#ifndef RECV_BUFF_SIZE
// 接收缓冲区最小单元大小
#define RECV_BUFF_SIZE 8192
#endif
#ifndef SEND_BUFF_SIZE
// 发送缓冲区最小单元大小
#define SEND_BUFF_SIZE 102400
#endif








#endif