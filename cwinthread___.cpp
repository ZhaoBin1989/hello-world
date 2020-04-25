// myPatience.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "_AFXTLS_.h"
#include <iostream>
#include <thread>
#include <string>
#include <process.h>
#include <mutex>
#define thread_local(a,b) CThreadLocal<a> b;
using namespace std;
struct CMyThreadData : public CNoTrackObject

{

	int nSomeData;

};

//CThreadLocal<CMyThreadData> g_myThreadData;//运用模版，减少转换
CThreadLocal<CMyThreadData> g_myThreadData;
//thread_local(CMyThreadData, g_myThreadData);

void ShowData();

UINT __stdcall ThreadFunc(LPVOID lpParam)
{
	g_myThreadData->nSomeData = (int)lpParam;//此处->重载

	ShowData();

	return 0;

}
int _tmain(int argc, _TCHAR* argv[])
{

	HANDLE h[10];
	UINT uID;

	// 启动十个线程，将i做为线程函数的参数传过去

	for (int i = 0; i<10; i++)
		h[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, (void*)(i + 1), 0, &uID);//i作为参数传进去

	WaitForMultipleObjects(1, h, TRUE, INFINITE);

	for (int i = 0; i<1; i++)

		CloseHandle(h[i]);

	getchar();
	return 0;
}


void ShowData()

{

	int nData = g_myThreadData->nSomeData;

	printf(" Thread ID: %-5d, nSomeData = %d \n", GetCurrentThreadId(), nData);

}