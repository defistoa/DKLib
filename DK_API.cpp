
#include "stdafx.h"
#include "DK_API.h"
#include "DKcommon.h"
#include <cmath>

#include <sstream> 
#include <iomanip>
#include <locale>
#include <ATLComTime.h>
#include <Iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#include <mmsystem.h>
#include <imm.h>

using namespace std;

#if 0
template<class T>
std::string FormatWithCommas(T value)
{
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
}

CString ToCurrency(LONGLONG a_nVal)
{
	CString str;
	LONGLONG abVal = _abs64(a_nVal);

	if(CHEON(1) > abVal)
		str.Format(_T("%d"), a_nVal);
	else if(MAHN(100) > abVal)
	{
		str.Format(_T("%d,%03d"), (int)a_nVal/CHEON(1), (int)abVal%CHEON(1));
	}
	else if(EOK(10) > abVal)
	{
		str.Format(_T("%d,%03d,%03d"), (int)a_nVal/MAHN(100), (int)(abVal%MAHN(100))/CHEON(1), (int)abVal%CHEON(1));
	}
	else
	{
		int count = 0;
		
		LONGLONG temp = a_nVal;
		while(TRUE)
		{
			temp = temp/CHEON(1);
			++count;
			if(0 == temp)
				break;
		}

		LONGLONG denom = CHEON(1);
		for(int i = 0 ; i < count; ++i)
		{
			CString strTemp;
			if(i == count -1)
			{
				strTemp.Format(_T("%d"), (int)(a_nVal/denom));
			}
			else if(i == 0)
			{
				strTemp.Format(_T(",%03d"), (int)(abVal%denom));
			}
			else
			{
				strTemp.Format(_T(",%03d"), (int)((abVal%(denom*CHEON(1)))/denom));
				denom *= CHEON(1);
			}
			str = strTemp + str;
		}
	}

	return str;
}

CString ToCurrency(DOUBLE a_val)
{
	DOUBLE nInteger;
	DOUBLE dFraction = modf(a_val, &nInteger);

	if(fabs(dFraction) > 0.99)
	{
		if(dFraction > 0)
			nInteger += 1;
		else 
			nInteger -= 1;
	}

	return ToCurrency((LONGLONG)nInteger);
}
#endif
#if 0
CString ToCurrency(const CString& str)
{
	ASSERT(!str.IsEmpty());
	BOOL bNeg = str[0] == _T('-');
	const int nLen = str.GetLength() - (bNeg ? 1 : 0);
	if(nLen <= 3)
		return str;
	int nDot = str.Find(_T('.'));
	if(-1 != nDot && bNeg)
		--nDot;
		
	int nCommas = ((nLen / 3) - 1) + ((nLen%3)>0 ? 1 : 0);

	CString sRet;
	int nCount = str.GetLength() - (3 * nCommas);
	sRet.Format(_T("%s,"), str.Mid(0, nCount));
	int nBegin = nCount;
	for(int i = 0 ; i < nCommas ; ++i)
	{
		if(nBegin + 3 == nDot)
		{
			sRet += str.Mid(nBegin, str.GetLength() - nBegin);
			break;
		}

		sRet += str.Mid(nBegin, 3);
		nBegin += 3;

		if(nCommas - 1 != i)
			 sRet += _T(",");
	}
	return sRet;
}
#endif

#if 0
CString FloatToComaString(DOUBLE a_val)
{
	CString str;
	if((-1 < a_val && a_val < 0) || (0 < a_val && a_val < 1))
		str.Format(_T("%.4f"), a_val);
	else
	{
		str = ToCurrency((LONGLONG)a_val);
		CString strTemp;
		strTemp.Format(_T("%.3f"), a_val);
		str+= strTemp.Right(4);
	}
	return str;
}
#endif

BOOL ParseLine(const CString& strIn, CString& left, CString& right, TCHAR token)
{
	int nIndex = strIn.Find(token);
	if(nIndex == NUL_IDX)
		return FALSE;

	left=strIn.Left(nIndex);
	right=strIn.Mid(nIndex+1, strIn.GetLength() - nIndex+1);
	return TRUE;
}

