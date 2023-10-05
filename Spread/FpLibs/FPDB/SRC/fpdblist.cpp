#ifdef NO_MFC

#include "stdafx.h"
//ATL$ #include "fpatl.h"
#include "fptools.h"
#include "fptstr.h"


#include "fpdblist.h"

#ifdef ASSERT_VALID
#undef ASSERT_VALID
#endif
#define ASSERT_VALID _ASSERT
#define AfxIsValidAddress(p, s)   TRUE

CPlex* PASCAL CPlex::Create(CPlex*& pHead, UINT nMax, UINT cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	CPlex* p = (CPlex*) new BYTE[sizeof(CPlex) + nMax * cbElement];
			// may throw exception
	p->pNext = pHead;
	pHead = p;  // change head (adds in reverse order for simplicity)
	return p;
}

void CPlex::FreeDataChain()     // free this one and links
{
	CPlex* p = this;
	while (p != NULL)
	{
		BYTE* bytes = (BYTE*) p;
		CPlex* pNext = p->pNext;
		delete[] bytes;
		p = pNext;
	}
}

void* PASCAL CObject::operator new(size_t nSize)
{
	return ::operator new(nSize);
}

void PASCAL CObject::operator delete(void* p)
{
//	free(p);
    delete(p);
}

CObList::CObList(int nBlockSize)
{
	ASSERT(nBlockSize > 0);

	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

void CObList::RemoveAll()
{
	ASSERT_VALID(this);

	// destroy elements
	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

CObList::~CObList()
{
	RemoveAll();
	ASSERT(m_nCount == 0);
}

/////////////////////////////////////////////////////////////////////////////
// Node helpers
/*
 * Implementation note: CNode's are stored in CPlex blocks and
 *  chained together. Free blocks are maintained in a singly linked list
 *  using the 'pNext' member of CNode with 'm_pNodeFree' as the head.
 *  Used blocks are maintained in a doubly linked list using both 'pNext'
 *  and 'pPrev' as links and 'm_pNodeHead' and 'm_pNodeTail'
 *   as the head/tail.
 *
 * We never free a CPlex block unless the List is destroyed or RemoveAll()
 *  is used - so the total number of CPlex blocks may grow large depending
 *  on the maximum past size of the list.
 */

CObList::CNode*
CObList::NewNode(CObList::CNode* pPrev, CObList::CNode* pNext)
{
	if (m_pNodeFree == NULL)
	{
		// add another block
		CPlex* pNewBlock = CPlex::Create(m_pBlocks, m_nBlockSize,
				 sizeof(CNode));

		// chain them into free list
		CNode* pNode = (CNode*) pNewBlock->data();
		// free in reverse order to make it easier to debug
		pNode += m_nBlockSize - 1;
		for (int i = m_nBlockSize-1; i >= 0; i--, pNode--)
		{
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
		}
	}
	ASSERT(m_pNodeFree != NULL);  // we must have something

	CObList::CNode* pNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNode->pPrev = pPrev;
	pNode->pNext = pNext;
	m_nCount++;
	ASSERT(m_nCount > 0);  // make sure we don't overflow




	pNode->data = 0; // start with zero

	return pNode;
}

void CObList::FreeNode(CObList::CNode* pNode)
{

	pNode->pNext = m_pNodeFree;
	m_pNodeFree = pNode;
	m_nCount--;
	ASSERT(m_nCount >= 0);  // make sure we don't underflow

	// if no more elements, cleanup completely
	if (m_nCount == 0)
		RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////

POSITION CObList::AddHead(CObject* newElement)
{
	ASSERT_VALID(this);

	CNode* pNewNode = NewNode(NULL, m_pNodeHead);
	pNewNode->data = newElement;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = pNewNode;
	else
		m_pNodeTail = pNewNode;
	m_pNodeHead = pNewNode;
	return (POSITION) pNewNode;
}

POSITION CObList::AddTail(CObject* newElement)
{
	ASSERT_VALID(this);

	CNode* pNewNode = NewNode(m_pNodeTail, NULL);
	pNewNode->data = newElement;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;
	return (POSITION) pNewNode;
}

void CObList::AddHead(CObList* pNewList)
{
	ASSERT_VALID(this);

	ASSERT(pNewList != NULL);
//	ASSERT_KINDOF(CObList, pNewList);
	ASSERT_VALID(pNewList);

	// add a list of same elements to head (maintain order)
	POSITION pos = pNewList->GetTailPosition();
	while (pos != NULL)
		AddHead(pNewList->GetPrev(pos));
}

void CObList::AddTail(CObList* pNewList)
{
	ASSERT_VALID(this);
	ASSERT(pNewList != NULL);
//	ASSERT_KINDOF(CObList, pNewList);
	ASSERT_VALID(pNewList);

	// add a list of same elements
	POSITION pos = pNewList->GetHeadPosition();
	while (pos != NULL)
		AddTail(pNewList->GetNext(pos));
}

CObject* CObList::RemoveHead()
{
	ASSERT_VALID(this);
	ASSERT(m_pNodeHead != NULL);  // don't call on empty list !!!
	ASSERT(AfxIsValidAddress(m_pNodeHead, sizeof(CNode)));

	CNode* pOldNode = m_pNodeHead;
	CObject* returnValue = pOldNode->data;

	m_pNodeHead = pOldNode->pNext;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = NULL;
	else
		m_pNodeTail = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

CObject* CObList::RemoveTail()
{
	ASSERT_VALID(this);
	ASSERT(m_pNodeTail != NULL);  // don't call on empty list !!!
	ASSERT(AfxIsValidAddress(m_pNodeTail, sizeof(CNode)));

	CNode* pOldNode = m_pNodeTail;
	CObject* returnValue = pOldNode->data;

	m_pNodeTail = pOldNode->pPrev;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = NULL;
	else
		m_pNodeHead = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

POSITION CObList::InsertBefore(POSITION position, CObject* newElement)
{
	ASSERT_VALID(this);

	if (position == NULL)
		return AddHead(newElement); // insert before nothing -> head of the list

	// Insert it before position
	CNode* pOldNode = (CNode*) position;
	CNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
	pNewNode->data = newElement;

	if (pOldNode->pPrev != NULL)
	{
		ASSERT(AfxIsValidAddress(pOldNode->pPrev, sizeof(CNode)));
		pOldNode->pPrev->pNext = pNewNode;
	}
	else
	{
		ASSERT(pOldNode == m_pNodeHead);
		m_pNodeHead = pNewNode;
	}
	pOldNode->pPrev = pNewNode;
	return (POSITION) pNewNode;
}

POSITION CObList::InsertAfter(POSITION position, CObject* newElement)
{
	ASSERT_VALID(this);

	if (position == NULL)
		return AddTail(newElement); // insert after nothing -> tail of the list

	// Insert it before position
	CNode* pOldNode = (CNode*) position;
	ASSERT(AfxIsValidAddress(pOldNode, sizeof(CNode)));
	CNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
	pNewNode->data = newElement;

	if (pOldNode->pNext != NULL)
	{
		ASSERT(AfxIsValidAddress(pOldNode->pNext, sizeof(CNode)));
		pOldNode->pNext->pPrev = pNewNode;
	}
	else
	{
		ASSERT(pOldNode == m_pNodeTail);
		m_pNodeTail = pNewNode;
	}
	pOldNode->pNext = pNewNode;
	return (POSITION) pNewNode;
}

void CObList::RemoveAt(POSITION position)
{
	ASSERT_VALID(this);

	CNode* pOldNode = (CNode*) position;
	ASSERT(AfxIsValidAddress(pOldNode, sizeof(CNode)));

	// remove pOldNode from list
	if (pOldNode == m_pNodeHead)
	{
		m_pNodeHead = pOldNode->pNext;
	}
	else
	{
		ASSERT(AfxIsValidAddress(pOldNode->pPrev, sizeof(CNode)));
		pOldNode->pPrev->pNext = pOldNode->pNext;
	}
	if (pOldNode == m_pNodeTail)
	{
		m_pNodeTail = pOldNode->pPrev;
	}
	else
	{
		ASSERT(AfxIsValidAddress(pOldNode->pNext, sizeof(CNode)));
		pOldNode->pNext->pPrev = pOldNode->pPrev;
	}
	FreeNode(pOldNode);
}


/////////////////////////////////////////////////////////////////////////////
// slow operations

POSITION CObList::FindIndex(int nIndex) const
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nCount)
		return NULL;  // went too far

	CNode* pNode = m_pNodeHead;
	while (nIndex--)
	{
		ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
		pNode = pNode->pNext;
	}
	return (POSITION) pNode;
}

POSITION CObList::Find(CObject* searchValue, POSITION startAfter) const
{
	ASSERT_VALID(this);

	CNode* pNode = (CNode*) startAfter;
	if (pNode == NULL)
	{
		pNode = m_pNodeHead;  // start at head
	}
	else
	{
		ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
		pNode = pNode->pNext;  // start after the one specified
	}

	for (; pNode != NULL; pNode = pNode->pNext)
		if (pNode->data == searchValue)
			return (POSITION) pNode;
	return NULL;
}

//---------------------------------------------------------------------
// SCP: Functions retrieved from templates
//---------------------------------------------------------------------
POSITION CObList::GetHeadPosition() const
{
  return (POSITION)m_pNodeHead;
}

int CObList::GetCount() const
{
  return m_nCount;
}

CObject*& CObList::GetHead()
{
  ASSERT(m_pNodeHead != NULL);
  return m_pNodeHead->data;
}

CObject* CObList::GetHead() const
{
  ASSERT(m_pNodeHead != NULL);
  return m_pNodeHead->data;
}

CObject*& CObList::GetPrev(POSITION& rPosition) // return *Position--
{
  CNode* pNode = (CNode*) rPosition;
  ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
  rPosition = (POSITION) pNode->pPrev;
  return pNode->data;
}

CObject* CObList::GetPrev(POSITION& rPosition) const // return *Position--
{
  CNode* pNode = (CNode*) rPosition;
  ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
  rPosition = (POSITION) pNode->pPrev;
  return pNode->data;
}

POSITION CObList::GetTailPosition() const
{
  return (POSITION) m_pNodeTail;
}

BOOL CObList::IsEmpty() const
{
  return m_nCount == 0;
}

CObject*& CObList::GetNext(POSITION& rPosition) // return *Position++
{
  CNode* pNode = (CNode*) rPosition;
  ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
  rPosition = (POSITION) pNode->pNext;
  return pNode->data;
}

CObject* CObList::GetNext(POSITION& rPosition) const // return *Position++
{
  CNode* pNode = (CNode*) rPosition;
  ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
  rPosition = (POSITION) pNode->pNext;
  return pNode->data;
}

CObject*& CObList::GetAt(POSITION position)
{
  CNode* pNode = (CNode*) position;
  ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
  return pNode->data;
}

CObject* CObList::GetAt(POSITION position) const
{
  CNode* pNode = (CNode*) position;
  ASSERT(AfxIsValidAddress(pNode, sizeof(CNode)));
  return pNode->data;
}




/////////////////////////////////////////////////////////////////////////////

CObArray::CObArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CObArray::~CObArray()
{
	ASSERT_VALID(this);
	delete[] (BYTE*)m_pData;
}

void CObArray::SetSize(int nNewSize, int nGrowBy)
{
	ASSERT_VALID(this);
	ASSERT(nNewSize >= 0);

	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0)
	{
		// shrink to nothing
		delete[] (BYTE*)m_pData;
		m_pData = NULL;
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
#ifdef SIZE_T_MAX
		ASSERT(nNewSize <= SIZE_T_MAX/sizeof(CObject*));    // no overflow
#endif
		m_pData = (CObject**) new BYTE[nNewSize * sizeof(CObject*)];

		memset(m_pData, 0, nNewSize * sizeof(CObject*));  // zero fill

		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
		{
			// initialize the new elements

			memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(CObject*));

		}

		m_nSize = nNewSize;
	}
	else
	{
		// otherwise, grow array
		int nGrowBy = m_nGrowBy;
		if (nGrowBy == 0)
		{
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy = min(1024, max(4, m_nSize / 8));
		}
		int nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy)
			nNewMax = m_nMaxSize + nGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush

		ASSERT(nNewMax >= m_nMaxSize);  // no wrap around
#ifdef SIZE_T_MAX
		ASSERT(nNewMax <= SIZE_T_MAX/sizeof(CObject*)); // no overflow
#endif
		CObject** pNewData = (CObject**) new BYTE[nNewMax * sizeof(CObject*)];

		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * sizeof(CObject*));

		// construct remaining elements
		ASSERT(nNewSize > m_nSize);

		memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(CObject*));


		// get rid of old stuff (note: no destructors called)
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

