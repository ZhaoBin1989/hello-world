#ifndef __AFXTLS_H__ // ——AFXTLS_H
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
	// 提供给用户的接口函数（Operations），用于添加、删除和遍历节点
	BOOL IsEmpty() const;
	void AddHead(void* p);
	void RemoveAll();
	void* GetHead() const;
	void* GetNext(void* p) const;
	BOOL Remove(void* p);

	// 为实现接口函数所需的成员（Implementation）
	void* m_pHead;      // 链表中第一个元素的地址
	size_t m_nNextOffset;   // 数据结构中pNext成员的偏移量
	void** GetNextPtr(void* p) const;
};

// 类的内联函数
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
public: //在此出过错，没有加public 默认为类的私有变量，MyThreadData继承这个类后也无法访问成员变量	
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

	// 属性成员（Attributes）

public:

	TYPE* GetData()

	{

		TYPE* pData = (TYPE*)CThreadLocalObject::GetData(&CreateObject);//父类函数 

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



	// 具体实现（Implementation）

public:

	static CNoTrackObject* CreateObject()

	{
		return new TYPE;
	}

};

#endif // __AFXTLS_H__