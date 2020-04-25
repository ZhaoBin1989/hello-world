#include "stdafx.h"
#include "_AFXTLS_.h"

void CSimpleList::AddHead(void* p)
{
	*GetNextPtr(p) = m_pHead;
	m_pHead = p;
}

BOOL CSimpleList::Remove(void* p)
{
	if (p == NULL)   // 检查参数
		return FALSE;

	BOOL bResult = FALSE; // 假设移除失败
	if (p == m_pHead)
	{
		// 要移除头元素
		m_pHead = *GetNextPtr(p);
		bResult = TRUE;
	}
	else
	{
		// 试图在表中查找要移除的元素
		void* pTest = m_pHead;
		while (pTest != NULL && *GetNextPtr(pTest) != p)
			pTest = *GetNextPtr(pTest);

		// 如果找到，就将元素移除
		if (pTest != NULL)
		{
			*GetNextPtr(pTest) = *GetNextPtr(p);
			bResult = TRUE;
		}
	}
	return bResult;
}

void* CNoTrackObject::operator new(size_t nSize){	
	//申请一块内存	
	void* p = ::GlobalAlloc(GPTR , nSize);
	return p;
} 
void CNoTrackObject::operator delete(void* p)
{	
	if (p != NULL)
	{		::GlobalFree(p);
	}
}

//-----------------CThreadSlotData------------------//

BYTE __afxThreadData[sizeof(CThreadSlotData)];
CThreadSlotData* _afxThreadData;

struct CSlotData
{
	DWORD dwFlags;
	HINSTANCE hInst;
};
struct CThreadData :public CNoTrackObject
{
	CThreadData* pNext;
	int nCount;
	LPVOID* pData;
};

#define SLOT_USED 0x01

CThreadSlotData::CThreadSlotData()
{
	m_list.Construct(offsetof(CThreadData, pNext));

	m_nMax = 0;
	m_nAlloc = 0;
	m_nRover = 1;
	m_pSlotData = NULL;
	m_tlsIndex = ::TlsAlloc();
	::InitializeCriticalSection(&m_cs);
}

int CThreadSlotData::AllocSlot()
{
	::EnterCriticalSection(&m_cs);
	int nAlloc = m_nAlloc;
	int nSlot = m_nRover;

	if (nSlot >= nAlloc || m_pSlotData[nSlot].dwFlags & SLOT_USED)
	{
		for (nSlot = 1; nSlot < nAlloc && m_pSlotData[nSlot].dwFlags & SLOT_USED; nSlot++);

		if (nSlot >= nAlloc)
		{
			int nNewAlloc = nAlloc + 32;
			HGLOBAL hSlotData;
			if (m_pSlotData == NULL)
			{
				hSlotData = ::GlobalAlloc(GMEM_MOVEABLE, nNewAlloc*sizeof(CSlotData));
			}
			else
			{
				hSlotData = ::GlobalHandle(m_pSlotData);
				::GlobalUnlock(hSlotData);
				hSlotData = ::GlobalReAlloc(hSlotData, nNewAlloc*sizeof(CSlotData), GMEM_MOVEABLE);
			}
			CSlotData* pSlotData = (CSlotData*)::GlobalLock(hSlotData);
			memset(pSlotData + m_nAlloc, 0, (nNewAlloc - nAlloc)*sizeof(CSlotData));
			m_nAlloc = nNewAlloc;
			m_pSlotData = pSlotData;
		}
	}

	if (nSlot >= m_nMax)
		m_nMax = nSlot + 1;

	m_pSlotData[nSlot].dwFlags |= SLOT_USED;
	m_nRover = nSlot + 1;
	::LeaveCriticalSection(&m_cs);
	return nSlot;
}

void CThreadSlotData::SetValue(int nSlot, void* pValue)
{
	CThreadData* pData = (CThreadData*)::TlsGetValue(m_tlsIndex);

	if ((pData == NULL || nSlot >= pData->nCount) && pValue != NULL)
	{
		if (pData == NULL)
		{
			pData = new CThreadData;
			pData->nCount = 0;
			pData->pData = NULL;
			::EnterCriticalSection(&m_cs);
			m_list.AddHead(pData);
			::LeaveCriticalSection(&m_cs);

		}

		if (pData->pData == NULL)
			pData->pData = (void**)::GlobalAlloc(LMEM_FIXED, m_nMax*sizeof(LPVOID));
		else
			pData->pData = (void**)::GlobalReAlloc(pData->pData, m_nMax*sizeof(LPVOID), LMEM_MOVEABLE);

		memset(pData->pData + pData->nCount, 0, (m_nMax - pData->nCount)*sizeof(LPVOID));
		pData->nCount = m_nMax;
		::TlsSetValue(m_tlsIndex, pData);
		pData->pData[nSlot] = pValue;

	}
}

