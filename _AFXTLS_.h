#ifndef __AFXTLS_H__ // ����AFXTLS_H
#define __AFXTLS_H__
#include <windows.h>
#include <process.h>
#include <iostream>
#include <stddef.h>
using namespace std;
class CSimpleList
{
public:
	CSimpleList(int nNextOffset = 0);
	void Construct(int nNextOffset);
	// �ṩ���û��Ľӿں�����Operations����������ӡ�ɾ���ͱ����ڵ�
	BOOL IsEmpty() const;
	void AddHead(void* p);
	void RemoveAll();
	void* GetHead() const;
	void* GetNext(void* p) const;
	BOOL Remove(void* p);

	// Ϊʵ�ֽӿں�������ĳ�Ա��Implementation��
	void* m_pHead;      // �����е�һ��Ԫ�صĵ�ַ
	size_t m_nNextOffset;   // ���ݽṹ��pNext��Ա��ƫ����
	void** GetNextPtr(void* p) const;
};

// �����������
inline CSimpleList::CSimpleList(int nNextOffset)
{
	m_pHead = NULL; m_nNextOffset = nNextOffset;
}
inline void CSimpleList::Construct(int nNextOffset)
{
	m_nNextOffset = nNextOffset;
}
inline BOOL CSimpleList::IsEmpty() const
{
	return m_pHead == NULL;
}
inline void CSimpleList::RemoveAll()
{
	m_pHead = NULL;
}
inline void* CSimpleList::GetHead() const
{
	return m_pHead;
}
inline void* CSimpleList::GetNext(void* preElement) const
{
	return *GetNextPtr(preElement);
}
inline void** CSimpleList::GetNextPtr(void* p) const
{
	return (void**)((BYTE*)p + m_nNextOffset);
}

template<class TYPE>
class CTypedSimpleList : public CSimpleList
{
public:
	CTypedSimpleList(int nNextOffset = 0)
		: CSimpleList(nNextOffset) { }
	void AddHead(TYPE p)
	{
		CSimpleList::AddHead((void*)p);
	}
	TYPE GetHead()
	{
		return (TYPE)CSimpleList::GetHead();
	}
	TYPE GetNext(TYPE p)
	{
		return (TYPE)CSimpleList::GetNext(p);
	}
	BOOL Remove(TYPE p)
	{
		return CSimpleList::Remove(p);
	}
	operator TYPE()
	{
		return (TYPE)CSimpleList::GetHead();
	}
};

class CNoTrackObject
{
public: //�ڴ˳�����û�м�public Ĭ��Ϊ���˽�б�����MyThreadData�̳�������Ҳ�޷����ʳ�Ա����	
	void* operator new(size_t nSize);	
	void operator delete(void*);	
	virtual ~CNoTrackObject(){}
};


struct MyThreadData
{
	MyThreadData* pNext;
	int nSomeData;
};

struct CSlotData;
struct CThreadData;
class CThreadSlotData
{
public:
	CThreadSlotData();
	int AllocSlot();
	void FreeSlot(int nSlot);
	void* GetThreadValue(int nSlot);
	void SetValue(int nSlot, void* pValue);
	void DeleteValues(HINSTANCE hInst, BOOL bAll = FALSE);
	DWORD m_tlsIndex;

	int m_nAlloc;
	int m_nRover;
	int m_nMax;
	CSlotData* m_pSlotData;
	CTypedSimpleList<CThreadData*> m_list;
	CRITICAL_SECTION m_cs;

	void* operator new(size_t,void* p)
	{
		return p;
	}
	void DeleteValues(CThreadData* pData, HINSTANCE hInst);
	~CThreadSlotData();
};

class CThreadLocalObject
{
public:
	CNoTrackObject* GetData(CNoTrackObject* (*pfnCreateObject)());
	CNoTrackObject* GetDataNA();

	DWORD m_nSlot;
	~CThreadLocalObject();
};

template<class TYPE>

class CThreadLocal : public CThreadLocalObject

{

	// ���Գ�Ա��Attributes��

public:

	TYPE* GetData()

	{

		TYPE* pData = (TYPE*)CThreadLocalObject::GetData(&CreateObject);//���ຯ�� 

		return pData;

	}

	TYPE* GetDataNA()

	{

		TYPE* pData = (TYPE*)CThreadLocalObject::GetDataNA();

		return pData;

	}

	operator TYPE*()

	{
		return GetData();
	}

	TYPE* operator->()

	{
		return GetData();
	}



	// ����ʵ�֣�Implementation��

public:

	static CNoTrackObject* CreateObject()

	{
		return new TYPE;
	}

};

#endif // __AFXTLS_H__