//=--------------------------------------------------------------------------=
// ARRTMPL.H:	Template file for CArray
//=--------------------------------------------------------------------------=
// Copyright  1997  Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#ifndef _ARRTMPL_H_
#define _ARRTMPL_H_

//---------------------------------------------------------------------------
// Helpers

#ifdef new
#define _REDEF_NEW new
#undef new
#endif

#ifndef _INC_NEW
	#include <new.h>
#endif

template<class TYPE>
#if VC8
inline void CopyElementsOld(TYPE* pDest, const TYPE* pSrc, int nCount = 1)
#else
inline void CopyElements(TYPE* pDest, const TYPE* pSrc, int nCount = 1)
#endif
{
	// Uses bitwise copy or overloaded operator=
	//
	while (nCount--)
		pDest[nCount] = pSrc[nCount];
}

#undef new
#ifdef _REDEF_NEW
#define new _REDEF_NEW
#undef _REDEF_NEW
#endif

#if VC8
template<class TYPE>
inline void ConstructElementsOld(TYPE* pElements, int nCount = 1)
{
	// first do bit-wise zero initialization
    //
	ZeroMemory((void*)pElements, nCount * sizeof(TYPE));

	// then call the constructor(s)
    //
	for (; nCount--; pElements++)
		::new((void*)pElements) TYPE;
}

template<class TYPE>
inline void DestructElementsOld(TYPE* pElements, int nCount = 1)
{
    // call the destructor(s)
    //
	for (; nCount--; pElements++)
		pElements->~TYPE();
}

//---------------------------------------------------------------------------
// CArrayOld<TYPE, ARG_TYPE>

template<class TYPE, class ARG_TYPE> 
class CArrayOld
{
// Construction
public:
    CArrayOld(int nGrowSize = 10);
   ~CArrayOld();

// Interface
public:
    // Operations
    int Add(ARG_TYPE item);
    int Add(const TYPE *pitem, int nCount);
    BOOL SetAt(int index, ARG_TYPE item);
    int InsertAt(int index, ARG_TYPE item);
    int InsertAt(int index, const TYPE *pitem, int nCount);
    int DeleteAt(int index, int nCount = 1);
    void RemoveAll(void);
    void Attach(TYPE* pItems, int nSize);
	BOOL SetSize(int nNewSize, int nGrowSize = -1);
        
    // Attributes
    int GetSize(void) const;
    int GetUpperBound(void) const;

    // Member access
    TYPE Item(int index) const;
    TYPE& Item(int index);

	// Direct Access to the element data (may return NULL)
	const TYPE* GetItems(void) const;
	TYPE* GetItems(void);

    // overloaded operator helpers
	TYPE operator[](int index) const;
	TYPE& operator[](int index);

// Data members
protected:
    int     m_nGrowSize;
    int     m_nSize;
    int     m_nMaxSize;
    TYPE   *m_items;
};

//
// In-line definitions
//


template<class TYPE, class ARG_TYPE>
inline int CArrayOld<TYPE, ARG_TYPE>::GetSize(void) const
{
    return m_nSize;
}

template<class TYPE, class ARG_TYPE>
inline int CArrayOld<TYPE, ARG_TYPE>::GetUpperBound(void) const
{
    return GetSize()-1;
}

template<class TYPE, class ARG_TYPE>
inline const TYPE* CArrayOld<TYPE, ARG_TYPE>::GetItems(void) const
{
    return m_items;
}

template<class TYPE, class ARG_TYPE>
inline TYPE* CArrayOld<TYPE, ARG_TYPE>::GetItems(void)
{
    return m_items;
}

template<class TYPE, class ARG_TYPE>
inline TYPE CArrayOld<TYPE, ARG_TYPE>::Item(int index) const
{
    return m_items[index];
}

template<class TYPE, class ARG_TYPE>
inline TYPE& CArrayOld<TYPE, ARG_TYPE>::Item(int index)
{
    return m_items[index];
}

