// NOTE:  This file is only valid when "NO_MFC" is defined.
// . . . or when SS40 is defined, for Spread 4 DAO binding
#if defined(NO_MFC) && !defined(FPDBLIST_H)
#define FPDBLIST_H

// setup default packing value
#ifndef _AFX_PACKING
	#define _AFX_PACKING    4   // default packs structs at 4 bytes
#endif

#ifndef ASSERT
#define ASSERT(x) _ASSERT(x)
#endif

#ifndef SS40

#define BEGIN_INTERFACE_PART(cls, interf)
#define INIT_INTERFACE_PART(cls, interf)
#define END_INTERFACE_PART(cls)

#else

#define BEGIN_INTERFACE_PART(localClass, baseClass) \
	class X##localClass : public baseClass \
	{ \
	public: \
		STDMETHOD_(ULONG, AddRef)(); \
		STDMETHOD_(ULONG, Release)(); \
		STDMETHOD(QueryInterface)(REFIID iid, LPVOID* ppvObj); \

#define INIT_INTERFACE_PART(theClass, localClass) \
		size_t m_nOffset; \
		INIT_INTERFACE_PART_DERIVE(theClass, localClass) \

#define INIT_INTERFACE_PART_DERIVE(theClass, localClass) \
		X##localClass() \
			{ m_nOffset = offsetof(theClass, m_x##localClass); } \

#define END_INTERFACE_PART(localClass) \
	} m_x##localClass; \
	friend class X##localClass; \

#ifndef offsetof
#define offsetof(s,m)   (size_t)&(((s *)0)->m)
#endif

#endif

#ifndef VC8
struct __POSITION { };
typedef __POSITION* POSITION;

struct CPlex     // warning variable length structure
{
	CPlex* pNext;
#if (_AFX_PACKING >= 8)
	DWORD dwReserved[1];    // align on 8 byte boundary
#endif
	// BYTE data[maxNum*elementSize];

	void* data() { return this+1; }

	static CPlex* PASCAL Create(CPlex*& head, UINT nMax, UINT cbElement);
			// like 'calloc' but no zero fill
			// may throw memory exceptions

	void FreeDataChain();       // free this one and links
};
#endif

#define CObject FP_CObject 
class CObject
{
public:
	void* PASCAL operator new(size_t nSize);
	void PASCAL operator delete(void* p);
};

#define CObList FP_CObList 
class CObList : public CObject
{
protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		CObject* data;
	};
public:

// Construction
	CObList(int nBlockSize = 10);

// Attributes (head and tail)
	// count of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// peek at head or tail
	CObject*& GetHead();
	CObject* GetHead() const;
	CObject*& GetTail();
	CObject* GetTail() const;

// Operations
	// get head or tail (and remove it) - don't call on empty list!
	CObject* RemoveHead();
	CObject* RemoveTail();

	// add before head or after tail
	POSITION AddHead(CObject* newElement);
	POSITION AddTail(CObject* newElement);

	// add another list of elements before head or after tail
	void AddHead(CObList* pNewList);
	void AddTail(CObList* pNewList);

	// remove all elements
	void RemoveAll();

	// iteration
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	CObject*& GetNext(POSITION& rPosition); // return *Position++
	CObject* GetNext(POSITION& rPosition) const; // return *Position++
	CObject*& GetPrev(POSITION& rPosition); // return *Position--
	CObject* GetPrev(POSITION& rPosition) const; // return *Position--

	// getting/modifying an element at a given position
	CObject*& GetAt(POSITION position);
	CObject* GetAt(POSITION position) const;
	void SetAt(POSITION pos, CObject* newElement);
	void RemoveAt(POSITION position);

	// inserting before or after a given position
	POSITION InsertBefore(POSITION position, CObject* newElement);
	POSITION InsertAfter(POSITION position, CObject* newElement);

	// helper functions (note: O(n) speed)
	POSITION Find(CObject* searchValue, POSITION startAfter = NULL) const;
						// defaults to starting at the HEAD
						// return NULL if not found
	POSITION FindIndex(int nIndex) const;
						// get the 'nIndex'th element (may return NULL)

// Implementation
protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	int m_nCount;
	CNode* m_pNodeFree;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CNode* NewNode(CNode*, CNode*);
	void FreeNode(CNode*);

public:
	virtual ~CObList();

	// local typedefs for class templates
	typedef CObject* BASE_TYPE;
	typedef CObject* BASE_ARG_TYPE;
};

#define CObArray FP_CObArray 
class CObArray : public CObject
{

public:

// Construction
	CObArray();

// Attributes
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

// Operations
	// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	CObject* GetAt(int nIndex) const;
	void SetAt(int nIndex, CObject* newElement);
	CObject*& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const CObject** GetData() const;
	CObject** GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, CObject* newElement);
	int Add(CObject* newElement);
	int Append(const CObArray& src);
	void Copy(const CObArray& src);

	// overloaded operator helpers
	CObject* operator[](int nIndex) const;
	CObject*& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, CObject* newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CObArray* pNewArray);

// Implementation
protected:
	CObject** m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	virtual ~CObArray();

protected:
	// local typedefs for class templates
	typedef CObject* BASE_TYPE;
	typedef CObject* BASE_ARG_TYPE;
};

class CfpString : public CObject
{
public:
  LPTSTR m_tstr;
  CfpString() 
    { m_tstr = NULL; }
  CfpString(LPTSTR val) 
    {
	  m_tstr = (val ? fpSysAllocTString(val) : NULL); 
	}

  virtual ~CfpString()
    { if (m_tstr) fpSysFreeTString(m_tstr); }

  LPTSTR GetValue()
    { return m_tstr; }

};

class CfpDWord : public CObject
{
public:
  DWORD m_dw;
  CfpDWord() 
    { m_dw = 0; }
  CfpDWord(DWORD val) 
    { m_dw = val; }

  CfpDWord* GetAt(int nIndex) const;

  DWORD GetValue()
    { return m_dw; }
};

#define CStringArray FP_CStringArray 
class CStringArray : public CObArray
{
public:
  virtual ~CStringArray();

  int Add(LPTSTR tstrVal);
  void RemoveAll();

  CfpString* GetAt(int nIndex) const;
  LPTSTR ValueAt(int nIndex);

  CfpString* operator[](int nIndex) const;
  CfpString*& operator[](int nIndex);

};

#define CDWordArray  FP_CDWordArray
class CDWordArray : public CObArray
{
public:
  virtual ~CDWordArray();

  int Add(DWORD dwVal);
  void RemoveAll();

  CfpDWord* GetAt(int nIndex) const;
  DWORD ValueAt(int nIndex);

  CfpDWord* operator[](int nIndex) const;
  CfpDWord*& operator[](int nIndex);

};


#endif // FPDBLIST_H
