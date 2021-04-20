// E:\Fundkeeper\GhostTrader\DKlib\src\DKMsgOnlyWnd.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "DKMsgOnlyWnd.h"

// DKMsgOnlyWnd

IMPLEMENT_DYNAMIC(DKMsgOnlyWnd, CWnd)

DKMsgOnlyWnd::DKMsgOnlyWnd()
{
	CString wnd_class_name = ::AfxRegisterWndClass(NULL);
	BOOL created = CWnd::CreateEx(0,wnd_class_name,
									_T("DKMsgOnlyWndClass"),0
									,0
									,0
									,0
									,0
									,HWND_MESSAGE,0);
}

DKMsgOnlyWnd::~DKMsgOnlyWnd()
{
	DestroyWindow();
}

BOOL DKMsgOnlyWnd::AddMsgHandler(UINT nMsg, DKMsgOnlyWndMsgFnc fnc)
{
	auto pos = m_handlers.PLookup(nMsg);
	if(pos != NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_handlers.SetAt(nMsg, fnc);
#if 0
	auto pos = m_handlers.find(nMsg);
	if(m_handlers.end() != pos)
		return FALSE;
	CHKINS((m_handlers.insert(make_pair(nMsg, fnc));
#endif
	return TRUE;
}

BOOL DKMsgOnlyWnd::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	auto pos = m_handlers.PLookup(message);
	if(pos != NULL)
	{
		pos->value(wParam, lParam);
		return TRUE;
	}
#if 0
	auto pos = m_handlers.find(message);
	if(pos != m_handlers.end())
	{
		return pos->second(wParam, lParam);
	}
#endif
	return CWnd::OnWndMsg(message, wParam, lParam, pResult);
}