template<class TYPE, class ARG_TYPE>
inline TYPE CArrayOld<TYPE, ARG_TYPE>::operator[](int index) const
{ 
    return Item(index); 
}

template<class TYPE, class ARG_TYPE>
inline TYPE& CArrayOld<TYPE, ARG_TYPE>::operator[](int index)
{ 
    return Item(index);
}

template<class TYPE, class ARG_TYPE>
inline BOOL CArrayOld<TYPE, ARG_TYPE>::SetAt(int index, ARG_TYPE item)
{
	if (index < 0 || index >= m_nSize)
		return FALSE;

	// Destroy current occupant at index
	//
	DestructElementsOld(m_items+index);

	// Copy given item
	//
	m_items[index] = item;

	return TRUE;
}

template<class TYPE, class ARG_TYPE>
inline CArrayOld<TYPE, ARG_TYPE>::~CArrayOld()
{
    RemoveAll();
}

//
// Out-of-line definitions
//

template<class TYPE, class ARG_TYPE>
inline CArrayOld<TYPE, ARG_TYPE>::CArrayOld(int nGrowSize)
{
    m_items = NULL;
    m_nGrowSize = nGrowSize;
    m_nSize = 0;
    m_nMaxSize = 0;
}

template<class TYPE, class ARG_TYPE>
inline void CArrayOld<TYPE, ARG_TYPE>::Attach(TYPE* pItems, int nSize)
{
    // Clear the current contents
    //
    RemoveAll();

    // Use the given arguments in ours
    //
    m_items = pItems
    m_nMaxSize = m_nSize = nSize;
}

template<class TYPE, class ARG_TYPE>
void CArrayOld<TYPE, ARG_TYPE>::RemoveAll(void)
{
    if (m_items)
    {
        // delete should call TYPE's destructor, do this
        // in reverse so that delete will not have to
        // shuffle the contents
        //
        DestructElementsOld(m_items, m_nSize);
        CoTaskMemFree((void*)m_items);

        m_nMaxSize = 0;
        m_nSize = 0;
        m_items = NULL; 
    }
}

template<class TYPE, class ARG_TYPE>
inline int CArrayOld<TYPE, ARG_TYPE>::Add(ARG_TYPE item)
{
	return Add(&item, 1);
}

template<class TYPE, class ARG_TYPE>
inline int CArrayOld<TYPE, ARG_TYPE>::Add(const TYPE *pitem, int nCount)
{
	return InsertAt(GetSize(), pitem, nCount);
}

template<class TYPE, class ARG_TYPE>
inline int CArrayOld<TYPE, ARG_TYPE>::InsertAt(int index, ARG_TYPE item)
{
	return InsertAt(index, &item, 1);
}

template<class TYPE, class ARG_TYPE>
int CArrayOld<TYPE, ARG_TYPE>::InsertAt(int index, const TYPE *pitems, int nCount)
{
	if (nCount < 1) return 0;
	if (index < 0 || index > m_nSize) return -1;

    // Grow the storage as needed
    //
    int nOldSize = m_nSize;
    
    if (SetSize(m_nSize+nCount))
    {
        if (index < nOldSize)
		{
            MoveMemory(m_items+index+nCount, m_items+index, (nOldSize-index) * sizeof(TYPE));
			ZeroMemory(m_items+index, nCount * sizeof(TYPE));
		}
        while (nCount--)
			m_items[index+nCount] = pitems[nCount];

        return m_nSize;
    }
    return 0;
}

template<class TYPE, class ARG_TYPE>
int CArrayOld<TYPE, ARG_TYPE>::DeleteAt(int index, int nCount /* = 1 */)
{
	if (-1 <= nCount && !(index < 0 || index >= m_nSize))
	{
		if (-1 == nCount || nCount > m_nSize-index)
			nCount = m_nSize-index;
			
		DestructElementsOld(m_items+index, nCount);
		m_nSize -= nCount;
    
		if (index < m_nSize && m_nSize > 0)
			MoveMemory((void*)(m_items+index), (void*)(m_items+index+nCount), (m_nSize-index) * sizeof(TYPE));
	}
    return m_nSize;
}
 
