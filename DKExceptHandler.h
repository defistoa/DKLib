#pragma once
#include "DKLibDef.h"

#include <imagehlp.h>
#include "loki/Singleton.h"

class DK_API DKExceptionHandler
{
public:

	DKExceptionHandler( );
	~DKExceptionHandler( );

	void SetLogFileName( LPCTSTR pszLogFileName );

private:

	// entry point where control comes on an unhandled exception
	static LONG WINAPI MSJUnhandledExceptionFilter(
		PEXCEPTION_POINTERS pExceptionInfo );

	// where report info is extracted and generated	
	static void GenerateExceptionReport( PEXCEPTION_POINTERS pExceptionInfo );

	// Helper functions
	static LPTSTR GetExceptionString( DWORD dwCode );
	static BOOL GetLogicalAddress( 	PVOID addr, PTSTR szModule, DWORD len,
		DWORD& section, DWORD& offset );
	static void IntelStackWalk( PCONTEXT pContext );
	static void ImagehlpStackWalk( PCONTEXT pContext );
	static int __cdecl _tprintf(const TCHAR * format, ...);
	static BOOL InitImagehlpFunctions( void );
	// Variables used by the class
	static TCHAR m_szLogFileName[MAX_PATH];
	static LPTOP_LEVEL_EXCEPTION_FILTER m_previousFilter;
	static HANDLE m_hReportFile;

	// Make typedefs for some IMAGEHLP.DLL functions so that we can use them
	// with GetProcAddress
	typedef BOOL (__stdcall * SYMINITIALIZEPROC)( HANDLE, LPSTR, BOOL );
	typedef BOOL (__stdcall *SYMCLEANUPPROC)( HANDLE );

	typedef BOOL (__stdcall * STACKWALKPROC)
		( DWORD, HANDLE, HANDLE, LPSTACKFRAME, LPVOID,
		PREAD_PROCESS_MEMORY_ROUTINE,PFUNCTION_TABLE_ACCESS_ROUTINE,
		PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE );

	typedef LPVOID (__stdcall *SYMFUNCTIONTABLEACCESSPROC)( HANDLE, DWORD );

	typedef DWORD (__stdcall *SYMGETMODULEBASEPROC)( HANDLE, DWORD );

	typedef BOOL (__stdcall *SYMGETSYMFROMADDRPROC)
		( HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL );

	typedef BOOL (__stdcall *SYMGETLINEFROMADDR)(HANDLE, DWORD, PDWORD, PIMAGEHLP_LINE);

	static SYMINITIALIZEPROC _SymInitialize;
	static SYMCLEANUPPROC _SymCleanup;
	static STACKWALKPROC _StackWalk;
	static SYMFUNCTIONTABLEACCESSPROC _SymFunctionTableAccess;
	static SYMGETMODULEBASEPROC _SymGetModuleBase;
	static SYMGETSYMFROMADDRPROC _SymGetSymFromAddr;
	static SYMGETLINEFROMADDR _SymGetLineFromAddr;
};

inline DKExceptionHandler& InstEH() 
{ 
	return  Loki::SingletonHolder<DKExceptionHandler, Loki::CreateUsingNew, Loki::DeletableSingleton>::Instance();
}

extern DKExceptionHandler g_exceptionHandler;	//  global instance of class

