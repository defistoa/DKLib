// FileToMem.cpp

#include "stdafx.h"
#include "DKcommon.h"
#include "DKFileToMem.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const ULONGLONG MAX_BUF = 32 * 1024 * 1024; // 64MB

DKFileToMem::DKFileToMem()
{
	m_pBuf = NULL;
	m_pos = 0;
	m_len = 0;
	m_hHeap = NULL;
	m_nBegin = NUL_IDX;
	m_nEnd = NUL_IDX;
}

DKFileToMem::~DKFileToMem()
{
	Close();
}
void DKFileToMem::Close()
{
	if(m_pBuf)
		HeapFree(m_hHeap, 0, m_pBuf);
	if(m_hHeap)
		HeapDestroy(m_hHeap);

	if(CFile::hFileNull != m_file)
		m_file.Close();

	m_pBuf = NULL;
	m_hHeap = NULL;
}

BOOL DKFileToMem::Open(LPCTSTR strFileName)
{
	Close();

	CFileException ex;
	if(!m_file.Open(strFileName, CFile::modeRead|CFile::shareDenyWrite, &ex))
	{
		return FALSE;
	}

	if(CFile::hFileNull == m_file)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	ULONGLONG nFileSize = m_file.GetLength();
	if(nFileSize == 0)
		return FALSE;

	m_len = nFileSize;
	m_hHeap = ::HeapCreate(0, 0, 0);
	if(NULL == m_hHeap)
		return FALSE;

	SIZE_T nSize = m_len > MAX_BUF ? (SIZE_T)MAX_BUF : (SIZE_T)m_len;
	
	m_pBuf = (CHAR*)HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY, nSize);
	if(NULL == m_pBuf)
	{
		HeapDestroy(m_hHeap);
		m_hHeap = NULL;
		return FALSE;
	}

	PrepareBlock(0, 0);

	return TRUE;
}

BOOL DKFileToMem::PrepareBlock(ULONGLONG pos, UINT nCount)
{
 	ASSERT(NULL != m_hHeap);

	ULONGLONG nEnd = pos + (ULONGLONG)nCount;

	if(m_nBegin <= pos && pos < m_nEnd && nEnd < m_nEnd)
		return TRUE;
		
	ULONGLONG nBlock = pos / MAX_BUF;

	m_nBegin = nBlock * MAX_BUF;
	m_nEnd = m_nBegin + MAX_BUF > m_len ? m_nBegin + (m_len - m_nBegin) : m_nBegin + MAX_BUF;
	
	if(m_nEnd < pos + nCount)
		m_nEnd = pos + nCount;
	
	ULONGLONG l = m_file.Seek(m_nBegin, CFile::begin);
	if(l != m_nBegin)
		return FALSE;
	
	if(m_file.Read(m_pBuf, (UINT)(m_nEnd - m_nBegin)) != (UINT)(m_nEnd - m_nBegin))
		return FALSE;
	
	return TRUE;
}


LONGLONG DKFileToMem::Seek(LONGLONG lOff, UINT nFrom)
{
	ASSERT(m_pBuf);

	if(nFrom == CFile::begin)
	{
		if(lOff >= m_len)
		{
			ASSERT(FALSE);
			m_pos = m_len;
			return -1;
		}

		m_pos = lOff;
	}
	else if(nFrom == CFile::current)
	{
		if(m_pos + lOff > m_len)
		{
			ASSERT(FALSE);
			m_pos = m_len;
			return -1;
		}
		m_pos += lOff;
	}
	else if(nFrom == CFile::end)
	{
		if(lOff >= m_len)
		{
			ASSERT(FALSE);
			m_pos = m_len;
			return -1;
		}
		m_pos = m_len - lOff;
	}
	
	if(FALSE == PrepareBlock(m_pos, 0))
		return -1;

	return m_pos;
}

UINT DKFileToMem::Read(void* lpBuf, UINT nCount)
{
	ASSERT(m_pBuf);
	
	if(m_pos + nCount > m_len)
		return 0;

	if(FALSE == PrepareBlock(m_pos, nCount))
		return 0;
	
	memcpy(lpBuf, m_pBuf + (m_pos - m_nBegin), nCount);

	m_pos += (ULONGLONG)nCount;

	return nCount;
}

BOOL DKFileToMem::IsOpen()
{
	return CFile::hFileNull != m_file;
}