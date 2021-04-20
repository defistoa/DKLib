#pragma once
#include "DKLibDef.h"

#include <vector>
#include <afxmt.h>

///////////////////////////////////////////////////////////////////////////
//
class DK_API DKScopeSync
{
public:
	DKScopeSync(CSyncObject& a_sync) : m_sync(a_sync)
	{
		Lock();
	}
	~DKScopeSync()
	{
		Unlock();
	}

	void Lock() { m_sync.Lock(); }
	void Unlock() { m_sync.Unlock(); }
private:
	CSyncObject& m_sync;
};

#define SCOPE_LOCK(__cs__) DKScopeSync __sync__(__cs__)


///////////////////////////////////////////////////////////////////////////
//
class DK_API DKPerformanceCounter
{
public:
	LARGE_INTEGER	m_start, m_end;

	void Start()
	{
		QueryPerformanceCounter(&m_start);
	}
	void End()
	{
		QueryPerformanceCounter(&m_end);
	}
	double Second()
	{
		LARGE_INTEGER frq;
		QueryPerformanceFrequency(&frq);

		return (m_end.QuadPart - m_start.QuadPart)/(double)frq.QuadPart;
	}
	LONGLONG Tick()
	{
		return m_end.QuadPart - m_start.QuadPart;
	}
};



///////////////////////////////////////////////////////////////////////////
//
class DKSimpleException
{
	CString m_msg;
public:

	DKSimpleException(LPCTSTR szFmt, ...)
	{
		va_list argptr;
		va_start(argptr, szFmt);
		m_msg.FormatV(szFmt, argptr);
		va_end(argptr);
	}
	const CString& GetMessage() { return m_msg; }
};


//////////////////////////////////////////////////////////////////////////////////////
class Tokenizer : public std::vector<CString>
{
public:
	Tokenizer(const CString& strIn, LPCTSTR token = _T(";"))
	{
		int nPos = 0;
		CString item;
		while(TRUE)
		{
			item = strIn.Tokenize(token, nPos);
			if(item.IsEmpty())
				break;

			push_back(item);
		}
	}
};

//////////////////////////////////////////////////////////////////////////////////////

struct DK_API LineType
{
	COLORREF	color;
	int			style;
	int			width;

	LineType(COLORREF cr = RGB(0,0,0), int s = PS_SOLID, int w = 1)
		: color(cr), width(w), style(s)
	{
	}

	void Create(CPen& pen)
	{
		pen.DeleteObject();
		pen.CreatePen(style, width, color);
	}
	bool operator == (const LineType& rhs) const
	{
		return color == rhs.color && width == rhs.width && style == rhs.style;
	}
	bool operator != (const LineType& rhs) const
	{
		return !(*this == rhs);
	}
	void Set(COLORREF cr = RGB(0,0,0), int s = PS_SOLID, int w = 1)
	{
		color = cr;
		style = s;
		width = w;
	}
};

//////////////////////////////////////////////////////////////////////////////////////
template<class T>
class DKOptional
{
	T		val;
	BOOL	bSet;

	typedef typename DKOptional<T> _MyT;
public:
	DKOptional() : bSet(FALSE) 
	{
	}
	~DKOptional() 
	{ 
	}
	
	DKOptional(T v) : val(v), bSet(TRUE)
	{
	}
	DKOptional(const _MyT& rhs) : val(rhs.val), bSet(rhs.bSet)
	{
	}

	_MyT& operator=(T rhs) 
	{
		val = rhs;
		bSet = TRUE;
		return *this;
	}

	_MyT& operator=(const _MyT& rhs) 
	{
		val = rhs.val;
		bSet = rhs.bSet;
		return *this;
	}

	operator T()
	{
		//ASSERT(val);
		return val;
	}

	operator bool()
	{
		return bSet != FALSE;
	}

	void SetNull()
	{
		bSet = FALSE;
	}

	BOOL IsNull() 
	{
		return bSet;
	}
};