int CObArray::Append(const CObArray& src)
{
	ASSERT_VALID(this);
	ASSERT(this != &src);   // cannot append to itself

	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);

	memcpy(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(CObject*));

	return nOldSize;
}

void CObArray::Copy(const CObArray& src)
{
	ASSERT_VALID(this);
	ASSERT(this != &src);   // cannot append to itself

	SetSize(src.m_nSize);

	memcpy(m_pData, src.m_pData, src.m_nSize * sizeof(CObject*));

}

void CObArray::FreeExtra()
{
	ASSERT_VALID(this);

	if (m_nSize != m_nMaxSize)
	{
		// shrink to desired size
#ifdef SIZE_T_MAX
		ASSERT(m_nSize <= SIZE_T_MAX/sizeof(CObject*)); // no overflow
#endif
		CObject** pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (CObject**) new BYTE[m_nSize * sizeof(CObject*)];
			// copy new data from old
			memcpy(pNewData, m_pData, m_nSize * sizeof(CObject*));
		}

		// get rid of old stuff (note: no destructors called)
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CObArray::SetAtGrow(int nIndex, CObject* newElement)
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nSize)
		SetSize(nIndex+1);
	m_pData[nIndex] = newElement;
}

int CObArray::Add(CObject* newElement)
{
  int nIndex = m_nSize;
  SetAtGrow(nIndex, newElement);
  return nIndex;
}

