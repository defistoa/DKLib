// FileToMem.h

#pragma once
#include "DKLibDef.h"

class DK_API DKFileToMem
{
public:
	DKFileToMem();
	~DKFileToMem();

	BOOL Open(LPCTSTR strFileName);
	LONGLONG Seek(LONGLONG lOff, UINT nFrom);
	UINT Read(void* lpBuf, UINT nCount);
	ULONGLONG GetPosition() const
	{
		return m_pos;
	}
	ULONGLONG GetLength() const
	{
		return m_len;
	}
	BOOL IsOpen();
	void Close();
protected:
	BOOL PrepareBlock(ULONGLONG pos, UINT nCount);
private:
	CFile		m_file;
	HANDLE		m_hHeap;
	CHAR*		m_pBuf;
	ULONGLONG	m_pos;
	ULONGLONG	m_len;

	LONGLONG	m_nBegin;
	LONGLONG	m_nEnd;
};

