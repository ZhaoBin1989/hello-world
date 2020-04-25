#ifndef __AFXSTAT_H__
#define __AFXSTAT_H__

#ifndef __AFXTLS_H__
	#include "_afxtls_.h"
#endif

class CWinThread;

// AFX_MODULE_THREAD_STATE
class AFX_MODULE_THREAD_STATE:public CNoTrackObject
{
public:
	// ֻ�뵱ǰ�̶߳���CWinThread���󡪡���ִ��
	CWinThread* m_pCurrentWinThread;
	AFX_MODULE_THREAD_STATE* m_pPrevModuleState;
	//AFX_MODULE_THREAD_STATE* GetData();
};
#define EXTERN_THREAD_LOCAL(class_name, ident_name) 
EXTERN_THREAD_LOCAL(AFX_MODULE_THREAD_STATE, _afxModuleThreadState)
AFX_MODULE_THREAD_STATE* AfxGetModuleThreadState();

//AFX_MODULE_THREAD_STATE* _afxModuleThreadState;




#endif //__AFXSTAT_HX__