BOOL ParseLineSep(const CString& strIn, vector<CString>& items, TCHAR token, BOOL bIgnoreEmpty)
{
	items.clear();
	int nCount = strIn.GetLength();
	if(nCount == 0)
	{
		return TRUE;
	}

	int nBegin = 0;
	for(int i = 0 ; i < nCount ; ++i)
	{
		if(strIn[i] == token)
		{
			CString str = strIn.Mid(nBegin, i - nBegin);
			if(!bIgnoreEmpty && str.IsEmpty())
			{
				items.clear();
				return FALSE;
			}

			items.push_back(str);
			nBegin = i+1;
		}
	}
	CString str = strIn.Mid(nBegin, nCount - nBegin);
	if(!bIgnoreEmpty && str.IsEmpty())
	{
		items.clear();
		return FALSE;
	}
	items.push_back(str);
	return TRUE;
}

int GetWeekOfMonth(const CTime& tm)
{
	CTime tmFirstDay(tm.GetYear(), tm.GetMonth(), 1, 0, 0, 0);
	return (int)(ceil(tm.GetDay() / 7.) + (tm.GetDayOfWeek() < tmFirstDay.GetDayOfWeek() ? 1 : 0));
}

BOOL SystemShutDown()
{
	HANDLE hToken; 
	TOKEN_PRIVILEGES tkp; 
	
	// Get a token for this process. 
	
	if (!OpenProcessToken(GetCurrentProcess(), 
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
		return( FALSE ); 
	
	// Get the LUID for the shutdown privilege. 
	
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, 
        &tkp.Privileges[0].Luid); 
	
	tkp.PrivilegeCount = 1;  // one privilege to set    
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
	
	// Get the shutdown privilege for this process. 
	
	AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, 
        (PTOKEN_PRIVILEGES)NULL, 0); 
	
	if (GetLastError() != ERROR_SUCCESS) 
		return FALSE; 
	
	// Shut down the system and force all applications to close. 
	
	if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 
		SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
		SHTDN_REASON_MINOR_UPGRADE |
		SHTDN_REASON_FLAG_PLANNED)) 
		return FALSE; 

	return TRUE;
}



void ErrMsgBox(LPCTSTR szFmt, ...)
{
	CString strText;

	va_list argptr;
    va_start(argptr, szFmt);
    strText.FormatV(szFmt, argptr);
    va_end(argptr);
	::MessageBox(CWnd::GetActiveWindow()->GetSafeHwnd(), strText, _T("에러"), MB_ICONERROR);
}

BOOL IsExistDirectory(const CString& path, BOOL bCreateIfNotExist)
{
	HANDLE hFile = ::CreateFile(path, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS, 0);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		if(bCreateIfNotExist)
		{
			vector<CString> paths;
			if(!ParseLineSep(path, paths, _T('\\'), TRUE))
				return 0;

			CString p(paths[0]);
			for(UINT i = 1 ; i < paths.size() ; ++i)
			{
				p += _T('\\') + paths[i];

				if(::CreateFile(p, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_BACKUP_SEMANTICS, 0)
					== INVALID_HANDLE_VALUE)
				{
					if(!::CreateDirectory(p, NULL))
						return FALSE;
				}
			}
			return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}

BOOL IsExistFile(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));

#if 0
	CFile file;
	if(!file.Open(szPath, CFile::modeReadWrite))
	{
		DWORD dw = GetLastError();
		return FALSE;
	}

	return TRUE;
#endif

	/*WIN32_FIND_DATA FindFileData;
	HANDLE handle = FindFirstFile(szPath, &FindFileData) ;
	BOOL found = handle != INVALID_HANDLE_VALUE;
	if(found) 
	{
		FindClose(&handle);
	}
	return found;*/
}

CString GetDirectoryFromFilePath(const CString& strFilePath, int nDepth)
{
	CString str(strFilePath);

	nDepth += 1;
	for(int i = 0 ; i < nDepth ; ++i)
	{
		int nPos = str.ReverseFind(_T('\\'));
		if(-1 == nPos)
			break;

		str  = str.Left(nPos + 1);
	} 
	return str;
}

static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	// If the BFFM_INITIALIZED message is received
	// set the path to the start path.
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
		{
			if (NULL != lpData)
			{
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			}
		}
	}

	return 0; // The function should always return 0.
}

