#ifdef NO_MFC	// ATL inplementation:

#ifndef THIS_CLASS
#pragma message("*** " __FILE__ " : You MUST define \"THIS_CLASS\" before including fpOleDrg.h ***" ) 
#endif

protected:
	FORMATETC m_Formats[8]; // array of supported formatetc
	STGMEDIUM m_Mediums[8]; // array of stgmedium

	short m_nDragButton;	// button used for a drag operation
	short ClipFormatToArrayIndex(CLIPFORMAT cf);
	long					m_lEffect;
	CfpDataObject*			m_pfpDataObject;	// for OLEDrag and XfpDataObject::GetData
	OLEDropModeConstants	m_OLEDropMode;
#ifdef DRAGAUTO
	OLEDragModeConstants    m_OLEDragMode;
	POINT					m_ptAnchor;		// anchor point for OLE drag/drop operation -scl
#define DRAG_DELTA	3	// distance in pixels from the anchor point that the mouse must move
						// to initiate an OLE drag/drop operation -scl
#endif	
//	BOOL					m_bDragging;	// currently dragging?

	STDMETHOD(OnDragOver)(LPDATAOBJECT pDataObject, ULONG* pdwEffect, ULONG grfKeyState, POINTL pt, short State);
	STDMETHOD(OLEDrag)(void);
	STDMETHOD(get_OLEDropMode)(OLEDropModeConstants* pVal);
    STDMETHOD(put_OLEDropMode)(OLEDropModeConstants newVal);
#ifdef DRAGAUTO
	STDMETHOD(get_OLEDragMode)(OLEDragModeConstants* pVal);
    STDMETHOD(put_OLEDragMode)(OLEDragModeConstants newVal);
#endif
public:
// IDropSource
	STDMETHOD(QueryContinueDrag)(int, ULONG);
	STDMETHOD(GiveFeedback)(ULONG);
// IDropTarget
	STDMETHOD(DragEnter)(LPDATAOBJECT, ULONG, POINTL, ULONG*);
	STDMETHOD(DragOver)(ULONG, POINTL, ULONG*);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(LPDATAOBJECT, ULONG, POINTL, ULONG*);
// IDataObject
	STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
	STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
	STDMETHOD(QueryGetData)(LPFORMATETC);
	STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
	STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
	STDMETHOD(EnumFormatEtc)(ULONG, LPENUMFORMATETC FAR*);
	STDMETHOD(DAdvise)(struct tagFORMATETC *,ULONG,struct IAdviseSink *,ULONG *);
	STDMETHOD(DUnadvise)(ULONG);
	STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*);
	
	class CfpEnumFormatEtc : public IEnumFORMATETC
	{
	private:
		long			m_lRefCount;	// ref count
		THIS_CLASS*		m_pCtl;		    // ptr to class implementing IDataObject (control class)
		DWORD			m_dwDir;		// either DATADIR_GET or DATADIR_SET
	public:
		long	m_lCurrent;		// index of next item to enumerate
		CfpEnumFormatEtc(THIS_CLASS* pCtl, DWORD dwDir)
			: m_pCtl(pCtl),
			  m_dwDir(dwDir),
			  m_lRefCount(1), 
			  m_lCurrent(0)
		{
		}
		STDMETHOD(QueryInterface)(REFIID iid, void** ppvObj)
		{
			if (!ppvObj)
				return E_POINTER;
			*ppvObj = NULL;
			if (IID_IUnknown == iid || IID_IEnumFORMATETC == iid)
				*ppvObj = this;

			if (NULL != *ppvObj)
			{
				((LPUNKNOWN)*ppvObj)->AddRef();
				return S_OK;
			}

			return E_NOINTERFACE;
		}
		STDMETHOD_(ULONG,AddRef)()
		{
			return ++m_lRefCount;
		}
		STDMETHOD_(ULONG,Release)()
		{
		   long tempCount;

		   tempCount = --m_lRefCount;
		   if(tempCount==0)
			  delete this;
		   return tempCount; 
		}
		STDMETHOD(Next)(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched)
		{
			long	lIndex1;
			ULONG	lIndex2 = 0;
			
			if (pceltFetched != NULL)
				*pceltFetched = 0;

			// If num of elements is 0, then just return
			if (!celt)
			  return S_OK;

			// If invalid ptr, then return
			if (!rgelt)
			  return E_POINTER;

			// Retrieve the next celt elements.
			for (lIndex1 = m_lCurrent;
				 lIndex1 < 8 && lIndex2 < celt;
				 lIndex1++)
			{
				if (DATADIR_GET == m_dwDir)
				{
					if (TYMED_NULL != m_pCtl->m_Mediums[lIndex1].tymed)
					{
						rgelt[lIndex2] = m_pCtl->m_Formats[lIndex1];
						lIndex2++;
					}
				}
				else	// DATADIR_SET
				{
					rgelt[lIndex2] = m_pCtl->m_Formats[lIndex1];
					lIndex2++;
				}
			}

			// Set count of elements retrieved
			if (pceltFetched != NULL)
				*pceltFetched = lIndex2;
			m_lCurrent = lIndex1;

			return  (lIndex2 < celt) ? ResultFromScode(S_FALSE) : NOERROR;
		}
		STDMETHOD(Skip)(ULONG celt)
		{
			m_lCurrent += celt; 
			if (m_lCurrent >= 8L)
			{
				m_lCurrent =  7;
				return ResultFromScode(S_FALSE);
			} 
			else 
				return NOERROR;
		}
		STDMETHOD(Reset)(void)
		{
			m_lCurrent = 0;
			return S_OK;
		}
		STDMETHOD(Clone)(IEnumFORMATETC** ppenum)
		{
			if (!ppenum)
				return E_POINTER;

			CfpEnumFormatEtc FAR* penum = NULL;

			penum = new CfpEnumFormatEtc(m_pCtl, m_dwDir);
			penum->m_lCurrent = m_lCurrent; 

			*ppenum = (IEnumFORMATETC*)penum;
			return NOERROR;
		}
	};

	friend class CfpEnumFormatEtc;

