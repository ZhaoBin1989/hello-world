#include "stdafx.h"
#include "_AFXWIN.h"
#include "_AFXSTAT_.h"
struct _AFX_THREAD_STARTUP
{
	CWinThread* pThread;
	HANDLE hEvent;
	HANDLE hEvent2;
	BOOL bError;
};
UINT __stdcall _AfxThreadEntry(void* pParam)
{
	_AFX_THREAD_STARTUP* pStartup = (_AFX_THREAD_STARTUP*)pParam;
	CWinThread* pThread = pStartup->pThread;
	try
	{
		AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
		pState->m_pCurrentWinThread = pThread;
	}
	catch(...)
	{
		pStartup->bError = TRUE;
		::SetEvent(pStartup->hEvent);
		AfxEndThread((UINT)-1, FALSE);
	}

	HANDLE hEvent2 = pStartup->hEvent2;
	::SetEvent(pStartup->hEvent);

	::WaitForSingleObject(hEvent2, INFINITE);
	::CloseHandle(hEvent2);
	DWORD nResult = (*pThread->m_pfnThreadProc)(pThread->m_pThreadParams);

	AfxEndThread(nResult);
	return 0;

}

CWinThread* AfxGetThread()
{
	AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
	return pState->m_pCurrentWinThread;
}

void AfxEndThread(UINT nExitCode,BOOL bDelete)
{
	AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
	CWinThread* pThread = pState->m_pCurrentWinThread;
	if (pThread != NULL)
	{
		if (bDelete)
			pThread->Delete();
		pState->m_pCurrentWinThread = NULL;
	}

	/*if (_afxThreadData != NULL)
	{
		_afxThreadData->DeleteValues(NULL, FALSE);
	}*/
	_endthreadex(nExitCode);
}

CWinThread* AfxBeginThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam, int nPriority, UINT nStackSize, DWORD dwCreateFlags, LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
	CWinThread* pThread = new CWinThread(pfnThreadProc, pParam);

	if (!pThread->CreateThread(dwCreateFlags | CREATE_SUSPENDED, nStackSize, lpSecurityAttrs))
	{
		pThread->Delete();
		return NULL;
	}

	pThread->SetThreadPriority(nPriority);

	if (!(dwCreateFlags & CREATE_SUSPENDED))
		pThread->ResumeThread();
	return pThread;
}

void CWinThread::CommonConstruct()
{
	m_hThread = NULL;
	m_nThreadID = 0;
	m_bAutoDelete = TRUE;
}

CWinThread::CWinThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam)
{
	m_pfnThreadProc = pfnThreadProc;
	m_pThreadParams = pParam;
	CommonConstruct();
}
CWinThread::CWinThread()
{
	m_pfnThreadProc = NULL;
	m_pThreadParams = NULL;
	CommonConstruct();
}
CWinThread::~CWinThread()
{
	if (m_hThread != NULL)
		CloseHandle(m_hThread);
	AFX_MODULE_THREAD_STATE* pState = AfxGetModuleThreadState();
	if (pState->m_pCurrentWinThread == this)
		pState->m_pCurrentWinThread = NULL;
}
BOOL CWinThread::CreateThread(DWORD dwCreateFlags, UINT nStackSize, LPSECURITY_ATTRIBUTES lpSecurityAttrs)
{
	_AFX_THREAD_STARTUP startup; memset(&startup, 0, sizeof(startup));
	startup.pThread = this;
	startup.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	startup.hEvent2 = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	m_hThread = (HANDLE)_beginthreadex(lpSecurityAttrs, nStackSize, &_AfxThreadEntry, &startup, dwCreateFlags | CREATE_SUSPENDED, (UINT*)&m_nThreadID);
	if (m_hThread == NULL)
		return FALSE;

	ResumeThread();
	::WaitForSingleObject(startup.hEvent, INFINITE);
	::CloseHandle(startup.hEvent);

	if (dwCreateFlags & CREATE_SUSPENDED)
		::SuspendThread(m_hThread);

	if (startup.bError)
	{
		::WaitForSingleObject(m_hThread, INFINITE);
		::CloseHandle(m_hThread);
		m_hThread = NULL;
		::CloseHandle(startup.hEvent2);
		return FALSE;

		::SetEvent(startup.hEvent2);
		return TRUE;
	}
}
void CWinThread::Delete()
{
	if (m_bAutoDelete)
		delete this;
}

