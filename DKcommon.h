#pragma once


#ifdef _UNICODE
#define TRACE_LINE TRACE(_T("[FUNC] %s (LINE:%d)\n"), __FUNCTIONW__, __LINE__);
#define TRACE_LINE_M(__s__) TRACE(_T("[FUNC] %s (LINE:%d) - %s\n"), __FUNCTIONW__, __LINE__, __s__);
#else
#define TRACE_LINE TRACE("FUNC:%s(LINE:%d)\n", __FUNCTION__, __LINE__);
#endif

const int NUL_IDX = -1;

#define CHECK(f)\
if(!(f))\
{\
	ASSERT(FALSE);\
	return FALSE;\
}

#define CHECK_RET(f, ret)\
if(!(f))\
{\
	ASSERT(FALSE);\
	return ret;\
}

#define CHECK_DO(f, doSome)\
if(!(f))\
{\
	ASSERT(FALSE);\
	doSome;\
	return FALSE;\
}

#define CHECK_DO2(f, doSome, ret)\
if(!(f))\
{\
	ASSERT(FALSE);\
	doSome;\
	return ret;\
}



#ifdef TRUE
#define YES		TRUE
#define NO		FALSE
#else
#define YES		1
#define NO		0
#endif

#define PI			((DOUBLE)3.141592654)
#define A_RADIAN	((DOUBLE)57.29577951)


#define NULL_DATE	NUL_IDX
#define NULL_TIME	NUL_IDX

///////////////////////////////////////////////////////////////////
// Own Heap Allocation
#if 0
#define DECLARE_OWN_HEAP()
#define IMPLEMENT_OWN_HEAP(className)
#else
#define DECLARE_OWN_HEAP()\
private:\
static HANDLE s_hHeap;\
public:\
static UINT s_uNumAllocsInHeap;\
public:\
	void* operator new(size_t size);\
	void operator delete(void* p);


#define IMPLEMENT_OWN_HEAP(className)\
HANDLE className::s_hHeap = NULL;\
UINT className::s_uNumAllocsInHeap = 0;\
void* className::operator new(size_t size)\
{\
	if(NULL == s_hHeap)\
	{\
		s_hHeap = ::HeapCreate(0, 0, 0);\
		if(s_hHeap == NULL)\
		{\
			ASSERT(FALSE);\
			return NULL;\
		}\
	}\
	void* p = ::HeapAlloc(s_hHeap, HEAP_ZERO_MEMORY, size);\
	ASSERT(p);\
	++s_uNumAllocsInHeap;\
	return p;\
}\
void className::operator delete(void* p)\
{\
	if(::HeapFree(s_hHeap, 0, p))\
		--s_uNumAllocsInHeap; \
	if(0 == s_uNumAllocsInHeap)\
	{\
		if(HeapDestroy(s_hHeap))\
			s_hHeap = NULL;\
	}\
}
#endif
///////////////////////////////////////////////////////////////////

#define FMT_BEGIN(ret)	{ CString& _r(ret); _r.Empty(); CString _t;
#define FMT_F(x)		{_t.Format(_T("%f\t"), (x)); _r += _t;}
#define FMT_FN(x, n)	{_t.Format(_T("%."#n"f\t"), (x)); _r += _t;}
#define FMT_D(x)		{_t.Format(_T("%d\t"), (x)); _r += _t;}
#define FMT_DN(x, n)	{_t.Format(_T("%"#n"d\t"), (x)); _r += _t;}
#define FMT_DN2(x, n)	{_t.Format(_T("%0"#n"d\t"), (x)); _r += _t;}
#define FMT_LD(x)		{_t.Format(_T("%I64d\t"), (x)); _r += _t;}
#define FMT_LDN(x, n)	{_t.Format(_T("%"#n"I64d\t"), (x)); _r += _t;}
#define FMT_S(x)		{_t.Format(_T("%s\t"), (x)); _r += _t;}
#define FMT_SP(x)		{_t.Format(_T("%s\t"), #x); _r += _t;}
#define FMT_C(x)		{_t.Format(_T("%c\t"), (x)); _r += _t;}
#define FMT_END()		if(!_r.IsEmpty())_r.Delete(_r.GetLength()-1);}
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////