#else // MFC implementation:
// NOTE: FPMYDISP_CTRL_NAME must be declared BEFORE including this file!!!
#ifndef FPMYDISP_CTRL_NAME
#pragma message("*** " __FILE__ " : You MUST define \"FPMYDISP_CTRL_NAME\" before including fpprpbag.h ***" ) 
#endif

public:
// Following creates member variable:  XfpDropSource m_xfpDropSource;
	BEGIN_INTERFACE_PART(fpDropSource, IDropSource)
		// need a constructor
		INIT_INTERFACE_PART(FPMYDISP_CTRL_NAME, fpDropSource)
		STDMETHOD(QueryContinueDrag)(int, ULONG);
		STDMETHOD(GiveFeedback)(ULONG);
		// member variable used when the right mouse button
		// is used for the drag
protected:
		short m_nDragButton;	// button used for a drag operation
		// ^- need to be able to access this from the control class
		friend class FPMYDISP_CTRL_NAME;
	END_INTERFACE_PART(fpDropSource)

// Following creates member variable:  XfpDropTarget m_xfpDropTarget;
	BEGIN_INTERFACE_PART(fpDropTarget, IDropTarget)
		INIT_INTERFACE_PART(FPMYDISP_CTRL_NAME, fpDropTarget)
		STDMETHOD(DragEnter)(LPDATAOBJECT, ULONG, POINTL, ULONG*);
		STDMETHOD(DragOver)(ULONG, POINTL, ULONG*);
		STDMETHOD(DragLeave)();
		STDMETHOD(Drop)(LPDATAOBJECT, ULONG, POINTL, ULONG*);
		// function to which XfpDropTarget::DragEnter, XfpDropTarget::DragOver,
		// and XfpDropTarget::DragLeave() are routed to fire the OLEDragOver event. -scl
protected:
		friend class FPMYDISP_CTRL_NAME;
	END_INTERFACE_PART(fpDropTarget)

// fix for SEL5002 -scl
	class CfpEnumFormatEtc; // forward declaration
// fix for SEL5002 -scl