void CObArray::InsertAt(int nIndex, CObject* newElement, int nCount)
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);    // will expand to meet need
	ASSERT(nCount > 0);     // zero or negative size not allowed

	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		SetSize(nIndex + nCount);  // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		SetSize(m_nSize + nCount);  // grow it to new size
		// shift old data up to fill gap
		memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(CObject*));

		// re-init slots we copied from

		memset(&m_pData[nIndex], 0, nCount * sizeof(CObject*));

	}

	// insert new value in the gap
	ASSERT(nIndex + nCount <= m_nSize);
	while (nCount--)
		m_pData[nIndex++] = newElement;
}

void CObArray::RemoveAt(int nIndex, int nCount)
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0);
	ASSERT(nCount >= 0);
	ASSERT(nIndex + nCount <= m_nSize);

	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);

	if (nMoveCount)
		memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(CObject*));
	m_nSize -= nCount;
}

void CObArray::InsertAt(int nStartIndex, CObArray* pNewArray)
{
	ASSERT_VALID(this);
	ASSERT(pNewArray != NULL);
//	ASSERT_KINDOF(CObArray, pNewArray);
	ASSERT_VALID(pNewArray);
	ASSERT(nStartIndex >= 0);

	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}

CObject* CObArray::operator[](int nIndex) const
{
  return GetAt(nIndex);
}

