#include "StdAfx.h"
#include "Logger.h"
#include "DK_API.h"

Logger::Logger(void) : m_fncNotify(Logger::NotifyDummy)
{
}


Logger::~Logger(void)
{
}


BOOL Logger::Init(LPCTSTR szRoot)
{
	if(m_files[0] != CFile::hFileNull)
		return FALSE;
	
	CTime tm(CTime::GetCurrentTime());
	CString strRoot;

#if 1
	if(!::IsExistDirectory(szRoot))
	{
		ASSERT(FALSE);
		return FALSE;
	}
	strRoot.Format(_T("%s\\%s"), szRoot, tm.Format(_T("%Y%m%d_%H%M%S.txt")));
	CFileException ex;
	if(!m_files[0].Open(strRoot, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite, &ex))
		return FALSE;
#else
	strRoot.Format(_T("%s\\%s"), szRoot, tm.Format(_T("%Y%m%d_%H%M%S")));

	if(!::IsExistDirectory(strRoot))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CFileException ex;
	for(UINT i = 0 ; i < lt_count ; ++i)
	{
		CString sType;
		switch(i)
		{
		case lt_none: sType = _T("ALL"); break;
		case lt_info: sType = _T("INF"); break;
		case lt_sys_err: sType = _T("SYS"); break;
		case lt_network_err: sType = _T("NET"); break;
		case lt_db_err: sType = _T("DB"); break;
		case lt_secure_err: sType = _T("SEC"); break;
		case lt_act: sType = _T("ACT"); break;
		case lt_diag: sType = _T("DIAG"); break;
		case lt_order: sType = _T("ORD"); break;
#ifdef _DEBUG
		case lt_devel: sType = _T("DEV"); break;
#endif
		default:
			ASSERT(FALSE);
		}
		CString strFilePath;
		strFilePath.Format(_T("%s\\%s.txt"), strRoot, sType);

		if(!m_files[i].Open(strFilePath, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite, &ex))
			return FALSE;
	}
#endif
	return TRUE;
}

void Logger::Log(LogType type, LPCTSTR szFmt, ...)
{
	if(m_files[0] == CFile::hFileNull)
		return;

	CString s;
	CString strText;

	va_list argptr;
	va_start(argptr, szFmt);
	strText.FormatV(szFmt, argptr);
	va_end(argptr);

	SYSTEMTIME st;
	::GetLocalTime(&st);
	
	s.Format(_T("%02d:%02d:%02d.%03d\t%s\r\n"), st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, strText);
		
	m_files[0].WriteString(s);
//	m_files[type].WriteString(s);

	m_files[0].Flush();
//	m_files[type].Flush();

	m_fncNotify(s);
	TRACE(s);
}

void Logger::LogD(LPCTSTR szFnc, int line, LPCTSTR szFmt, ...)
{
	if(m_files[0] == CFile::hFileNull)
		return;

	CString s;
	CString strText;

	va_list argptr;
	va_start(argptr, szFmt);
	strText.FormatV(szFmt, argptr);
	va_end(argptr);

	SYSTEMTIME st;
	::GetLocalTime(&st);

	s.Format(_T("%02d:%02d:%02d.%03d\t(%s[%d]) %s\r\n"), st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, szFnc, line, strText);
		
	m_files[0].WriteString(s);
//	m_files[lt_devel].WriteString(s);
	m_files[0].Flush();
//	m_files[lt_devel].Flush();
	m_fncNotify(s);
	TRACE(s);
}
