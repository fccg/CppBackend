#include <stdlib.h>
#include "Alloctor.h"
#include <iostream>
#include <thread>
#include <mutex>//锁
#include "CELLTimestamp.hpp"
#include "CellobjectPool.hpp"


std::mutex m;
const int tCount = 8;
const int mCount = 10000;
const int nCount = mCount/tCount;

void workFun(int index)
{
	char* data[nCount];
	for (size_t i = 0; i < nCount; i++)
	{
		data[i] = new char[(rand()%128)+1];
	}
	for (size_t i = 0; i < nCount; i++)
	{
		delete[] data[i];
	}
	/*
	for (int n = 0; n < nCount; n++)
	{
		//自解锁
		//lock_guard<mutex> lg(m);
		//临界区域-开始
		//m.lock();

		//m.unlock();
		//临界区域-结束
	}//线程安全 线程不安全
	 //原子操作 计算机处理命令时最小的操作单位
	 */
}//抢占式


int main(){

    std::thread t[tCount];
	for (int i = 0; i < tCount; i++)
	{
		t[i] = std::thread(workFun, i);
	}
	CELLTimestamp tTime;
	for (int i = 0; i < tCount; i++)
	{
		t[i].join();
		//t[n].detach();
	}
	std::cout << tTime.getElapsedTimeInMilliSec() << std::endl;
	std::cout << "Hello,main thread." << std::endl;


    system("pause");

    return 0;
}