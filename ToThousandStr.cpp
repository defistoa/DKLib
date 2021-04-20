#include "stdafx.h"

#include "ToThousandStr.h"

/*static*/ LPNUMBERFMT ToThousandStr::s_lpNumberFormat = NULL;


static NUMBERFMT GetIntFM()
{
	NUMBERFMT nf;
	nf.NumDigits = 0;    
	nf.LeadingZero = 0;  
	nf.Grouping = 3;     
	nf.lpDecimalSep = _T("."); 
	nf.lpThousandSep = _T(",");
	nf.NegativeOrder = 1;
	return nf;
}

NUMBERFMT ToThousandStr::s_nfINT = GetIntFM();

#define INT8BUFSIZE  10
#define INT16BUFSIZE 20
#define INT32BUFSIZE 20
#define INT64BUFSIZE 40


void ToThousandStr::operator=(signed __int8& i8Val) {
	TCHAR numBuffer[INT8BUFSIZE+1] = {0};
	_itot_s(i8Val, numBuffer, INT8BUFSIZE, 10 );
	GetNumberFormat(LOCALE_USER_DEFAULT, 0,  numBuffer, &s_nfINT, m_lpcBuffer, INT8BUFSIZE);
}

void ToThousandStr::operator=(unsigned __int8& u8Val) {
	TCHAR numBuffer[INT8BUFSIZE+1] = {0};
	_itot_s(u8Val, numBuffer, INT8BUFSIZE, 10 );
	GetNumberFormat(LOCALE_USER_DEFAULT, 0,  numBuffer, &s_nfINT, m_lpcBuffer, INT8BUFSIZE);
}

void ToThousandStr::operator=(signed __int16& i16Val) {
	TCHAR numBuffer[INT16BUFSIZE+1] = {0};
	_itot_s(i16Val, numBuffer, INT16BUFSIZE, 10 );
	GetNumberFormat(LOCALE_USER_DEFAULT, 0,  numBuffer, &s_nfINT, m_lpcBuffer, INT16BUFSIZE);
}

void ToThousandStr::operator=(unsigned __int16& u16Val) {
	TCHAR numBuffer[INT16BUFSIZE+1] = {0};
	_itot_s(u16Val, numBuffer, INT16BUFSIZE, 10 );
	GetNumberFormat(LOCALE_USER_DEFAULT, 0,  numBuffer, &s_nfINT, m_lpcBuffer, INT16BUFSIZE);
}

void ToThousandStr::operator=(signed __int32& i32Val) {
	TCHAR numBuffer[INT32BUFSIZE+1] = {0};
	_itot_s(i32Val, numBuffer, INT32BUFSIZE, 10 );
	GetNumberFormat(LOCALE_USER_DEFAULT, 0,  numBuffer, &s_nfINT, m_lpcBuffer, INT32BUFSIZE);
}

void ToThousandStr::operator=(unsigned __int32& u32Val) {
	TCHAR numBuffer[INT32BUFSIZE+1] = {0};
	_ultot_s(u32Val, numBuffer, 10 );
	GetNumberFormat(LOCALE_USER_DEFAULT, 0,  numBuffer, &s_nfINT, m_lpcBuffer, INT32BUFSIZE);
}

void ToThousandStr::operator=(signed __int64& i64Val) {
	TCHAR numBuffer[INT64BUFSIZE+1] = {0};
	_i64tot_s(i64Val, numBuffer, INT64BUFSIZE, 10 );
	GetNumberFormat(LOCALE_USER_DEFAULT, 0,  numBuffer, &s_nfINT, m_lpcBuffer, INT64BUFSIZE);
}

void ToThousandStr::operator=(unsigned __int64& u64Val) {
	TCHAR numBuffer[INT64BUFSIZE+1] = {0};
	_ui64tot_s(u64Val, numBuffer, INT64BUFSIZE, 10 );
	GetNumberFormat(LOCALE_USER_DEFAULT, 0,  numBuffer, &s_nfINT, m_lpcBuffer, INT64BUFSIZE);
}

void ToThousandStr::operator=(float& fVal) {
	TCHAR numBuffer[_CVTBUFSIZE+1] = {0};
	TCHAR bufFormatter[20] = TEXT("%f");
	if(s_lpNumberFormat) {
		_stprintf_s(bufFormatter, 20, TEXT("%%.%df"), s_lpNumberFormat->NumDigits);
	}
	_stprintf_s(numBuffer, _CVTBUFSIZE, bufFormatter, fVal);
	GetNumberFormat(LOCALE_USER_DEFAULT, 0,  numBuffer, s_lpNumberFormat, m_lpcBuffer, _CVTBUFSIZE);
}

void ToThousandStr::operator=(double& dbVal) {
	TCHAR numBuffer[(_CVTBUFSIZE*2)+1] = {0};
	TCHAR bufFormatter[20] = TEXT("%f");
	if(s_lpNumberFormat) {
		_stprintf_s(bufFormatter, 20, TEXT("%%.%df"), s_lpNumberFormat->NumDigits);
	}
	_stprintf_s(numBuffer, (_CVTBUFSIZE*2), bufFormatter, dbVal);
	GetNumberFormat(LOCALE_USER_DEFAULT, 0,  numBuffer, s_lpNumberFormat, m_lpcBuffer, (_CVTBUFSIZE*2));
}
