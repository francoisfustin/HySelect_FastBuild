/****************************************************************************
* FPATLCOR.H  -  Declaration of Font & Picture classes for ATL
*
*
*
*
*
*
*
*
*
*
****************************************************************************/
#ifndef FPATLCOR_H
#define FPATLCOR_H

class CfpAtlBase;

#define RELEASE(lpUnk) do \
	{ if ((lpUnk) != NULL) { (lpUnk)->Release(); (lpUnk) = NULL; } } while (0)

//---------------------------------------------------------------------------
//
// class CPictureHolder - helper class for dealing with picture objects
//
//---------------------------------------------------------------------------
class CPictureHolder
{
// Constructors
public:
	CPictureHolder();

// Attributes
	LPPICTURE m_pPict;

// Operations
	BOOL CreateEmpty();

	BOOL CreateFromBitmap(UINT idResource);
//	BOOL CreateFromBitmap(CBitmap* pBitmap, CPalette* pPal = NULL,
//		BOOL bTransferOwnership = TRUE);
	BOOL CreateFromBitmap(HBITMAP hbm, HPALETTE hpal = NULL,
		BOOL bTransferOwnership = FALSE);

	BOOL CreateFromMetafile(HMETAFILE hmf, int xExt, int yExt,
		BOOL bTransferOwnership = FALSE);

	BOOL CreateFromIcon(UINT idResource);
	BOOL CreateFromIcon(HICON hIcon, BOOL bTransferOwnership = FALSE);

	short GetType();
	BOOL GetDisplayString(LPSTR strValue);
	LPPICTUREDISP GetPictureDispatch();
	void SetPictureDispatch(LPPICTUREDISP pDisp);
	void Render(HDC hDC, LPCRECT prcRender, LPCRECT prcWBounds);

// Implementation
public:
	~CPictureHolder();
};


//---------------------------------------------------------------------------
//
// class CfpEnumOleVerb
//
//---------------------------------------------------------------------------
//typedef CComObjectRootEx<CComGlobalsThreadModel> CfpComGlobalObjectRoot;
//class CfpEnumOleVerb : public CfpComGlobalObjectRoot, public IEnumOLEVERB
class CfpEnumOleVerb : public CComObjectRoot, public IEnumOLEVERB
{
// Constructors
public:
	CfpEnumOleVerb(const void* pvEnum, UINT nCount, BOOL bNeedFree = TRUE);

// Implementation
public:
	virtual ~CfpEnumOleVerb();

protected:
	size_t m_nSizeElem;     // size of each item in the array
	CfpEnumOleVerb* m_pClonedFrom;  // used to keep original alive for clones

	BYTE* m_pvEnum;     // pointer data to enumerate
	UINT m_nCurPos;     // current position in m_pvEnum
	UINT m_nSize;       // total number of items in m_pvEnum
	BOOL m_bNeedFree;   // free on release?

    virtual BOOL OnNext(void* pv, HRESULT *retError);
	virtual BOOL OnSkip();
	virtual void OnReset();
	virtual CfpEnumOleVerb* OnClone();

// Interface Maps
public:
	STDMETHOD_(ULONG, AddRef)()  {return InternalAddRef();}

	STDMETHOD_(ULONG, Release)() 
    {
      DWORD dwSaveRef = m_dwRef;
      ULONG ulRet = InternalRelease();
      if (dwSaveRef == 1)
        delete this;
      return ulRet;
    }

	STDMETHOD(QueryInterface)(REFIID iid, LPVOID*ppvObj)
      { return _InternalQueryInterface(iid, ppvObj); }

	STDMETHOD(Next)(ULONG celt, LPOLEVERB rgelt, ULONG *pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)(void);
	STDMETHOD(Clone)(LPENUMOLEVERB *ppenum);

BEGIN_COM_MAP(CfpEnumOleVerb)
	COM_INTERFACE_ENTRY(IEnumOLEVERB)
FP_END_COM_MAP()
};

//---------------------------------------------------------------------------
//
// class CfpAtlFont
//
//---------------------------------------------------------------------------
typedef CComPtr<IFontDisp> CfpAtlComFontPtr;

class CfpAtlFont : public CComObjectRoot,
  public CfpAtlComFontPtr, public IPropertyNotifySink
{
public:
    CfpAtlFont();
    virtual ~CfpAtlFont();

    void Init(CfpAtlBase *lpCtl, DISPID dispID);
     
	IFontDisp* operator=(IFontDisp* lp);

	STDMETHOD_(ULONG, AddRef)()  {return InternalAddRef();}

	STDMETHOD_(ULONG, Release)() {return InternalRelease();}

	STDMETHOD(QueryInterface)(REFIID iid, LPVOID*ppvObj)
      { return _InternalQueryInterface(iid, ppvObj); }

    STDMETHOD(OnChanged)(DISPID dispID);
    STDMETHOD(OnRequestEdit)(DISPID dispID);

BEGIN_COM_MAP(CfpAtlFont)
	COM_INTERFACE_ENTRY(IPropertyNotifySink)
FP_END_COM_MAP()

    CfpAtlBase *m_pOwner;
    DWORD       m_dwConnectCookie;
    BOOL        m_fAdvise;
    DISPID      m_dispID;
};
#endif // FPATLCOR_H