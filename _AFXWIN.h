#pragma once
#include <Windows.h>
typedef UINT(__cdecl *AFX_THREADPROC)(LPVOID);

class CWinThread  //_AFXWIN.H
{
public:
	CWinThread();
	BOOL CreateThread(DWORD dwCreateFlags = 0, UINT nStackSize = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

	BOOL m_bAutoDelete;
	HANDLE m_hThread;
	DWORD m_nThreadID;
	operator HANDLE() const
	{ return this == NULL?NULL:m_hThread; }

	int GetThreadPriority()
	{
		return ::GetThreadPriority(m_hThread);
	}
	BOOL SetThreadPriority(int nPriority)
	{
		return ::SetThreadPriority(m_hThread, nPriority);
	}
	DWORD SuspendThread()
	{
		return ::SuspendThread(m_hThread);
	}
	DWORD ResumeThread()
	{
		return ::ResumeThread(m_hThread);
	}

public:
	virtual ~CWinThread();
	virtual void Delete();
	void CommonConstruct();
public:
	CWinThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam);
	LPVOID m_pThreadParams;
	AFX_THREADPROC m_pfnThreadProc;
};

CWinThread* AfxBeginThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam, 
	int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0, 
	DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

CWinThread* AfxGetThread();
void AfxEndThread(UINT nExitCode, BOOL bDelete = TRUE);