template<class TYPE, class ARG_TYPE>
BOOL CArrayOld<TYPE, ARG_TYPE>::SetSize(int nNewSize, int nGrowSize)
{
	if (nGrowSize != -1)
		m_nGrowSize = nGrowSize;  // set new size

    // Easy case
    //
	if (nNewSize == 0)
        RemoveAll();
    
    // First time case
    //
    else if (NULL == m_items)
    {
		// Allocate at least by grow size
		//
		int nMaxSize = nNewSize < nGrowSize ? nGrowSize : nNewSize;

        // Allocate to new size
        //
        m_items = (TYPE*)CoTaskMemAlloc(nMaxSize * sizeof(TYPE));

        if (NULL == m_items)
            return FALSE;

        // initialize the new elements
        //
        ConstructElementsOld(m_items, nNewSize);

		// Update state
		//
        m_nSize = nNewSize;
		m_nMaxSize = nMaxSize;
    }
    // If the new size is still less then the max size then
    // no need for allocations
    //
    else if (nNewSize <= m_nMaxSize)
	{
        // it fits
        if (nNewSize > m_nSize)
        {
            // initialize the new elements
            //
            ConstructElementsOld(&m_items[m_nSize], nNewSize-m_nSize);
        }
		// destroy the old elements if count is greater
        // 
		if (m_nSize > nNewSize)
		{
			DestructElementsOld(&m_items[nNewSize], m_nSize-nNewSize);
		}
		m_nSize = nNewSize;
	}
    // otherwise, grow array
    //        
	else
	{
        TYPE* p;
        int nMaxSize;

		nGrowSize = m_nGrowSize;
		if (nGrowSize < 0)
		{
			// heuristically determine growth when grow size == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowSize = m_nSize / 8;
			nGrowSize = (nGrowSize < 4) ? 4 : ((nGrowSize > 1024) ? 1024 : nGrowSize);
		}
        // Adjust new max size to granularity
        //
		if (nNewSize < m_nMaxSize + nGrowSize)
			nMaxSize = m_nMaxSize + nGrowSize;
		else
            nMaxSize = nNewSize;

        // ReAlloc to new size
        //
        p = (TYPE*)CoTaskMemRealloc((void*)m_items, nMaxSize * sizeof(TYPE));

        if (NULL == p)
            return FALSE;

        // Assign new array data buffer and size
        //
        m_items = p;
        m_nMaxSize = nMaxSize;

        // Construct new elements
        //
        ConstructElementsOld(&m_items[m_nSize], nNewSize-m_nSize);
        m_nSize = nNewSize;
    }
    return TRUE;
}
#else
template<class TYPE>
inline void ConstructElements(TYPE* pElements, int nCount = 1)
{
	// first do bit-wise zero initialization
    //
	ZeroMemory((void*)pElements, nCount * sizeof(TYPE));

	// then call the constructor(s)
    //
	for (; nCount--; pElements++)
		::new((void*)pElements) TYPE;
}

template<class TYPE>
inline void DestructElements(TYPE* pElements, int nCount = 1)
{
    // call the destructor(s)
    //
	for (; nCount--; pElements++)
		pElements->~TYPE();
}


//---------------------------------------------------------------------------
// CArray<TYPE, ARG_TYPE>

template<class TYPE, class ARG_TYPE> 
class CArray
{
// Construction
public:
    CArray(int nGrowSize = 10);
   ~CArray();

// Interface
public:
    // Operations
    int Add(ARG_TYPE item);
    int Add(const TYPE *pitem, int nCount);
    BOOL SetAt(int index, ARG_TYPE item);
    int InsertAt(int index, ARG_TYPE item);
    int InsertAt(int index, const TYPE *pitem, int nCount);
    int DeleteAt(int index, int nCount = 1);
    void RemoveAll(void);
    void Attach(TYPE* pItems, int nSize);
	BOOL SetSize(int nNewSize, int nGrowSize = -1);
        
    // Attributes
    int GetSize(void) const;
    int GetUpperBound(void) const;

