#pragma once

template<class TYPE>
struct _DKSTLAllocator_base
{	
	typedef TYPE value_type;
};

template<class TYPE>
struct _DKSTLAllocator_base<const TYPE>
{	
	typedef TYPE value_type;
};

// TEMPLATE CLASS DKSTLAllocator
template<class TYPE>
class DKSTLAllocator: public _DKSTLAllocator_base<TYPE>
{	// generic DKSTLAllocator for objects of class TYPE
private:
	static HANDLE s_hHeap;
	static UINT s_uNumAllocsInHeap;

public:
	typedef _DKSTLAllocator_base<TYPE> _Mybase;
	typedef typename _Mybase::value_type value_type;
	typedef value_type *pointer;
	typedef value_type& reference;
	typedef const value_type *const_pointer;
	typedef const value_type& const_reference;

	typedef size_t size_type;
#if (_MSC_VER <= 1600)
	typedef _PDFT difference_type;
#endif

	template<class _Other>
	struct rebind
	{	// convert an DKSTLAllocator<TYPE> to an DKSTLAllocator <_Other>
		typedef DKSTLAllocator<_Other> other;
	};

	pointer address(reference _Val) const
	{	// return address of mutable _Val
		return (&_Val);
	}

	const_pointer address(const_reference _Val) const
	{	// return address of nonmutable _Val
		return (&_Val);
	}

	DKSTLAllocator() throw ()
	{	// construct default DKSTLAllocator (do nothing)
	}

	DKSTLAllocator(const DKSTLAllocator<TYPE>&) throw ()
	{	// construct by copying (do nothing)
	}

	template<class _Other>
	DKSTLAllocator(const DKSTLAllocator<_Other>&) throw ()
	{	// construct from a related DKSTLAllocator (do nothing)
	}

	template<class _Other>
	DKSTLAllocator<TYPE>& operator=(const DKSTLAllocator<_Other>&)
	{	// assign from a related DKSTLAllocator (do nothing)
		return (*this);
	}

	void deallocate(pointer _Ptr, size_type)
	{	
		if(HeapFree(s_hHeap, 0, (void*)_Ptr))
		--s_uNumAllocsInHeap;
		if(0 == s_uNumAllocsInHeap)
		{
			if(HeapDestroy(s_hHeap))
				s_hHeap = NULL;
		}
	}

	pointer allocate(size_type _Count)
	{	
		if(NULL == s_hHeap)
		{
			s_hHeap = HeapCreate(0, 0, 0);
			if(s_hHeap == NULL)
			{
				return NULL;
			}
		}
		void* p = HeapAlloc(s_hHeap, HEAP_ZERO_MEMORY, _Count*sizeof(TYPE));
		++s_uNumAllocsInHeap;
		return (pointer)(p);
	}

	pointer allocate(size_type _Count, const void *)
	{	// allocate array of _Count elements, ignore hint
		return (allocate(_Count));
	}

	void construct(pointer _Ptr, const TYPE& _Val)
	{	// construct object at _Ptr with value _Val
		void *_Vptr = _Ptr;
		::new (_Vptr) TYPE(_Val);
	}

	void destroy(pointer _Ptr)
	{	// destroy object at _Ptr
		(_Ptr)->~TYPE();
	}

	size_t max_size() const throw ()
	{	// estimate maximum array size
		size_t _Count = (size_t)(-1) / sizeof (TYPE);
		return (0 < _Count ? _Count : 1);
	}
};

template<class TYPE>
HANDLE DKSTLAllocator<TYPE>::s_hHeap = NULL;

template<class TYPE>
UINT DKSTLAllocator<TYPE>::s_uNumAllocsInHeap = 0;

template<class TYPE, class _Other> 
inline bool operator==(const DKSTLAllocator<TYPE>&, const DKSTLAllocator<_Other>&) throw ()
{	// test for DKSTLAllocator equality (always true)
	return (true);
}

template<class TYPE, class _Other> 
inline bool operator!=(const DKSTLAllocator<TYPE>&, const DKSTLAllocator<_Other>&) throw ()
{	// test for DKSTLAllocator inequality (always false)
	return (false);
}