// Following creates member variable:  XfpDataObject m_xfpDataObject;
	BEGIN_INTERFACE_PART(fpDataObject, IDataObject)
		XfpDataObject();	// constructor
		~XfpDataObject();
		INIT_INTERFACE_PART(FPMYDISP_CTRL_NAME, fpDataObject)
		STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(QueryGetData)(LPFORMATETC);
		STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
		STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, int);
		STDMETHOD(EnumFormatEtc)(ULONG, LPENUMFORMATETC FAR*);
		STDMETHOD(DAdvise)(struct tagFORMATETC *,ULONG,struct IAdviseSink *,ULONG *);
		STDMETHOD(DUnadvise)(ULONG);
		STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*);
protected:
		FORMATETC m_Formats[8]; // array of supported formatetc
		STGMEDIUM m_Mediums[8]; // array of stgmedium

		short ClipFormatToArrayIndex(CLIPFORMAT cf);
		friend class CfpEnumFormatEtc; // needs access to arrays above -scl
    END_INTERFACE_PART(fpDataObject)

// fix for SEL5002 -scl
	class CfpEnumFormatEtc : public IEnumFORMATETC
	{
	private:
		long			m_lRefCount;	// ref count
		XfpDataObject*	m_pXfpDataObject;	// ptr to class implementing IDataObject
		DWORD			m_dwDir;		// either DATADIR_GET or DATADIR_SET
	public:
		long	m_lCurrent;		// index of next item to enumerate
		CfpEnumFormatEtc(XfpDataObject* pDataObject, DWORD dwDir)
			: m_pXfpDataObject(pDataObject),
			  m_dwDir(dwDir),
			  m_lRefCount(1), 
			  m_lCurrent(0)
		{
		}
		STDMETHOD(QueryInterface)(REFIID iid, void** ppvObj)
		{
			if (!ppvObj)
				return E_POINTER;
			*ppvObj = NULL;
			if (IID_IUnknown == iid || IID_IEnumFORMATETC == iid)
				*ppvObj = this;

			if (NULL != *ppvObj)
			{
				((LPUNKNOWN)*ppvObj)->AddRef();
				return S_OK;
			}

			return E_NOINTERFACE;
		}
		STDMETHOD_(ULONG,AddRef)()
		{
			return ++m_lRefCount;
		}
		STDMETHOD_(ULONG,Release)()
		{
		   long tempCount;

		   tempCount = --m_lRefCount;
		   if(tempCount==0)
			  delete this;
		   return tempCount; 
		}
		STDMETHOD(Next)(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched)
		{
			long	lIndex1;
			ULONG	lIndex2 = 0;
			
			if (pceltFetched != NULL)
				*pceltFetched = 0;

			// If num of elements is 0, then just return
			if (!celt)
			  return S_OK;

			// If invalid ptr, then return
			if (!rgelt)
			  return E_POINTER;

			// Retrieve the next celt elements.
			for (lIndex1 = m_lCurrent;
				 lIndex1 < 8 && lIndex2 < celt;
				 lIndex1++)
			{
				if (DATADIR_GET == m_dwDir)
				{
					if (TYMED_NULL != m_pXfpDataObject->m_Mediums[lIndex1].tymed)
					{
						rgelt[lIndex2] = m_pXfpDataObject->m_Formats[lIndex1];
						lIndex2++;
					}
				}
				else	// DATADIR_SET
				{
					rgelt[lIndex2] = m_pXfpDataObject->m_Formats[lIndex1];
					lIndex2++;
				}
			}

			// Set count of elements retrieved
			if (pceltFetched != NULL)
				*pceltFetched = lIndex2;
			m_lCurrent = lIndex1;

			return  (lIndex2 < celt) ? ResultFromScode(S_FALSE) : NOERROR;
		}
		STDMETHOD(Skip)(ULONG celt)
		{
			m_lCurrent += celt; 
			if (m_lCurrent >= 8L)
			{
				m_lCurrent =  7;
				return ResultFromScode(S_FALSE);
			} 
			else 
				return NOERROR;
		}
		STDMETHOD(Reset)(void)
		{
			m_lCurrent = 0;
			return S_OK;
		}
		STDMETHOD(Clone)(IEnumFORMATETC** ppenum)
		{
			if (!ppenum)
				return E_POINTER;

			CfpEnumFormatEtc FAR* penum = NULL;

			penum = new CfpEnumFormatEtc(m_pXfpDataObject, m_dwDir);
			penum->m_lCurrent = m_lCurrent; 

			*ppenum = (IEnumFORMATETC*)penum;
			return NOERROR;
		}
	};