CString ShowFolderSelectDialog(const CString& pathDef)
{
	BROWSEINFO   bi = { 0 };
	LPITEMIDLIST pidl;
	TCHAR        szDisplay[MAX_PATH];
	BOOL         retval;

	CoInitialize(NULL);

	bi.hwndOwner      = AfxGetMainWnd()->GetSafeHwnd();
	bi.pszDisplayName = szDisplay;
	bi.lpszTitle      = TEXT("폴더를 선택해 주세요");
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn           = BrowseCallbackProc;
	bi.lParam         = (LPARAM)(LPCTSTR)pathDef;

	pidl = SHBrowseForFolder(&bi);
	TCHAR        szPath[MAX_PATH];
	if (NULL != pidl)
	{
		retval = SHGetPathFromIDList(pidl, szPath);
		CoTaskMemFree(pidl);
	}
	else
	{
		retval = FALSE;
	}

	if (!retval)
	{
		szPath[0] = TEXT('\0');
	}

	CoUninitialize();
	return szPath;
}


CString COMErrMsg(HRESULT hr)
{
	return WinErrMsg((DWORD)hr);
}


BOOL IsNumeric(const CString& val, BOOL* bNatural)
{
	if(val.IsEmpty())
		return FALSE;

	CString v = val;
	v.Remove(_T(' '));
	
	if(v.IsEmpty())
		return FALSE;

	int i = 0;
	if(v[0] == _T('+') || v[0] == _T('-'))
		++i;

	BOOL bExistPoint = FALSE;

	for( ; i < v.GetLength() ; ++i)
	{
		if(v[i] == _T('.'))
		{
			if(bExistPoint)
				return FALSE;

			bExistPoint = TRUE;
			continue;
		}

		if(v[i] < _T('0') || _T('9') < v[i])
			return FALSE;
	}
	if(bNatural)
		*bNatural = bExistPoint == FALSE;
	return TRUE;
}

CString WinErrMsg(DWORD dw)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL );
	CString msg((LPCTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);
	return msg;
}


CString ToString(const VARIANT& var, int nPrecision)
{
	CString s;
	switch(var.vt)
	{
	case VT_EMPTY:
	case VT_NULL:
		return _T("");
	case VT_I2:
		s.Format(_T("%d"), var.iVal);
		break;
	case VT_I4: 
	case VT_INT:
		s.Format(_T("%d"), var.intVal);
		break;
	case VT_UI2: 
		s.Format(_T("%d"), var.uiVal);
		break;
	case VT_UI4: 
	case VT_UINT:
		s.Format(_T("%d"), var.uintVal);
		break;
	case VT_R4: 
		s = ::ToString(var.fltVal, nPrecision);
		break;
	case VT_R8: 
		s = ::ToString(var.dblVal, nPrecision);
		break;
	case VT_CY:
		s = ::ToThousand(var.cyVal.int64);
		break;
	case VT_DATE:
		{
			COleDateTime date(var);
			s = date.Format(_T("%Y-%m-%d %H:%M:%S"));
		}
		break;
	case VT_BSTR:
		s = var.bstrVal;
		break;
	case VT_BOOL:
		s = var.boolVal == VARIANT_TRUE? _T("true") : _T("false");
		break;
	case VT_DECIMAL:
		{
		std::stringstream ss;
		
		ss << std::fixed << std::setprecision(nPrecision) << ::ToDecimal(var);
		s = ss.str().c_str();
		//s = ::ToString(::ToDecimal(var));
		}
		break;
	case VT_I1:
		s.Format(_T("%c"), var.cVal);
		break;
	case VT_UI1:
		s.Format(_T("%c"), (UCHAR)var.cVal);
		break;
	case VT_I8:
		s.Format(_T("%I64d"), var.llVal);
		break;
	case VT_UI8:
		s.Format(_T("%I64d"), var.ullVal);
		break;
	default:
		ASSERT(FALSE);
	}
	return s;
}

CString ToString(INT iVal)
{
	CString str;
	str.Format(_T("%d"), iVal);
	return str;
}
CString ToString(LONGLONG llVal)
{
	CString str;
	str.Format(_T("%I64d"), llVal);
	return str;
}
CString ToString(UINT uiVal)
{
	CString str;
	str.Format(_T("%d"), uiVal);
	return str;
}

