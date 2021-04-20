
#pragma once

#include "DKcommon.h"
#include "DKobject.h"
#include "DK_API.h"
#include "DK_DECIMAL.h"

class DKStr2Val
{
public:
	enum ValType
	{
		_vt_none = -1,
		_vt_string,
		_vt_int,
		_vt_double,
		_vt_longlong,
		_vt_decimal,
	};

	DKStr2Val() : m_type(_vt_none) {}
	DKStr2Val(LPCTSTR val) :m_str(val), m_type(_vt_string) {}
	DKStr2Val(INT val) : m_type(_vt_int)
	{
		m_int = val;
		m_str = ::ToString(val);
	}

	DKStr2Val(DOUBLE val) : m_type(_vt_double)
	{
		m_double = val;
		m_str = ::ToString(val, 4);
	}

	DKStr2Val(LONGLONG val) : m_type(_vt_longlong)
	{
		m_longlong = val;
		m_str = ::ToString(val);
	}

	DKStr2Val(DK_DECIMAL val, int nPrecision) : m_type(_vt_decimal)
	{
		m_decimal = val;
		m_str = ::ToString(val, nPrecision);
	}

	ValType GetType() { return m_type; }
	operator LPCTSTR() { return (LPCTSTR)m_str; }
	operator INT() { return ToINT(); }
	operator DOUBLE() { return ToDOUBLE(); }
	operator LONGLONG() { return ToLONGLONG(); }

	DOUBLE ToDOUBLE()
	{
		if(false == (bool)m_double)
		{
			m_double = ::ToDOUBLE(m_str);
		}
		return (DOUBLE)m_double;
	}

	INT ToINT()
	{
		if(false == (bool)m_int)
		{
			m_int = ::ToINT(m_str);
		}
		return (INT)m_int;
	}
	LONGLONG ToLONGLONG()
	{
		if(false == (bool)m_longlong)
		{
			m_longlong = ::ToLONGLONG(m_str);
		}
		return (LONGLONG)m_longlong;
	}
	DK_DECIMAL ToDecimal()
	{
		if(false == (bool)m_decimal)
		{
			m_decimal = ::ToDecimal(m_str);
		}
		return (DK_DECIMAL)m_decimal;
	}

	INT DoubleToInt()
	{
		if(false == (bool)m_doubleToInt)
		{
			CString s = m_str;
			s.Remove(_T('.'));
			m_doubleToInt = ::ToINT(s);
		}
		return m_doubleToInt;
	}

	CString ToString()
	{
		return m_str;
	}

	const CString& ToString() const 
	{
		return m_str;
	}
	DKStr2Val& operator =(CString rhs)
	{
		m_str = rhs;
		m_type = _vt_string;
		m_double.SetNull();
		m_int.SetNull();
		m_longlong.SetNull();
		return *this;
	}

	DKStr2Val& operator =(CStringA rhs)
	{
		m_str = rhs;
		m_type = _vt_string;
		m_double.SetNull();
		m_int.SetNull();
		m_longlong.SetNull();
		return *this;
	}

	DKStr2Val& operator =(const DKStr2Val& rhs)
	{
		m_str = rhs.m_str;
		m_type = rhs.m_type;
		m_double = rhs.m_double;
		m_int = rhs.m_int;
		m_longlong = rhs.m_longlong;
		return *this;
	}

	friend bool operator == (const DKStr2Val& lhs, const DKStr2Val& rhs);
	friend bool operator != (const DKStr2Val& lhs, const DKStr2Val& rhs);
	friend bool operator < (const DKStr2Val& lhs, const DKStr2Val& rhs);
	friend bool operator > (const DKStr2Val& lhs, const DKStr2Val& rhs);

	friend bool operator == (const DKStr2Val& lhs, const CString& rhs);
	friend bool operator != (const DKStr2Val& lhs, const CString& rhs);
	friend bool operator < (const DKStr2Val& lhs, const CString& rhs);
	friend bool operator > (const DKStr2Val& lhs, const CString& rhs);

	friend bool operator == (const CString& lhs, const DKStr2Val& rhs);
	friend bool operator != (const CString& lhs, const DKStr2Val& rhs);
	friend bool operator < (const CString& lhs, const DKStr2Val& rhs);
	friend bool operator > (const CString& lhs, const DKStr2Val& rhs);
private:
	CString			m_str;
	ValType			m_type;
	DKOptional<DOUBLE>		m_double;
	DKOptional<INT>			m_int;
	DKOptional<DK_DECIMAL>	m_decimal;
	DKOptional<LONGLONG>	m_longlong;
	DKOptional<INT>			m_doubleToInt;
};

static bool operator == (const DKStr2Val& lhs, const DKStr2Val& rhs)
{
	return lhs.m_str == rhs.m_str;
}
static bool operator != (const DKStr2Val& lhs, const DKStr2Val& rhs)
{
	return lhs.m_str != rhs.m_str;
}

static bool operator < (const DKStr2Val& lhs, const DKStr2Val& rhs)
{
	return lhs.m_str < rhs.m_str;
}

static bool operator > (const DKStr2Val& lhs, const DKStr2Val& rhs)
{
	return lhs.m_str > rhs.m_str;
}

static bool operator == (const DKStr2Val& lhs, const CString& rhs)
{
	return lhs.m_str == rhs;
}
static bool operator != (const DKStr2Val& lhs, const CString& rhs)
{
	return lhs.m_str != rhs;
}

static bool operator < (const DKStr2Val& lhs, const CString& rhs)
{
	return lhs.m_str < rhs;
}

static bool operator > (const DKStr2Val& lhs, const CString& rhs)
{
	return lhs.m_str > rhs;
}

static bool operator == (const CString& lhs, const DKStr2Val& rhs)
{
	return lhs == rhs.m_str;
}
static bool operator != (const CString& lhs, const DKStr2Val& rhs)
{
	return lhs != rhs.m_str;
}

static bool operator < (const CString& lhs, const DKStr2Val& rhs)
{
	return lhs < rhs.m_str;
}

static bool operator > (const CString& lhs, const DKStr2Val& rhs)
{
	return lhs > rhs.m_str;
}

///////////////////////////////////////////////////////////////////////////

