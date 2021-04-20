
#include "stdafx.h"
#include "DKObserver.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


DKObserver::~DKObserver()
{
#ifdef _DKOBSERVER_CHECK_CALLER
	if (!m_callers.empty())
	{
		for (auto pos = m_callers.begin(); pos != m_callers.end(); ++pos)
		{
			(*pos)->OnDetachObserver(this);
		}
	}
#endif
}
#ifdef _DKOBSERVER_CHECK_CALLER

void DKObserver::AddCaller(DKNotifier* pCaller)
{
	m_callers.push_back(pCaller);
}
void DKObserver::RemoveCaller(DKNotifier* pCaller)
{
	auto pos = std::find(m_callers.begin(), m_callers.end(), pCaller);
	if (pos == m_callers.end())
		return;

	m_callers.erase(pos);
}
#endif




/////////////////////////////////////////////////////////////////////////////////////////////////
DKNotifier::DKNotifier()
#ifdef _LOCK_OBSERVER_DETACH
: m_bLockObserverDetach(FALSE)
#endif
{
}


DKNotifier::~DKNotifier()
{
#ifdef _DKOBSERVER_CHECK_CALLER
	for (auto pos = m_observers.begin(); pos != m_observers.end(); ++pos)
	{
		(*pos)->RemoveCaller(this);
	}
#endif
}

void DKNotifier::AttachObserver(DKObserver* pObserver)
{
	auto pos = find(m_observers.begin(), m_observers.end(), pObserver);
	if (pos != m_observers.end())
	{
		return;
	}
#ifdef _DKOBSERVER_CHECK_CALLER
	pObserver->AddCaller(this);
#endif
	m_observers.push_back(pObserver);
}

void DKNotifier::DetachObserver(DKObserver* pObserver)
{
#ifdef _LOCK_OBSERVER_DETACH
	if (m_bLockObserverDetach)
	{
		m_observersDetachReserved.push_back(pObserver);
		return;
	}
#endif

	auto pos = find(m_observers.begin(), m_observers.end(), pObserver);
	if (pos == m_observers.end())
		return;

#ifdef _DKOBSERVER_CHECK_CALLER
	pObserver->RemoveCaller(this);
#endif
	m_observers.erase(pos);
}

void DKNotifier::OnDetachObserver(DKObserver* pObserver)
{
	auto pos = find(m_observers.begin(), m_observers.end(), pObserver);
	if (pos == m_observers.end())
		return;

	m_observers.erase(pos);
}

#ifdef _LOCK_OBSERVER_DETACH
void DKNotifier::UnlockObserverDetach()
{
	m_bLockObserverDetach = FALSE; // DetachObserver가 호출되기전에 셋팅해야 호출이 될수 있다.

	if (m_observersDetachReserved.empty())
		return;

	for (auto pos = m_observersDetachReserved.begin(); pos != m_observersDetachReserved.end(); ++pos)
		DetachObserver(*pos);

	m_observersDetachReserved.clear();
}

#endif