#pragma once

#include "DKLibDef.h"

class DK_API ToThousandStr
{
public:
	static LPNUMBERFMT s_lpNumberFormat;
	static NUMBERFMT s_nfINT;
public:
	void operator=(signed __int8&);
	void operator=(signed __int16&);
	void operator=(signed __int32&);
	void operator=(signed __int64&);
	void operator=(unsigned __int8&);
	void operator=(unsigned __int16&);
	void operator=(unsigned __int32&);
	void operator=(unsigned __int64&);
	void operator=(float&);
	void operator=(double&);
	
	LPCTSTR GetString(void){return m_lpcBuffer;}

private:
	TCHAR m_lpcBuffer[(_CVTBUFSIZE*2)+1];
};