CString ToString(ULONGLONG ullVal)
{
	CString str;
	str.Format(_T("%I64d"), ullVal);
	return str;
}
CString ToString(DOUBLE dVal, int nPrecision)
{
#ifdef _UNICODE
	std::wstringstream ss;
#else
	std::stringstream ss;
#endif
	ss << std::fixed << std::setprecision(nPrecision) << dVal;
	return ss.str().c_str();
	//The C++ Standard Library P.698 참조
	//return CString(decimal.str(14).c_str());

}
CString ToString(FLOAT fVal, int nPrecision)
{
#ifdef _UNICODE
	std::wstringstream ss;
#else
	std::stringstream ss;
#endif
	ss << std::fixed << std::setprecision(nPrecision) << fVal;
	return ss.str().c_str();
	//The C++ Standard Library P.698 참조
	//return CString(decimal.str(14).c_str());
}


//////////////////////////////////////////////////////////////////////////////////////
void GetMACaddress(vector<CString>& cont)
{
	IP_ADAPTER_INFO AdapterInfo[16];			// Allocate information for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);		// Save the memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(			// Call GetAdapterInfo
		AdapterInfo,							// [out] buffer to receive data
		&dwBufLen);								// [in] size of receive data buffer
	assert(dwStatus == ERROR_SUCCESS);			// Verify return value is valid, no buffer overflow

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
	do
	{
		CString s;
		s.Format(_T("%02X-%02X-%02X-%02X-%02X-%02X"), 
			pAdapterInfo->Address[0],
			pAdapterInfo->Address[1],
			pAdapterInfo->Address[2],
			pAdapterInfo->Address[3],
			pAdapterInfo->Address[4],
			pAdapterInfo->Address[5]);

		cont.push_back(s);
		pAdapterInfo = pAdapterInfo->Next;		// Progress through linked list
	}
	while(pAdapterInfo);						// Terminate if last adapter
}
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
int MoveFiles(LPCTSTR source, LPCTSTR destination)
{
	SHFILEOPSTRUCT SHFileOp;
	ZeroMemory(&SHFileOp, sizeof(SHFILEOPSTRUCT));

	SHFileOp.hwnd = NULL;
	SHFileOp.wFunc = FO_MOVE;
	SHFileOp.pFrom = source;
	SHFileOp.pTo = destination;
	SHFileOp.fFlags = FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_MULTIDESTFILES;

	return ::SHFileOperation(&SHFileOp);
}
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
void PlayWave(const CString& strFileName)
{
	::PlaySound((LPCTSTR)strFileName, NULL, SND_ASYNC);
}
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
void SetKeyboard(HWND hWnd, BOOL bEng)
{
	DWORD conVersion,senTence;
	HIMC hIMC=::ImmGetContext(hWnd);
	::ImmGetConversionStatus(hIMC,&conVersion,&senTence);

	if(bEng)
	{
		if(conVersion != 0) //영문이 아니라면
			::ImmSetConversionStatus(hIMC, 0, senTence);
	}
	else
	{
		if(conVersion != 1) //한글이 아니라면
			::ImmSetConversionStatus(hIMC, 1, senTence);
	}
}
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
WindowsVersion GetWindowsVersion()
{
	OSVERSIONINFOEX osver;
    osver.dwOSVersionInfoSize = sizeof(osver);
    if(!GetVersionEx((LPOSVERSIONINFO)&osver))
		return WIN_NO;

	DWORD dwMajor = osver.dwMajorVersion;
	DWORD dwMinor = osver.dwMinorVersion;
	WORD type = osver.wProductType;


	if(5 == dwMajor)
	{
		switch(dwMinor)
		{
		case 0: return WIN_2000;
		case 1: return WIN_XP;
		case 2: 
			type = GetSystemMetrics(SM_SERVERR2);
			if(type == 0)
				return WIN_SERVER_2003;
			else
				return WIN_SERVER_2003_R2;
		}
	}
	else if(6 == dwMajor)
	{
		switch(dwMinor)
		{
		case 0: 
			if(type == VER_NT_WORKSTATION)
				return WIN_VISTA;
			else
				return WIN_SERVER_2008;

		case 1:
			if(type != VER_NT_WORKSTATION)
				return WIN_SERVER_2008_R2;
			else
				return WIN_7;
		case 2: 
			if(type != VER_NT_WORKSTATION)
				return WIN_SERVER_2012;
			else
				return WIN_8;
		case 3: 
			if(type != VER_NT_WORKSTATION)
				return WIN_SERVER_2012_R2;
			else
				return WIN_8_1;
		}
	}
	return WIN_NO;
}
//////////////////////////////////////////////////////////////////////////////////////
