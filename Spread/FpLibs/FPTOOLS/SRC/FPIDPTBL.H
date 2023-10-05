/***********************************************************************
* FPIDPTBL.H : Declaration of the CfpIDispatchTbl class.
*
* Copyright (C) 1991-1996 - FarPoint Technologies, Inc.
* All Rights Reserved.
*
* No part of this source code may be copied, modified or reproduced
* in any form without retaining the above copyright notice.  This
* source code, or source code derived from it, may not be redistributed
* without express written permission of FarPoint Technologies.
***********************************************************************/

#if (!defined(FPIDPTBL_H) && !defined(NO_MFC))
#define FPIDPTBL_H

class CfpIDispatchTbl : public CObject
{
// Constructor
public:
	CfpIDispatchTbl();

public:
	virtual ~CfpIDispatchTbl();


public:
	long BuildDispatchMap(
      FP_PROPLIST FAR *propList, 
      int iListCt, AFX_PMSG pfnDefGet, AFX_PMSG pfnDefSet, 
      const AFX_DISPMAP_ENTRY FAR *defDispatchMap, int iDefCt);

protected:
    void Init();
    long AllocDispatchTbl(int numOfEntries);
    long AddPropEntry(FP_PROPLIST FAR *pPropEntry, WORD wPropID,
           AFX_PMSG pfnDefGet, AFX_PMSG pfnDefSet);
    long AddDispatchEntry(const AFX_DISPMAP_ENTRY FAR *pDispEntry, int index);
    long AddPropId(int index, WORD wPropID);

public:
    AFX_DISPMAP_ENTRY FAR *m_dispatchTbl;
    WORD FAR *m_PropIdTbl;     // array of PropID's indexed by (DispID-1)
    int       m_PropIdTblCt;   // number of entries in table 

protected:
    int m_iTblSizeCt;          // number of entries allocated
    int m_iLastEntryIndex;     // index of last entry in use
    int m_iLastDispidIndex;    // index of last entry with DispID > 0
};

/*-----------------------------------------------------------------------
 * WIN32
 * Macros to use instead of MFC macros   (prefixed with "FP_")
 *----------------------------------------------------------------------*/
#ifdef WIN32
#ifdef VC50
/*-----------------------------------------------------------------------
 * WIN32 / VC 5.0  -  Macros
 *----------------------------------------------------------------------*/
#define FP_BEGIN_DISPATCH_MAP(theClass, baseClass) \
	const AFX_DISPMAP* PASCAL theClass::_GetBaseDispatchMap() \
		{ return &baseClass::dispatchMap; } \
	const AFX_DISPMAP* theClass::GetDispatchMap() const \
		{ return (const AFX_DISPMAP*)&theClass::MyDispatchMap; } \
	AFX_DISPMAP theClass::MyDispatchMap = \
		{ &theClass::_GetBaseDispatchMap, &theClass::_dispatchEntries[0], \
			&theClass::_dispatchEntryCount, &theClass::_dwStockPropMask }; \
	UINT theClass::_dispatchEntryCount = (UINT)-1; \
	DWORD theClass::_dwStockPropMask = (DWORD)-1; \
	const AFX_DISPMAP_ENTRY theClass::_dispatchEntries[] = \
	{ \

#define FP_DECLARE_DISPATCH_MAP() \
private: \
	static const AFX_DISPMAP_ENTRY _dispatchEntries[]; \
	static UINT _dispatchEntryCount; \
	static DWORD _dwStockPropMask; \
protected: \
 /* static AFX_DATA const AFX_DISPMAP dispatchMap; */ \
	static AFX_DATA AFX_DISPMAP MyDispatchMap; \
	static const AFX_DISPMAP* PASCAL _GetBaseDispatchMap(); \
	virtual const AFX_DISPMAP* GetDispatchMap() const; \

#else // not VC50
/*-----------------------------------------------------------------------
 * WIN32 / VC 4.x  -  Macros
 *----------------------------------------------------------------------*/
#define FP_BEGIN_DISPATCH_MAP(theClass, baseClass) \
	const AFX_DISPMAP* PASCAL theClass::_GetBaseDispatchMap() \
		{ return &baseClass::dispatchMap; } \
	const AFX_DISPMAP* theClass::GetDispatchMap() const \
		{ return (const AFX_DISPMAP*)&theClass::MyDispatchMap; } \
	AFX_DISPMAP theClass::MyDispatchMap = \
		{ &theClass::_GetBaseDispatchMap, &theClass::_dispatchEntries[0], \
			&theClass::_dispatchEntryCount }; \
	UINT theClass::_dispatchEntryCount = (UINT)-1; \
	const AFX_DISPMAP_ENTRY theClass::_dispatchEntries[] = \
	{ \

#define FP_DECLARE_DISPATCH_MAP() \
private: \
	static const AFX_DISPMAP_ENTRY _dispatchEntries[]; \
	static UINT _dispatchEntryCount; \
protected: \
 /* static AFX_DATA const AFX_DISPMAP dispatchMap; */ \
	static AFX_DATA AFX_DISPMAP MyDispatchMap; \
	static const AFX_DISPMAP* PASCAL _GetBaseDispatchMap(); \
	virtual const AFX_DISPMAP* GetDispatchMap() const; \

#endif //if-else VC50
#else  //WIN16
/*-----------------------------------------------------------------------
 * WIN16
 * Macros to use instead of MFC macros   (prefixed with "FP_")
 *----------------------------------------------------------------------*/
#define FP_BEGIN_DISPATCH_MAP(theClass, baseClass) \
	AFX_DISPMAP FAR* theClass::GetDispatchMap() const \
		{ return &theClass::MyDispatchMap; } \
/*	AFX_DISPMAP BASED_CODE theClass::dispatchMap = \
		{ &(baseClass::dispatchMap), theClass::_dispatchEntries }; */ \
	AFX_DISPMAP theClass::MyDispatchMap = \
		{ &(baseClass::dispatchMap), theClass::_dispatchEntries }; \
	AFX_DISPMAP_ENTRY BASED_CODE theClass::_dispatchEntries[] = \
	{

#define FP_DECLARE_DISPATCH_MAP() \
private: \
	static AFX_DISPMAP_ENTRY BASED_CODE _dispatchEntries[]; \
protected: \
 /* static AFX_DISPMAP BASED_CODE dispatchMap; */ \
	static AFX_DISPMAP MyDispatchMap; \
	virtual AFX_DISPMAP FAR* GetDispatchMap() const;

#endif //if-else WIN32

#define FP_END_DISPATCH_MAP() \
	{ VTS_NONE, DISPID_UNKNOWN, VTS_NONE, VT_VOID, \
		(AFX_PMSG)NULL, (AFX_PMSG)NULL, (size_t)-1, afxDispCustom } }; \

#endif
