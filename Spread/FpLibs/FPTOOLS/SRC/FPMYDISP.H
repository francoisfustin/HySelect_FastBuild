// fpmydisp.h : Declaration "stuff" needed in the declaration of 
//              the OLE control class.

// NOTE: FPMYDISP_CTRL_NAME must be declared BEFORE including this file!!!
#ifndef FPMYDISP_CTRL_NAME
#pragma message("*** " __FILE__ " : You MUST define \"FPMYDISP_CTRL_NAME\" before including fpmydisp.h ***" ) 
#endif

// Following creates member variable:  XMyDispatch m_xMyDispatch;
public:
    BEGIN_INTERFACE_PART(MyDispatch, IDispatch)
      INIT_INTERFACE_PART(FPMYDISP_CTRL_NAME, MyDispatch)
      STDMETHOD(GetTypeInfoCount)(unsigned int*);
      STDMETHOD(GetTypeInfo)(unsigned int, LCID, ITypeInfo**);
      STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, unsigned int, LCID, DISPID*);
      STDMETHOD(Invoke)(DISPID, REFIID, LCID, unsigned short, DISPPARAMS*,
 					  VARIANT*, EXCEPINFO*, unsigned int*);
    END_INTERFACE_PART(MyDispatch)

friend class XMyDispatch;

    void  mydpSetProp(DISPID dispID, long lSetIndex, VARIANT FAR *lpVar);
    DWORD mydpGetProp(DISPID dispID, long lGetIndex);

protected:
    long  mydpInitTable(FP_PROPLIST FAR *propList, int iListCt);
    WORD  mydpGetPropIDFromDispID(DISPID dispID);
#ifdef NO_MFC
	static UINT GetEntryCount(const AFX_DISPMAP* pDispMap);
	const AFX_DISPMAP_ENTRY* GetDispEntry(LONG memid);
	static LONG MemberIDFromName(const AFX_DISPMAP* pDispMap, LPCTSTR lpszName);
#endif  // NO_MFC

    // Static member variable used by the above functions.
    // NOTE: "static" means only ONE of these is created for ALL instances
    //       of the control.
    static CfpIDispatchTbl m_mydpIDispTbl;