CObject*& CObArray::operator[](int nIndex)
{
  return ElementAt(nIndex);
}

int CObArray::GetSize() const
{
  return m_nSize;
}

CObject*& CObArray::ElementAt(int nIndex)
{
  ASSERT(nIndex >= 0 && nIndex < m_nSize);
  return m_pData[nIndex];
}

CObject* CObArray::GetAt(int nIndex) const
{
  ASSERT(nIndex >= 0 && nIndex < m_nSize);
  return m_pData[nIndex];
}


void CObArray::SetAt(int nIndex, CObject* newElement)
{
  ASSERT(nIndex >= 0 && nIndex < m_nSize);
  m_pData[nIndex] = newElement;
}

/////////////////////////////////////////////////////////////////////////////
int CStringArray::Add(LPTSTR tstrVal)
{
  return CObArray::Add(new CfpString(tstrVal));
}


void CStringArray::RemoveAll()
{
  int ct = GetSize();
  for (int i = 0; i < ct; i++)
    delete GetAt(i);

  SetSize(0, -1);
}

CStringArray::~CStringArray()
{
    RemoveAll();
}

CfpString* CStringArray::GetAt(int nIndex) const
{
  return (CfpString*)CObArray::GetAt(nIndex);
}

CfpString* CStringArray::operator[](int nIndex) const
{
  return GetAt(nIndex);
}
CfpString*& CStringArray::operator[](int nIndex)
{
  return (CfpString*&)ElementAt(nIndex);
}

LPTSTR CStringArray::ValueAt(int nIndex)
{
  return GetAt(nIndex)->GetValue();
}


int CDWordArray::Add(DWORD dwVal)
{
  return CObArray::Add(new CfpDWord(dwVal));
}

void CDWordArray::RemoveAll()
{
  int ct = GetSize();
  for (int i = 0; i < ct; i++)
    delete GetAt(i);

  SetSize(0, -1);
}

CDWordArray::~CDWordArray()
{
    RemoveAll();
}

CfpDWord* CDWordArray::GetAt(int nIndex) const
{
  return (CfpDWord*)CObArray::GetAt(nIndex);
}

CfpDWord* CDWordArray::operator[](int nIndex) const
{
  return GetAt(nIndex);
}
CfpDWord*& CDWordArray::operator[](int nIndex)
{
  return (CfpDWord*&)ElementAt(nIndex);
}

DWORD CDWordArray::ValueAt(int nIndex)
{
  return GetAt(nIndex)->GetValue();
}

#endif // NO_MFC
