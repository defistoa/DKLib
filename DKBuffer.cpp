#include "stdafx.h"
#include "DKBuffer.h"

//IMPLEMENT_OWN_HEAP(DKBuffer)

static DWORD GetPageSize()
{
	SYSTEM_INFO si;
	::GetSystemInfo(&si);
	return si.dwPageSize;
}
DWORD DKBuffer::s_dwDefSize = GetPageSize();
HANDLE DKBuffer::s_hBufHeap = NULL;
UINT DKBuffer::s_nObjCount = 0;

DKBuffer::DKBuffer(DWORD nInitSize) : m_dwParam(0)
{
	if(NULL == s_hBufHeap)
	{
		ASSERT(s_nObjCount == 0);
		s_hBufHeap = HeapCreate(0, 0, 0);
		if(s_hBufHeap == NULL)
		{
			throw DKSimpleException(_T("Heap Create Failed"));
		}
	}
	if (nInitSize < GetDefSize())
	{
		nInitSize = GetDefSize();
	}

	m_pBuf = (CHAR*)::HeapAlloc(s_hBufHeap, HEAP_ZERO_MEMORY, nInitSize);
	m_nBuffLen = nInitSize;
	m_pCur = m_pBuf;
	m_mode = M_WRITE;

	++s_nObjCount;
#ifdef _DEBUG
	m_nPos = 0;
#endif
}

DKBuffer::DKBuffer(CHAR* pBuf, int nLen) :
	m_pBuf(pBuf), m_nBuffLen(nLen), m_pCur(pBuf), m_mode(M_READ), m_dwParam(0)
{
#ifdef _DEBUG
	m_nPos = 0;
#endif
}

DKBuffer::~DKBuffer()
{
	Release();
}

// pBuf가 NULL이면 쓰기 버퍼로 작용한다.
BOOL DKBuffer::Set(CHAR* pBuf, UINT nLen)
{
	Init();
	if(NULL == pBuf)
	{
		if(NULL == s_hBufHeap)
		{
			ASSERT(s_nObjCount == 0);
			s_hBufHeap = HeapCreate(0, 0, 0);
			if(s_hBufHeap == NULL)
			{
				ASSERT(FALSE);
				return FALSE;
			}
		}
		++s_nObjCount;

		m_pBuf = (CHAR*)::HeapAlloc(s_hBufHeap, HEAP_ZERO_MEMORY, nLen);
		m_nBuffLen = nLen;
		m_pCur = m_pBuf;
		m_mode = M_WRITE;
	}
	else
	{
		m_pBuf = pBuf;
		m_nBuffLen = nLen;
		m_pCur = m_pBuf;
		m_mode = M_READ;
	}
#ifdef _DEBUG
	m_nPos = 0;
#endif
	return TRUE;
}


BOOL DKBuffer::SetPos(int nPos, BOOL bFromBegin)
{
	if(bFromBegin)
	{
		if(nPos > m_nBuffLen)
		{
			if(m_mode != M_WRITE)
				return FALSE;

			UINT nLen = nPos - m_nBuffLen;
			UINT nNewLen = ((nLen / 1024) + 1) * 1024;
			if(!IncreaseBuffer(nNewLen + m_nBuffLen))
				return FALSE;

		}
		m_pCur = m_pBuf + nPos;
#ifdef _DEBUG
		m_nPos = nPos;
#endif
	}
	else
	{
		if((int)GetPos() + nPos > m_nBuffLen)
		{
			if(m_mode != M_WRITE)
				return FALSE;

			UINT nLen = (GetPos() + nPos) - m_nBuffLen;
			UINT nNewLen = ((nLen / 1024) + 1) * 1024;
			if(!IncreaseBuffer(nNewLen + m_nBuffLen))
				return FALSE;
		}

		m_pCur += nPos;
#ifdef _DEBUG
		m_nPos += nPos;
#endif
	}
	return TRUE;
}


void DKBuffer::Init()
{
	Release();
	m_pBuf = NULL;
	m_pCur = NULL;
	m_mode = M_NONE;
	m_nBuffLen = 0;
#ifdef _DEBUG
	m_nPos = 0;
#endif
}

void DKBuffer::Release()
{
	if(m_pBuf && M_WRITE == m_mode)
	{
		::HeapFree(s_hBufHeap, 0, m_pBuf);
		if(--s_nObjCount == 0)
		{
			if(!::HeapDestroy(s_hBufHeap))
				ASSERT(FALSE);

			s_hBufHeap = NULL;
		}
	}
}

const DWORD dwMaxBufferLen = 1024 * 1024 * 2; //2M이상 금지

BOOL DKBuffer::IncreaseBuffer(UINT nNewLen)
{
	ASSERT(M_WRITE == m_mode);

	//if (nNewLen >= dwMaxBufferLen)
	//	return FALSE;

	if(nNewLen < m_nBuffLen)
		return TRUE;

	CHAR* p = (CHAR*)::HeapAlloc(s_hBufHeap, HEAP_ZERO_MEMORY, nNewLen);
	if(NULL == p)
		return FALSE;

	memcpy(p, m_pBuf, m_nBuffLen);

	int nPos = m_pCur - m_pBuf;
	::HeapFree(s_hBufHeap, 0, m_pBuf);

	m_pBuf = p;
	m_pCur = m_pBuf + nPos;
	m_nBuffLen = nNewLen;
	return TRUE;
}
