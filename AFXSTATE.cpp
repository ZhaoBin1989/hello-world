#include "stdafx.h"
#include "_afxstat_.h"

#define THREAD_LOCAL(class_name, ident_name)

THREAD_LOCAL(AFX_MODULE_THREAD_STATE, _afxModuleThreadState)
AFX_MODULE_THREAD_STATE* AfxGetModuleThreadState()
{
	//return _afxModuleThreadState.GetData();


	AFX_MODULE_THREAD_STATE* fdgdf = new AFX_MODULE_THREAD_STATE;
	return fdgdf;
}



