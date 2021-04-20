#pragma once
#include "DKLibDef.h"
#include "StdioFileEx.h"

#define LOKI_FUNCTOR_IS_NOT_A_SMALLOBJECT
#include <loki/Function.h>
#include "loki/Singleton.h"
#include <boost/function.hpp>

enum LogType
{
	lt_none = 0,
	lt_info,
	lt_sys_err,
	lt_network_err,
	lt_db_err,
	lt_secure_err,
	lt_act,
	lt_order, 
	lt_diag,		// 배포후에도 필요한 진단 관련 로그
#ifdef _DEBUG
	lt_devel,		// 개발단계에서만 보여지게될 로그
#endif
//////////////////////////////
	lt_count
};


class DK_API Logger
{
	
public:
	Logger(void);
	~Logger(void);

	typedef Loki::Function<void (LPCTSTR)> NotifyFnc;
	
	BOOL Init(LPCTSTR szRoot);
	void Log(LogType type, LPCTSTR szFmt, ...);
	void SetNotifyFnc(NotifyFnc fnc) { m_fncNotify = fnc; }
	void LogD(LPCTSTR szFnc, int line, LPCTSTR szFmt, ...);

	static void NotifyDummy(LPCTSTR) {}
private:
	CStdioFileEx	m_files[lt_count];
	NotifyFnc		m_fncNotify;
};

inline Logger& InstLog() { return Loki::SingletonHolder<Logger, Loki::CreateUsingNew, Loki::DeletableSingleton>::Instance(); }

#if 1
#define LOG_INF(__str__, ...) InstLog().Log(lt_info, __str__, ##__VA_ARGS__ )
#define LOG_SYS(__str__, ...) InstLog().Log(lt_sys_err, __str__, ##__VA_ARGS__ )
#define LOG_NET(__str__, ...) InstLog().Log(lt_network_err, __str__, ##__VA_ARGS__ )
#define LOG_DB(__str__, ...) InstLog().Log(lt_db_err, __str__, ##__VA_ARGS__ )
#define LOG_SEC(__str__, ...) InstLog().Log(lt_secure_err, __str__, ##__VA_ARGS__ )
#define LOG_ACT(__str__, ...) InstLog().Log(lt_act, __str__, ##__VA_ARGS__ )
#define LOG_DIAG(__str__, ...) InstLog().Log(lt_diag, __str__, ##__VA_ARGS__ )
#define LOG_ORD(__str__, ...) InstLog().Log(lt_order, __str__, ##__VA_ARGS__ )
#define LOG_DEV(__str__, ...) InstLog().LogD(__FUNCTIONW__, __LINE__, __str__, ##__VA_ARGS__ )
#else
#define LOG_INF(__str__, ...) ((void)0)
#define LOG_SYS(__str__, ...) ((void)0)
#define LOG_NET(__str__, ...) ((void)0)
#define LOG_DB(__str__, ...) ((void)0)
#define LOG_SEC(__str__, ...) ((void)0)
#define LOG_ACT(__str__, ...) ((void)0)
#define LOG_DIAG(__str__, ...) ((void)0)
#define LOG_ORD(__str__, ...) ((void)0)
#endif

#ifdef _DEBUG
#ifdef _UNICODE
#define DLOG(__str__, ...) InstLog().LogD(__FUNCTIONW__, __LINE__, __str__, ##__VA_ARGS__ )
#else
#define DLOG(__str__, ...) InstLog().LogD(__FUNCTION__, __LINE__, __str__, ##__VA_ARGS__ )
#endif
#else
#define DLOG(__str__, ...) ((void)0)
#endif
 