    // Member access
    TYPE Item(int index) const;
    TYPE& Item(int index);

	// Direct Access to the element data (may return NULL)
	const TYPE* GetItems(void) const;
	TYPE* GetItems(void);

    // overloaded operator helpers
	TYPE operator[](int index) const;
	TYPE& operator[](int index);

// Data members
protected:
    int     m_nGrowSize;
    int     m_nSize;
    int     m_nMaxSize;
    TYPE   *m_items;
};

//
// In-line definitions
//


template<class TYPE, class ARG_TYPE>
inline int CArray<TYPE, ARG_TYPE>::GetSize(void) const
{
    return m_nSize;
}

template<class TYPE, class ARG_TYPE>
inline int CArray<TYPE, ARG_TYPE>::GetUpperBound(void) const
{
    return GetSize()-1;
}

template<class TYPE, class ARG_TYPE>
inline const TYPE* CArray<TYPE, ARG_TYPE>::GetItems(void) const
{
    return m_items;
}

template<class TYPE, class ARG_TYPE>
inline TYPE* CArray<TYPE, ARG_TYPE>::GetItems(void)
{
    return m_items;
}

template<class TYPE, class ARG_TYPE>
inline TYPE CArray<TYPE, ARG_TYPE>::Item(int index) const
{
    return m_items[index];
}

template<class TYPE, class ARG_TYPE>
inline TYPE& CArray<TYPE, ARG_TYPE>::Item(int index)
{
    return m_items[index];
}

template<class TYPE, class ARG_TYPE>
inline TYPE CArray<TYPE, ARG_TYPE>::operator[](int index) const
{ 
    return Item(index); 
}

template<class TYPE, class ARG_TYPE>
inline TYPE& CArray<TYPE, ARG_TYPE>::operator[](int index)
{ 
    return Item(index);
}

template<class TYPE, class ARG_TYPE>
inline BOOL CArray<TYPE, ARG_TYPE>::SetAt(int index, ARG_TYPE item)
{
	if (index < 0 || index >= m_nSize)
		return FALSE;

	// Destroy current occupant at index
	//
	DestructElements(m_items+index);

	// Copy given item
	//
	m_items[index] = item;

	return TRUE;
}

template<class TYPE, class ARG_TYPE>
inline CArray<TYPE, ARG_TYPE>::~CArray()
{
    RemoveAll();
}

//
// Out-of-line definitions
//

template<class TYPE, class ARG_TYPE>
inline CArray<TYPE, ARG_TYPE>::CArray(int nGrowSize)
{
    m_items = NULL;
    m_nGrowSize = nGrowSize;
    m_nSize = 0;
    m_nMaxSize = 0;
}

