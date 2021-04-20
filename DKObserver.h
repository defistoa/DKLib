
#pragma once
#include "DKLibDef.h"
#include <list>

#define _DKOBSERVER_CHECK_CALLER
#define _LOCK_OBSERVER_DETACH


class DK_API DKObserver
{
	friend class DKNotifier;
public:

	virtual ~DKObserver();
#ifdef _DKOBSERVER_CHECK_CALLER
private:
	void AddCaller(DKNotifier* pCaller);
	void RemoveCaller(DKNotifier* pCaller);
private:
	std::list<DKNotifier*>	m_callers;
#endif
};


class DK_API DKNotifier
{
public:
	DKNotifier();
	virtual ~DKNotifier();
	virtual void AttachObserver(DKObserver* pObserver);
	virtual void DetachObserver(DKObserver* pObserver);
protected:
	friend class DKObserver;

	void OnDetachObserver(DKObserver* pObserver);

	std::list<DKObserver*>	m_observers;

#ifdef _LOCK_OBSERVER_DETACH
	BOOL m_bLockObserverDetach;
	std::list<DKObserver*> m_observersDetachReserved;

	void LockObserverDetach() { m_bLockObserverDetach = TRUE; }
	void UnlockObserverDetach();
#endif
};


#ifdef _LOCK_OBSERVER_DETACH
#define BEGIN_OBSERVER_ITERATION(ObserverType) \
	LockObserverDetach(); \
	for (auto pos = m_observers.begin(); pos != m_observers.end(); ++pos)\
{\
	ObserverType* pObserver = dynamic_cast<ObserverType*>(*pos); \
	ASSERT(pObserver);

#define END_OBSERVER_ITERATION() \
}\
	UnlockObserverDetach();

#else
#define BEGIN_OBSERVER_ITERATION(ObserverType) \
	for (auto pos = m_observers.begin(); pos != m_observers.end(); ++pos)\
{\
	ObserverType* pObserver = dynamic_cast<ObserverType*>(*pos);

#define END_OBSERVER_ITERATION() \
}\

#endif


