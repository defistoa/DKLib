
#pragma once
#include "DKLibDef.h"
#include "DKcommon.h"

#include <boost/noncopyable.hpp>

//////////////////////////////////////////////////////////////////////////////////////
class DK_API DKBuffer : private boost::noncopyable
{
//	DECLARE_OWN_HEAP()

	enum Mode
	{
		M_NONE = -1,
		M_WRITE = 0,
		M_READ = 1,
	};
public:
	DKBuffer(DWORD nInitSize = DKBuffer::s_dwDefSize);
	DKBuffer(CHAR* pBuf, int nLen);
	~DKBuffer();

	// pBuf가 NULL이면 쓰기 버퍼로 작용한다.
	BOOL Set(CHAR* pBuf, UINT nLen);

	CHAR* Get(int nPos = 0)
	{
		return &m_pBuf[nPos];
	}

	CHAR* GetCur()
	{
		return m_pCur;
	}

	BOOL SetPos(int nPos, BOOL bFromBegin = TRUE);
	
	UINT GetPos()
	{
		return m_pCur - m_pBuf;
	}
	UINT GetLength()
	{
		return m_mode == M_READ ? m_nBuffLen : m_pCur - m_pBuf;
	}

	UINT GetBufferLength()
	{
		return m_nBuffLen;
	}
	UINT GetEnableWrite()
	{
		if(M_WRITE != m_mode)
			return 0;
		return m_nBuffLen;
	}

	BOOL RD(LPVOID pDest, int nLen)
	{
		ASSERT(m_pBuf);
		if( ((m_pCur - m_pBuf) + nLen) > m_nBuffLen)
			return FALSE;

		memcpy_s(pDest, nLen, m_pCur, nLen); 
		m_pCur += nLen;
#ifdef _DEBUG
		m_nPos += nLen;
#endif

		return TRUE;
	}

	template<class T>
	BOOL RD(T& var)
	{
		return RD(&var, sizeof(var));
	}

	BOOL RD(CStringW& str)
	{
		ASSERT(m_pBuf);
		char cookie;
		RD(cookie);
		if(cookie != 'U')
		{
			ASSERT(FALSE);
			return FALSE;
		}
		str.Empty();
		UINT nLen;
		if(!RD(nLen))
			return FALSE;
		if(nLen > 4096)
		{
			ASSERT(FALSE);
			return FALSE;
		}
		/*if(GetPos() + nLen >= GetLength())
			return FALSE;*/
		if(nLen == 0)
		{
			str = L"";
			return TRUE;
		}

		str.Append((LPCWSTR)GetCur(), nLen);
		SetPos(nLen * sizeof(WCHAR), FALSE);
		return TRUE;
	}

	BOOL RD(CStringA& str)
	{
		ASSERT(m_pBuf);
		char cookie;
		RD(cookie);
		if(cookie != 'M')
		{
			ASSERT(FALSE);
			return FALSE;
		}
		str.Empty();
		UINT nLen;
		if(!RD(nLen))
			return FALSE;
		if(nLen > 4096)
		{
			ASSERT(FALSE);
			return FALSE;
		}
		/*if(GetPos() + nLen >= GetLength())
			return FALSE;*/
		if(nLen == 0)
		{
			str = L"";
			return TRUE;
		}

		str.Append((LPCSTR)GetCur(), nLen);
		SetPos(nLen, FALSE);
		return TRUE;
	}
		
	BOOL WT(LPVOID p, int nLen)
	{
		ASSERT(m_pBuf);
#ifdef _DEBUG
		if(M_WRITE != m_mode)
		{
			ASSERT(FALSE);
			return FALSE;
		}
#endif
		if(((int)GetPos() + nLen) > m_nBuffLen)
		{
			UINT nNewLen = ((nLen / 1024) + 1) * 1024;
			if(!IncreaseBuffer(nNewLen + m_nBuffLen))
				return FALSE;
		}

		memcpy(m_pCur, p, nLen);
		m_pCur += nLen;
#ifdef _DEBUG
		m_nPos += nLen;
#endif
		return TRUE;
	}
	template<class T>
	BOOL WT(T var)
	{
		return WT(&var, sizeof(var));
	}

	BOOL WT(CStringW str)
	{
		WT('U');
		int nLen = str.GetLength();
		if(!WT(nLen))
			return FALSE;

		if(0 == nLen)
			return TRUE;

		return WT((LPVOID)(LPCWSTR)str.GetBuffer(), nLen * sizeof(WCHAR));
	}

	BOOL WT(LPCWSTR pStr)
	{
		return WT(CStringW(pStr));
	}
	BOOL WT(WCHAR* pStr)
	{
		return WT(CStringW(pStr));
	}

	BOOL WT(CStringA str)
	{
		WT('M');
		int nLen = str.GetLength();
		if(!WT(nLen))
			return FALSE;

		if(0 == nLen)
			return TRUE;

		return WT((LPVOID)(LPCSTR)str.GetBuffer(), nLen);
	}

	BOOL WT(LPCSTR pStr)
	{
		return WT(CStringA(pStr));
	}
	BOOL WT(CHAR* pStr)
	{
		return WT(CStringA(pStr));
	}

	static DWORD GetDefSize() { return s_dwDefSize; }
	BOOL IsValid() { return m_pBuf != NULL; }
	BOOL IncreaseBuffer(UINT nNewLen);

	void SetParam(DWORD dwParam) { m_dwParam = dwParam; }
	DWORD GetParam() { return m_dwParam; }
private:
	void Init();
	void Release();
private:
	CHAR*	m_pBuf;
	CHAR*	m_pCur;
	int		m_nBuffLen;
	Mode	m_mode;
	DWORD	m_dwParam;
#ifdef _DEBUG
public:
#endif
	static DWORD	s_dwDefSize;
	static HANDLE	s_hBufHeap;
	static UINT		s_nObjCount;
#ifdef _DEBUG
	int		m_nPos;
#endif
};