// fix for SEL5002 -scl

protected:
		long			m_lEffect;
		CfpDataObject*	m_pfpDataObject;	// for OLEDrag and XfpDataObject::GetData
		// ^- need to be able to access this from the XfpDataObject class
		friend class XfpDataObject;
		short			m_OLEDropMode;
		
		STDMETHOD(OnDragOver)(LPDATAOBJECT pDataObject, ULONG* pdwEffect, ULONG grfKeyState, POINTL pt, short State);
public:
	void FireOLECompleteDrag(long* Effect)
		{FireEvent(eventidOLECompleteDrag,EVENT_PARAM(VTS_PI4), Effect);}
	
	void FireOLEDragDrop(CfpDataObject** DataObject, long* Effect, short* Button, short* Shift, float* X, float* Y)
		{FireEvent(eventidOLEDragDrop,EVENT_PARAM(VTS_PDISPATCH  VTS_PI4  VTS_PI2  VTS_PI2 VTS_PR4 VTS_PR4), DataObject, Effect, Button, Shift, X, Y);}
	
	void FireOLEDragOver(CfpDataObject** DataObject, long* Effect, short* Button, short* Shift, float* X, float* Y, short* State)
		{FireEvent(eventidOLEDragOver,EVENT_PARAM(VTS_PDISPATCH  VTS_PI4  VTS_PI2  VTS_PI2 VTS_PR4 VTS_PR4  VTS_PI2), DataObject,  Effect, Button, Shift, X, Y, State);}
	
	void FireOLEGiveFeedback(long* Effect, BOOL* DefaultCursors)
		{FireEvent(eventidOLEGiveFeedback,EVENT_PARAM(VTS_PI4  VTS_PBOOL), Effect, DefaultCursors);}
	void FireOLESetData(CfpDataObject** DataObject, short* Format)
		{FireEvent(eventidOLESetData,EVENT_PARAM(VTS_PDISPATCH  VTS_PI2), DataObject, Format);}
	void FireOLEStartDrag(CfpDataObject** DataObject, long* AllowedEffects)
		{FireEvent(eventidOLEStartDrag,EVENT_PARAM(VTS_PDISPATCH  VTS_PI4), DataObject, AllowedEffects);}

	afx_msg short GetOLEDropMode();
    afx_msg void SetOLEDropMode(short newValue);
	afx_msg	void OLEDrag(void);

#endif	// MFC implementation

/*
//#include <windows.h>
//#include <oleauto.h>
#include <fpdatobj.h>

class CfpDropSource : public IDropSource
{
private:
	long				m_lRefCount;
	ULONG				m_dwDragButton;
	FPMYDISP_CTRL_NAME*	m_pCtrl;

public:
	CfpDropSource(FPMYDISP_CTRL_NAME*, ULONG);
	~CfpDropSource();
	STDMETHOD(QueryInterface)(REFIID, void**);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();
	STDMETHOD(QueryContinueDrag)(int, ULONG);
	STDMETHOD(GiveFeedback)(ULONG);

	friend class FPMYDISP_CTRL_NAME;	// to access event firing functions
	friend class CfpDataObject;
};

class CfpDropTarget : public IDropTarget
{
private:
	long				m_lRefCount;
	FPMYDISP_CTRL_NAME*	m_pCtrl;

	HRESULT OnDragOver(ULONG* pdwEffect, ULONG dwKeyState, POINTL pt, short State);

public:
	CfpDropTarget(FPMYDISP_CTRL_NAME*);
	~CfpDropTarget();
	STDMETHOD(QueryInterface)(REFIID, void**);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();
	STDMETHOD(DragEnter)(LPDATAOBJECT, ULONG, POINTL, ULONG*);
	STDMETHOD(DragOver)(ULONG, POINTL, ULONG*);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(LPDATAOBJECT, ULONG, POINTL, ULONG*);

	friend class FPMYDISP_CTRL_NAME;	// to access event firing functions
	friend class CfpDataObject;			// to access protected Get/SetIDataObject()
};
*/