template<class TYPE, class ARG_TYPE>
inline void CArray<TYPE, ARG_TYPE>::Attach(TYPE* pItems, int nSize)
{
    // Clear the current contents
    //
    RemoveAll();

    // Use the given arguments in ours
    //
    m_items = pItems
    m_nMaxSize = m_nSize = nSize;
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::RemoveAll(void)
{
    if (m_items)
    {
        // delete should call TYPE's destructor, do this
        // in reverse so that delete will not have to
        // shuffle the contents
        //
        DestructElements(m_items, m_nSize);
        CoTaskMemFree((void*)m_items);

        m_nMaxSize = 0;
        m_nSize = 0;
        m_items = NULL; 
    }
}

template<class TYPE, class ARG_TYPE>
inline int CArray<TYPE, ARG_TYPE>::Add(ARG_TYPE item)
{
	return Add(&item, 1);
}

template<class TYPE, class ARG_TYPE>
inline int CArray<TYPE, ARG_TYPE>::Add(const TYPE *pitem, int nCount)
{
	return InsertAt(GetSize(), pitem, nCount);
}

template<class TYPE, class ARG_TYPE>
inline int CArray<TYPE, ARG_TYPE>::InsertAt(int index, ARG_TYPE item)
{
	return InsertAt(index, &item, 1);
}

template<class TYPE, class ARG_TYPE>
int CArray<TYPE, ARG_TYPE>::InsertAt(int index, const TYPE *pitems, int nCount)
{
	if (nCount < 1) return 0;
	if (index < 0 || index > m_nSize) return -1;

    // Grow the storage as needed
    //
    int nOldSize = m_nSize;
    
    if (SetSize(m_nSize+nCount))
    {
        if (index < nOldSize)
		{
            MoveMemory(m_items+index+nCount, m_items+index, (nOldSize-index) * sizeof(TYPE));
			ZeroMemory(m_items+index, nCount * sizeof(TYPE));
		}
        while (nCount--)
			m_items[index+nCount] = pitems[nCount];

        return m_nSize;
    }
    return 0;
}

template<class TYPE, class ARG_TYPE>
int CArray<TYPE, ARG_TYPE>::DeleteAt(int index, int nCount /* = 1 */)
{
	if (-1 <= nCount && !(index < 0 || index >= m_nSize))
	{
		if (-1 == nCount || nCount > m_nSize-index)
			nCount = m_nSize-index;
			
		DestructElements(m_items+index, nCount);
		m_nSize -= nCount;
    
		if (index < m_nSize && m_nSize > 0)
			MoveMemory((void*)(m_items+index), (void*)(m_items+index+nCount), (m_nSize-index) * sizeof(TYPE));
	}
    return m_nSize;
}
 
template<class TYPE, class ARG_TYPE>
BOOL CArray<TYPE, ARG_TYPE>::SetSize(int nNewSize, int nGrowSize)
{
	if (nGrowSize != -1)
		m_nGrowSize = nGrowSize;  // set new size

    // Easy case
    //
	if (nNewSize == 0)
        RemoveAll();
    
    // First time case
    //
    else if (NULL == m_items)
    {
		// Allocate at least by grow size
		//
		int nMaxSize = nNewSize < nGrowSize ? nGrowSize : nNewSize;

        // Allocate to new size
        //
        m_items = (TYPE*)CoTaskMemAlloc(nMaxSize * sizeof(TYPE));

        if (NULL == m_items)
            return FALSE;

        // initialize the new elements
        //
        ConstructElements(m_items, nNewSize);

		// Update state
		//
        m_nSize = nNewSize;
		m_nMaxSize = nMaxSize;
    }
    // If the new size is still less then the max size then
    // no need for allocations
    //
    else if (nNewSize <= m_nMaxSize)
	{
        // it fits
        if (nNewSize > m_nSize)
        {
            // initialize the new elements
            //
            ConstructElements(&m_items[m_nSize], nNewSize-m_nSize);
        }
		// destroy the old elements if count is greater
        // 
		if (m_nSize > nNewSize)
		{
			DestructElements(&m_items[nNewSize], m_nSize-nNewSize);
		}
		m_nSize = nNewSize;
	}
    // otherwise, grow array
    //        
	else
	{
        TYPE* p;
        int nMaxSize;

		nGrowSize = m_nGrowSize;
		if (nGrowSize < 0)
		{
			// heuristically determine growth when grow size == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowSize = m_nSize / 8;
			nGrowSize = (nGrowSize < 4) ? 4 : ((nGrowSize > 1024) ? 1024 : nGrowSize);
		}
        // Adjust new max size to granularity
        //
		if (nNewSize < m_nMaxSize + nGrowSize)
			nMaxSize = m_nMaxSize + nGrowSize;
		else
            nMaxSize = nNewSize;

        // ReAlloc to new size
        //
        p = (TYPE*)CoTaskMemRealloc((void*)m_items, nMaxSize * sizeof(TYPE));

        if (NULL == p)
            return FALSE;

        // Assign new array data buffer and size
        //
        m_items = p;
        m_nMaxSize = nMaxSize;

        // Construct new elements
        //
        ConstructElements(&m_items[m_nSize], nNewSize-m_nSize);
        m_nSize = nNewSize;
    }
    return TRUE;
}
#endif

#endif // !defined(_ARRTMPL_H_)
