#pragma once

#include <map>

template <class T>
class DKManagedPtrCont;

//////////////////////////////////////////////////////////////////////////////////////
template<class T>
class DKManagedPtr
{
private:
	friend class DKManagedPtrCont<T>;
	DKManagedPtrCont<T>*	m_pCont;
	T*					m_pObj;
public:
	DKManagedPtr() : m_pCont(NULL), m_pObj(NULL){}

	DKManagedPtr(const DKManagedPtr<T>& rhs)
	{
		m_pObj = NULL;
		m_pCont = NULL;
		*this = rhs;
	}

	DKManagedPtr<T>& operator=(const DKManagedPtr<T>& rhs)
	{
		if(rhs.m_pObj == m_pObj)
			return *this;

		Release();
	
		m_pCont = rhs.m_pCont;
		m_pObj = rhs.m_pObj;
		if(m_pCont)
			m_pCont->AddRef(m_pObj);
		return *this;
	}

	~DKManagedPtr()
	{
		Release();
	}

	operator T*() const
	{
		return m_pObj;
	}
	T& operator*() const
	{
		return *(GetPtr());
	}
	T* operator->() const
	{
		return GetPtr();
	}
	T* GetPtr() const
	{
		return m_pObj;
	}

	void Clear()
	{
		Release();
		m_pCont = NULL;
		m_pObj = NULL;
	}
private:
	void DKManagedPtr<T>::Release()
	{
		if(m_pObj)
			m_pCont->ReleaseRef(m_pObj);
	}
};

//////////////////////////////////////////////////////////////////////////////////////
template<class T>
class DKManagedPtrCont
{
	friend class DKManagedPtr<T>;
	typedef typename T::IDType ObjID;
	typedef std::map<ObjID, std::pair<T*, int> > Objects;
	Objects		m_objs;
public:
	DKManagedPtr<T> GetObject(ObjID id)
	{
		auto pos = m_objs.find(id);
		if(pos != m_objs.end())
		{
			DKManagedPtr<T> sp;
			sp.m_pCont = this;
			sp.m_pObj = pos->second.first;
			++(pos->second.second);
			return sp;
		}
		
		T* pObj = GetNewObject(id);
		DKManagedPtr<T> sp;

		if(pObj)
		{
			sp.m_pCont = this;
			sp.m_pObj = pObj;
			m_objs.insert(std::make_pair(id, std::make_pair(pObj, 1)));
		}
		return sp;
	}
protected:
	virtual T* GetNewObject(ObjID id) = 0;
private:
	void AddRef(T* pObj)
	{
		for(auto pos = m_objs.begin() ; pos != m_objs.end() ; ++pos)
		{
			if(pObj == pos->second.first)
			{
				++(pos->second.second);
				return;
			}
		}
	}

	void ReleaseRef(T* pObj)
	{
		for(auto pos = m_objs.begin() ; pos != m_objs.end() ; ++pos)
		{
			if(pObj == pos->second.first)
			{
				--(pos->second.second);
				if(0 == pos->second.second)
				{
					delete pos->second.first;
					m_objs.erase(pos);
				}
				return;
			}
		}
	}
};
//////////////////////////////////////////////////////////////////////////////////////

#pragma region DKManagedPtrSample
#if 0
class Stock
{
public:
	typedef int IDType;

	Stock(IDType c) : code(c)
	{
	}

	void Foo()
	{
	}
private:
	IDType code;
};

typedef DKManagedPtr<Stock> StockPtr;

class StockManager : public DKManagedPtrCont<Stock>
{
protected:
	virtual Stock* GetNewObject(Stock::IDType id)
	{
		Stock* pObj = new Stock(id);
		return pObj;
	}
};




void StockTest(StockPtr pStock)
{
	StockManager sm;

	StockPtr ptr = sm.GetObject(1);
}
#endif
#pragma endregion DKManagedPtrSample