#define A2B(a, cond, b) ((a) <= (cond) && (cond) <=(b))
#define NOT_A2B(a, cond, b) (!A2B(a, cond, b))


///////////////////////////////////////////////////////////////////

#define won 
#define CHEON(__x__)	((__x__)*1000)		// 천
#define MAHN(__x__)		((__x__)*10000)		// 만
#define EOK(__x__)		((__x__)*(LONGLONG)100000000)	//억
#define CHO(__x__)		((__x__)*(LONGLONG)1000000000000)	//조


///////////////////////////////////////////////////////////////////

struct Colors
{
	enum
	{
		AliceBlue                 = 0XFFF8F0
,		AntiqueWhite	          = 0XD7EBFA
,		Aqua	                  = 0XFFFF00
,		Aquamarine	              = 0XD4FF7F
,		Azure	                  = 0XFFFFF0
,		Beige	                  = 0XDCF5F5
,		Bisque	                  = 0XC4E4FF
,		Black	                  = 0X000000
,		BlanchedAlmond	          = 0XCDEBFF
,		Blue	                  = 0XFF0000
,		BlueViolet	              = 0XE22B8A
,		Brown	                  = 0X2A2AA5
,		Burlywood	              = 0X87B8DE
,		CadetBlue	              = 0XA09E5F
,		Chartreuse	              = 0X00FF7F
,		Chocolate	              = 0X1E69D2
,		Coral	                  = 0X507FFF
,		Cornflower	              = 0XED9564
,		Cornsilk	              = 0XDCF8FF
,		Crimson	                  = 0X3C14DC
,		Cyan	                  = 0XFFFF00
,		DarkBlue	              = 0X8B0000
,		DarkCyan	              = 0X8B8B00
,		DarkGoldenrod	          = 0X0B86B8
,		DarkGray	              = 0XA9A9A9
,		DarkGreen	              = 0X006400
,		DarkKhaki	              = 0X6BB7BD
,		DarkMagenta	              = 0X8B008B
,		DarkOliveGreen	          = 0X2F6B55
,		DarkOrange	              = 0X008CFF
,		DarkOrchid	              = 0XCC3299
,		DarkRed	                  = 0X00008B
,		DarkSalmon	              = 0X7A96E9
,		DarkSeaGreen	          = 0X8FBC8F
,		DarkSlateBlue	          = 0X8B3D48
,		DarkSlateGray	          = 0X4F4F2F
,		DarkTurquoise	          = 0XD1CE00
,		DarkViolet	              = 0XD30094
,		DeepPink	              = 0X9314FF
,		DeepSkyBlue	              = 0XFFBF00
,		DimGray	                  = 0X696969
,		DodgerBlue	              = 0XFF901E
,		Firebrick	              = 0X2222B2
,		FloralWhite	              = 0XF0FAFF
,		ForestGreen	              = 0X228B22
,		Fuchsia	                  = 0XFF00FF
,		Gainsboro	              = 0XDCDCDC
,		GhostWhite	              = 0XFFF8F8
,		Gold	                  = 0X00D7FF
,		Goldenrod	              = 0X20A5DA
,		Gray	                  = 0XBEBEBE
,		Green	                  = 0X00FF00
,		GreenYellow	              = 0X2FFFAD
,		Honeydew	              = 0XF0FFF0
,		HotPink	                  = 0XB469FF
,		IndianRed	              = 0X5C5CCD
,		Indigo	                  = 0X82004B
,		Ivory	                  = 0XF0FFFF
,		Khaki	                  = 0X8CE6F0
,		Lavender	              = 0XFAE6E6
,		LavenderBlush	          = 0XF5F0FF
,		LawnGreen	              = 0X00FC7C
,		LemonChiffon	          = 0XCDFAFF
,		LightBlue	              = 0XE6D8AD
,		LightCoral	              = 0X8080F0
,		LightCyan	              = 0XFFFFE0
,		LightGoldenrod	          = 0XD2FAFA
,		LightGray	              = 0XD3D3D3
,		LightGreen	              = 0X90EE90
,		LightPink	              = 0XC1B6FF
,		LightSalmon	              = 0X7AA0FF
,		LightSeaGreen	          = 0XAAB220
,		LightSkyBlue	          = 0XFACE87
,		LightSlateGray	          = 0X998877
,		LightSteelBlue	          = 0XDEC4B0
,		LightYellow	              = 0XE0FFFF
,		LimeGreen	              = 0X32CD32
,		Linen	                  = 0XE6F0FA
,		Magenta	                  = 0XFF00FF
,		Maroon	                  = 0X6030B0
,		MediumAquamarine	      = 0XAACD66
,		MediumBlue	              = 0XCD0000
,		MediumOrchid	          = 0XD355BA
,		MediumPurple	          = 0XDB7093
,		MediumSeaGreen	          = 0X71B33C
,		MediumSlateBlue	          = 0XEE687B
,		MediumSpringGreen	      = 0X9AFA00
,		MediumTurquoise	          = 0XCCD148
,		MediumVioletRed	          = 0X8515C7
,		MidnightBlue	          = 0X701919
,		MintCream	              = 0XFAFFF5
,		MistyRose	              = 0XE1E4FF
,		Moccasin	              = 0XB5E4FF
,		NavajoWhite	              = 0XADDEFF
,		Navy	                  = 0X800000
,		OldLace	                  = 0XE6F5FD
,		Olive	                  = 0X008080
,		OliveDrab	              = 0X238E6B
,		Orange	                  = 0X00A5FF
,		OrangeRed	              = 0X0045FF
,		Orchid	                  = 0XD670DA
,		PaleGoldenrod	          = 0XAAE8EE
,		PaleGreen	              = 0X98FB98
,		PaleTurquoise	          = 0XEEEEAF
,		PaleVioletRed	          = 0X9370DB
,		PapayaWhip	              = 0XD5EFFF
,		PeachPuff	              = 0XB9DAFF
,		Peru	                  = 0X3F85CD
,		Pink	                  = 0XCBC0FF
,		Plum	                  = 0XDDA0DD
,		PowderBlue	              = 0XE6E0B0
,		Purple	                  = 0XF020A0
,		Red	                      = 0X0000FF
,		RosyBrown	              = 0X8F8FBC
,		RoyalBlue	              = 0XE16941
,		SaddleBrown	              = 0X13458B
,		Salmon	                  = 0X7280FA
,		SandyBrown	              = 0X60A4F4
,		SeaGreen	              = 0X578B2E
,		Seashell	              = 0XEEF5FF
,		Sienna	                  = 0X2D52A0
,		Silver	                  = 0XC0C0C0
,		SkyBlue	                  = 0XEBCE87
,		SlateBlue	              = 0XCD5A6A
,		SlateGray	              = 0X908070
,		Snow	                  = 0XFAFAFF
,		SpringGreen	              = 0X7FFF00
,		SteelBlue	              = 0XB48246
,		Tan	                      = 0X8CB4D2
,		Teal	                  = 0X808000
,		Thistle	                  = 0XD8BFD8
,		Tomato	                  = 0X4763FF
,		Turquoise	              = 0XD0E040
,		Violet	                  = 0XEE82EE
,		Wheat	                  = 0XB3DEF5
,		White	                  = 0XFFFFFF
,		WhiteSmoke	              = 0XF5F5F5
,		Yellow	                  = 0X00FFFF
,		YellowGreen	              = 0X32CD9A
	};	
};

enum Color
{
	COLOR_RISE			= RGB(204,0,0),
	COLOR_FALL			= Colors::Navy,
};


///////////////////////////////////////////////////////////////////

#define FMT_DATE			_T("%Y-%m-%d")
#define FMT_DATE_TIME		_T("%Y-%m-%d %H:%M:%S")
#define FMT_DATE_NO_SEP		_T("%Y%m%d")
#define FMT_24TIME_NO_SEP	_T("%H%M%S")

#define OVERRIDE

#ifdef _DEBUG
#define CHKINS(_ins_) { auto insRet = _ins_; ASSERT(insRet.second == true); }
#else
#define CHKINS(_ins_) ((void)(_ins_))
#endif


enum WindowsVersion
{
	WIN_NO,
	WIN_2000,
	WIN_XP,
	WIN_SERVER_2003,
	WIN_SERVER_2003_R2,
	WIN_VISTA,
	WIN_SERVER_2008,
	WIN_SERVER_2008_R2,
	WIN_7,
	WIN_SERVER_2012,
	WIN_8,
	WIN_SERVER_2012_R2,
	WIN_8_1,
};

