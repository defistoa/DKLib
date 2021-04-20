#include "StdAfx.h"
#include "DKEnvironment.h"
#include "DK_API.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



DKEnvironment::DKEnvironment(void)
{
}


DKEnvironment::~DKEnvironment(void)
{
	
}


BOOL DKEnvironment::Init(LPCTSTR szIniFile)
{
	m_strSettingFile = szIniFile;
	CStdioFile file;
	TRY
	{
		if(file.Open(m_strSettingFile, CFile::modeRead))
		{
			CString str;
			while(file.ReadString(str))
			{
				CString key, val;
				::ParseLine(str, key, val, _T('='));
				key.MakeUpper();

				CHKINS(m_stringMap.insert(make_pair(key, val)));
			}
			return TRUE;
		}
		return FALSE;
	}
	CATCH(CException, pEx)
	{
		TCHAR sz[256];
		pEx->GetErrorMessage(sz, sizeof(TCHAR) * 256);
		throw DKSimpleException(sz);
	}
	END_CATCH
	return TRUE;
}

void DKEnvironment::FlushSettings()
{
	CStdioFile file;
	if(file.Open(m_strSettingFile, CFile::modeCreate|CFile::modeReadWrite))
	{
		for(auto pos = m_stringMap.begin() ; pos != m_stringMap.end() ; ++pos)
		{
			CString str;
			str.Format(_T("%s=%s\n"), pos->first, pos->second);
			file.WriteString(str);
		}
	}
}


DKStr2Val DKEnvironment::Get(INT key)
{
	auto pos = m_map.find(key);
	if(m_map.end() == pos)
	{
		ASSERT(FALSE);
		return DKStr2Val();
	}
	return pos->second;
}

BOOL DKEnvironment::Set(INT key, const CString& value)
{
	Set(key, DKStr2Val(value));
	return TRUE;
}

BOOL DKEnvironment::Set(INT key, INT value)
{
	Set(key, DKStr2Val(value));
	return TRUE;
}
BOOL DKEnvironment::Set(INT key, DOUBLE value)
{
	Set(key, DKStr2Val(value));
	return TRUE;
}

BOOL DKEnvironment::Set(INT key, const DKStr2Val& value)
{
	auto pos = m_map.find(key);
	if(pos != m_map.end())
	{
		if(pos->second == value)
			return FALSE;

		pos->second = value;
	}
	else
		m_map.insert(make_pair(key, value));
	return TRUE;
}


BOOL DKEnvironment::Set(const CString& key, const CString& val, BOOL bFlush)
{
	return _Set(key, val, bFlush);
}

BOOL DKEnvironment::_Set(const CString& key, const CString& val, BOOL bFlush)
{
	CString _key = key;
	_key.MakeUpper();

	auto pos = m_stringMap.find(key);
	if(pos != m_stringMap.end())
	{
		if(pos->second == val)
			return FALSE;
		pos->second = val;
	}
	else
		m_stringMap.insert(make_pair(_key, val));

	if(bFlush)
		FlushSettings();

	return TRUE;
}

CString DKEnvironment::Get(CString key, CString strDefault, BOOL bFlush)
{
	key.MakeUpper();
	auto pos = m_stringMap.find(key);
	if(pos == m_stringMap.end())
	{
		if(!strDefault.IsEmpty())
		{
			_Set(key, strDefault, bFlush);
			return strDefault;
		}
		return _T("");
	}

	return pos->second;
}

