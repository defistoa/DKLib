#pragma once
#include "DKLibDef.h"
#include "DKcommon.h"

// DKMsgOnlyWnd
#define LOKI_FUNCTOR_IS_NOT_A_SMALLOBJECT
#include <loki/Function.h>
#include <vector>

typedef Loki::Function<BOOL (WPARAM, LPARAM)> DKMsgOnlyWndMsgFnc;

class DK_API DKMsgOnlyWnd : public CWnd
{
	DECLARE_DYNAMIC(DKMsgOnlyWnd)

public:
	DKMsgOnlyWnd();
	virtual ~DKMsgOnlyWnd();
protected:

	BOOL AddMsgHandler(UINT nMsg, DKMsgOnlyWndMsgFnc fnc);

private:
	typedef CMap<UINT, UINT&, DKMsgOnlyWndMsgFnc, DKMsgOnlyWndMsgFnc&> Handlers;
	//typedef std::map<UINT, DKMsgOnlyWndMsgFnc>	Handlers;
	Handlers	m_handlers;
public:
	OVERRIDE BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};

#define ADD_HANDLER(msg, memFnc) DKMsgOnlyWnd::AddMsgHandler(msg, DKMsgOnlyWndMsgFnc(this, &memFnc));