inline void* CThreadSlotData::GetThreadValue(int nSlot)
{
	CThreadData* pData = (CThreadData*)::TlsGetValue(m_tlsIndex);
	if (pData == NULL || nSlot >= pData->nCount)
	{
		return NULL;
	}
	return pData->pData[nSlot];
}
void CThreadSlotData::FreeSlot(int nSlot)
{
	::EnterCriticalSection(&m_cs);
	CThreadData* pData = m_list;
	while (pData != NULL)
	{
		if (nSlot < pData->nCount)
		{
			delete(CNoTrackObject*)pData->pData[nSlot];
			pData->pData[nSlot] = NULL;
		}
		pData = pData->pNext;
	}

	m_pSlotData[nSlot].dwFlags &= ~SLOT_USED;
	::LeaveCriticalSection(&m_cs);
}

void CThreadSlotData::DeleteValues(HINSTANCE hInst, BOOL bAll)
{
	::EnterCriticalSection(&m_cs);
	if (!bAll)
	{
		CThreadData* pData = (CThreadData*)::TlsGetValue(m_tlsIndex);
		if (pData != NULL)
			DeleteValues(pData, hInst);
	}
	else
	{
		CThreadData* pData = m_list.GetHead();
		while(pData != NULL)
		{
			CThreadData* pNextData = pData->pNext;
			DeleteValues(pData, hInst);
		}
	}

	::LeaveCriticalSection(&m_cs);
}

void CThreadSlotData::DeleteValues(CThreadData* pData, HINSTANCE hInst)
{
	BOOL bDelete = TRUE;
	for (int i = 1; i < pData->nCount; i++)
	{
		if (hInst == NULL || m_pSlotData[i].hInst == hInst)
		{
			delete(CNoTrackObject*)pData->pData[i];
			pData->pData[i] = NULL;
		}
		else
		{
			if (pData->pData[i] != NULL)
			bDelete = FALSE;
		}
	}

	if (bDelete)
	{
		::EnterCriticalSection(&m_cs);
		m_list.Remove(pData);
		::LeaveCriticalSection(&m_cs);
		::LocalFree(pData->pData);
		delete pData;

		::TlsSetValue(m_tlsIndex, NULL);
	}

}

CThreadSlotData::~CThreadSlotData()
{
	CThreadData* pData = m_list;
	while (pData != NULL)
	{
		CThreadData* pDatanext = pData->pNext;
		DeleteValues(pData, NULL);
		pData = pData->pNext;
	}

	if (m_tlsIndex != (DWORD)-1)
		::TlsFree(m_tlsIndex);

	if (m_pSlotData!=NULL)
	{
		HGLOBAL hSlotData = ::GlobalHandle(m_pSlotData);
		::GlobalUnlock(hSlotData);
		::GlobalFree(m_pSlotData);
	}
	::DeleteCriticalSection(&m_cs);
}

CNoTrackObject* CThreadLocalObject::GetData(CNoTrackObject* (*pfnCreateObject)())
{

	if (m_nSlot == 0)//变量没分配槽号

	{

		if (_afxThreadData == NULL)

			_afxThreadData = new(__afxThreadData)CThreadSlotData;//此处new重构 只返回参数中的指针作为对象的首地址 BYTE __MYThreadData[sizeof(CThreadSlotData)];



		m_nSlot = _afxThreadData->AllocSlot();//变为1

	}

	CNoTrackObject* pValue = (CNoTrackObject*)_afxThreadData->GetThreadValue(m_nSlot);//当槽里没有东西时返回null

	if (pValue == NULL)
	{
		// 创建一个数据项
		pValue = (*pfnCreateObject)();
		// 使用线程私有数据保存新创建的对象

		_afxThreadData->SetValue(m_nSlot, pValue);

	}
	return pValue;
}
CNoTrackObject* CThreadLocalObject::GetDataNA()
{
	if (m_nSlot == 0 || _afxThreadData == 0)
		return NULL;
	return (CNoTrackObject*)_afxThreadData->GetThreadValue(m_nSlot);
}
CThreadLocalObject::~CThreadLocalObject()
{
	if (m_nSlot != 0 && _afxThreadData != NULL)
	{
		_afxThreadData->FreeSlot(m_nSlot);
		m_nSlot = 0;
	}
}
