#pragma once
#include "DKLibDef.h"

#include "DKObserver.h"
#include "DKStr2Val.h"
#include <hash_map>
#include <map>

class DKEnvironment;
class DK_API DKEnvironmentEventObserver : public DKObserver
{
public:
	virtual void OnEnvEventUpdated(INT key, const CString& value) = 0;
	virtual void OnEnvEventUpdated(const CString& key, const CString& value){}
};

class DK_API DKEnvironment
{
public:
	
	DKEnvironment(void);
	virtual ~DKEnvironment(void);

	virtual BOOL Init(LPCTSTR szIniFile);
	DKStr2Val Get(INT key);
	BOOL Set(INT key, const CString& value);
	BOOL Set(INT key, INT value);
	BOOL Set(INT key, DOUBLE value);
	virtual BOOL Set(INT key, const DKStr2Val& value);

	CString Get(CString key, CString strDefault = _T(""), BOOL bFlush = TRUE);
	BOOL Set(const CString& key, const CString& val, BOOL bFlush = TRUE);
	BOOL Set(LPCTSTR key, const CString& val) { return Set(CString(key), val); }
	void FlushSettings();
protected:
	void OnChanged(INT key, const CString& value); // 매니저에 의해 또느 DB에서 설정이 변경되면 호출 
	
	BOOL _Set(const CString& key, const CString& val, BOOL bFlush = FALSE);

	typedef std::hash_map<INT, DKStr2Val> MyMap;
	const MyMap& GetINTMap() { return m_map; }
private:
	
	MyMap m_map;

	std::map<CString, CString>  m_stringMap;

	CString		m_strSettingFile;
};

