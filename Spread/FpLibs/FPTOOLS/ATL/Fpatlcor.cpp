/****************************************************************************
* FPATLCOR.CPP  -  Implementation of core (Font & Picture) classes for ATL
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
//#include "ctlcore.h"
#include "stdafx.h"
#include "fpatl.h"
#include "fputil.h"

#define HIMETRIC_PER_INCH   2540


CPictureHolder::CPictureHolder() :
	m_pPict(NULL)
{
}

CPictureHolder::~CPictureHolder()
{
	RELEASE(m_pPict);
}

BOOL CPictureHolder::CreateEmpty()
{
	RELEASE(m_pPict);
	PICTDESC pdesc;
	pdesc.cbSizeofstruct = sizeof(pdesc);
	pdesc.picType = PICTYPE_NONE;
	return SUCCEEDED(OleCreatePictureIndirect(&pdesc, IID_IPicture, FALSE,
		(LPVOID*)&m_pPict));
}

BOOL CPictureHolder::CreateFromBitmap(UINT idResource)
{
	HBITMAP hBmp = NULL;
//	hBmp = ::LoadBitmap(idResource);
    _ASSERT(0); // Add support - SCP 8/6/97 
    return CreateFromBitmap((HBITMAP)hBmp, NULL, TRUE);
}

BOOL CPictureHolder::CreateFromBitmap(HBITMAP hbm, HPALETTE hpal,
	BOOL bTransferOwnership)
{
	RELEASE(m_pPict);
	PICTDESC pdesc;
	pdesc.cbSizeofstruct = sizeof(pdesc);
	pdesc.picType = PICTYPE_BITMAP;
	pdesc.bmp.hbitmap = hbm;
	pdesc.bmp.hpal = hpal;
	return SUCCEEDED(OleCreatePictureIndirect(&pdesc, IID_IPicture,
		bTransferOwnership, (LPVOID*)&m_pPict));
}

BOOL CPictureHolder::CreateFromMetafile(HMETAFILE hmf, int xExt,
	int yExt, BOOL bTransferOwnership)
{
	RELEASE(m_pPict);
	PICTDESC pdesc;
	pdesc.cbSizeofstruct = sizeof(pdesc);
	pdesc.picType = PICTYPE_METAFILE;
	pdesc.wmf.hmeta = hmf;
	pdesc.wmf.xExt = xExt;
	pdesc.wmf.yExt = yExt;
	return SUCCEEDED(OleCreatePictureIndirect(&pdesc, IID_IPicture,
		bTransferOwnership, (LPVOID*)&m_pPict));
}

BOOL CPictureHolder::CreateFromIcon(UINT idResource)
{
	HICON hIcon = NULL;
//	hIcon = ::LoadIcon(idResource);
    _ASSERT(0); // Add support - SCP 8/6/97 
	return CreateFromIcon(hIcon, TRUE);
}

BOOL CPictureHolder::CreateFromIcon(HICON hicon, BOOL bTransferOwnership)
{
	RELEASE(m_pPict);
	PICTDESC pdesc;
	pdesc.cbSizeofstruct = sizeof(pdesc);
	pdesc.picType = PICTYPE_ICON;
	pdesc.icon.hicon = hicon;
	return SUCCEEDED(OleCreatePictureIndirect(&pdesc, IID_IPicture,
		bTransferOwnership, (LPVOID*)&m_pPict));
}

LPPICTUREDISP CPictureHolder::GetPictureDispatch()
{
	LPPICTUREDISP pPictDisp = NULL;

	if ((m_pPict != NULL) &&
		SUCCEEDED(m_pPict->QueryInterface(IID_IPictureDisp, (LPVOID*)&pPictDisp)))
	{
	}

	return pPictDisp;
}

void CPictureHolder::SetPictureDispatch(LPPICTUREDISP pDisp)
{
	LPPICTURE pPict = NULL;

	if (m_pPict != NULL)
		m_pPict->Release();

	if ((pDisp != NULL) &&
		SUCCEEDED(pDisp->QueryInterface(IID_IPicture, (LPVOID*)&pPict)))
	{
		m_pPict = pPict;
	}
	else
	{
		m_pPict = NULL;
	}
}

void CPictureHolder::Render(HDC hDC, LPCRECT prcRender,
	LPCRECT prcWBounds)
{
	if (m_pPict != NULL)
	{
		long hmWidth;
		long hmHeight;

		m_pPict->get_Width(&hmWidth);
		m_pPict->get_Height(&hmHeight);

		m_pPict->Render(hDC, prcRender->left, prcRender->top,
			prcRender->right - prcRender->left, 
			prcRender->bottom - prcRender->top, 
            0, hmHeight-1, hmWidth, -hmHeight, 
			(LPCRECT)prcWBounds);
	}
}

short CPictureHolder::GetType()
{
	short sPicType = (short)PICTYPE_UNINITIALIZED;

	if (m_pPict != NULL)
	{
		m_pPict->get_Type(&sPicType);
	}

	return sPicType;
}

BOOL CPictureHolder::GetDisplayString(LPSTR strValue)
{
	short sPicType = GetType();

//	UINT idsType = AFX_IDS_PICTYPE_UNKNOWN;

//	if ((sPicType >= PICTYPE_NONE) && (sPicType <= PICTYPE_ICON))
//		idsType = AFX_IDS_PICTYPE_NONE + sPicType;

//	::LoadString(idsType);
//	::LoadString(AFX_IDS_DISPLAYSTRING_PICTURE);

//	TCHAR szValue[_MAX_PATH];
//	wsprintf(szValue, (LPCTSTR)strFormat, (LPCTSTR)strType);

//	strValue = szValue;
	return TRUE;
}

/***************************************************************************
 *
 * Implementation of CfpEnumOleVerb
 *
 **************************************************************************/

/***************************************************************************
 *
 * CLASS:  CfpEnumOleVerb
 *
 * FUNCTION:  CfpEnumOleVerb()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpEnumOleVerb::CfpEnumOleVerb(const void* pvEnum, UINT nSize,
	BOOL bNeedFree)
{
	m_nSizeElem = sizeof(OLEVERB);
	m_pClonedFrom = NULL;

	m_nCurPos = 0;
	m_nSize = nSize;
	m_pvEnum = (BYTE*)pvEnum;
	m_bNeedFree = bNeedFree;

	_ASSERT(this);
}

/***************************************************************************
 *
 * CLASS:  CfpEnumOleVerb
 *
 * FUNCTION:  ~CfpEnumOleVerb()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpEnumOleVerb::~CfpEnumOleVerb()
{
//  LPMALLOC pmalloc;
//  CoGetMalloc(MEMCTX_TASK, &pmalloc);

	_ASSERT(this);

	if (m_pClonedFrom == NULL)
	{
		UINT iVerb;
		LPOLEVERB lpVerb = (LPOLEVERB)(void*)m_pvEnum;
		for (iVerb = 0; iVerb < m_nSize; iVerb++)
			CoTaskMemFree(lpVerb[iVerb].lpszVerbName);
//          pmalloc->Free(lpVerb[iVerb].lpszVerbName);
	}
    else // Free the actual array (if it was not a clone).
    {
	    // Release the clone pointer (only for clones)
		m_pClonedFrom->InternalRelease();
		_ASSERT(!m_bNeedFree);
	}

	// release the pointer (should only happen on non-clones)
	if (m_bNeedFree)
	{
		_ASSERT(m_pClonedFrom == NULL);
		delete m_pvEnum;
	}

//  pmalloc->Release();
}

/***************************************************************************
 *
 * CLASS:  CfpEnumOleVerb
 *
 * FUNCTION:  OnNext()
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
BOOL CfpEnumOleVerb::OnNext(void* pv, HRESULT *retError)
{
	_ASSERT(this);
    *retError = S_OK;

	if (m_nCurPos >= m_nSize)
		return FALSE;

	memcpy(pv, &m_pvEnum[m_nCurPos*m_nSizeElem], m_nSizeElem);
	++m_nCurPos;

	// outgoing OLEVERB requires the verb name to be copied
	//  (the caller has responsibility to free it)

	LPOLEVERB lpVerb = (LPOLEVERB)pv;
	if (lpVerb->lpszVerbName != NULL)
	{
		lpVerb->lpszVerbName = fpAtlCoTaskCopyOleStr(lpVerb->lpszVerbName);
		if (lpVerb->lpszVerbName == NULL)
        {
			*retError = E_OUTOFMEMORY;
            return FALSE;
        }
	}

	// otherwise, copying worked...
	return TRUE;
}



BOOL CfpEnumOleVerb::OnSkip()
{
	_ASSERT(this);

	if (m_nCurPos >= m_nSize)
		return FALSE;

	return ++m_nCurPos < m_nSize;
}

void CfpEnumOleVerb::OnReset()
{
	_ASSERT(this);

	m_nCurPos = 0;
}

CfpEnumOleVerb* CfpEnumOleVerb::OnClone()
{
	_ASSERT(this);

	// set up an exact copy of this object
	//  (derivatives may have to replace this code)
	CfpEnumOleVerb* pClone;
	pClone = new CfpEnumOleVerb(m_pvEnum, m_nSize, FALSE);
	_ASSERT(pClone != NULL);
	_ASSERT(!pClone->m_bNeedFree);   // clones should never free themselves
	pClone->m_nCurPos = m_nCurPos;

	// finally, return the clone to OLE
	_ASSERT(pClone);
	return pClone;
}


STDMETHODIMP CfpEnumOleVerb::Next(
	ULONG celt, LPOLEVERB rgelt, ULONG* pceltFetched)
{
	if (pceltFetched != NULL)
		*pceltFetched = 0;

	_ASSERT(celt > 0);
	_ASSERT(celt == 1 || pceltFetched != NULL);

	BYTE* pchCur = (BYTE*)rgelt;
	ULONG nFetched = 0;

	ULONG celtT = celt;
	SCODE sc;
	while (celtT != 0 && OnNext((void*)pchCur, &sc))
	{
		pchCur += m_nSizeElem;
		--celtT;
	}

    if (sc != S_OK)
       return sc;

	if (pceltFetched != NULL)
		*pceltFetched = celt - celtT;
	sc = celtT == 0 ? S_OK : S_FALSE;
	
	return sc;
}

STDMETHODIMP CfpEnumOleVerb::Skip(ULONG celt)
{
	ULONG celtT = celt;
	SCODE sc = E_UNEXPECTED;

	while (celtT != 0 && OnSkip())
		--celtT;
	sc = celtT == 0 ? S_OK : S_FALSE;

	return celtT != 0 ? S_FALSE : S_OK;
}

STDMETHODIMP CfpEnumOleVerb::Reset()
{
	OnReset();
	return S_OK;
}

STDMETHODIMP CfpEnumOleVerb::Clone(LPENUMOLEVERB *ppenum)
{
	*ppenum = NULL;
	CfpEnumOleVerb* pEnumHelper = OnClone();

	_ASSERT(pEnumHelper);
    if (!pEnumHelper)
        return E_OUTOFMEMORY;

	// we use an extra reference to keep the original object alive
	//  (the extra reference is removed in the clone's destructor)
	if (m_pClonedFrom != NULL)
		pEnumHelper->m_pClonedFrom = m_pClonedFrom;
	else
		pEnumHelper->m_pClonedFrom = this;
	pEnumHelper->m_pClonedFrom->InternalAddRef();
	*ppenum = pEnumHelper;

	return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:    CfpAtlFont()
 *
 * CLASS:       CfpAtlFont
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpAtlFont::CfpAtlFont()
{
  m_pOwner = NULL;
  m_dispID = DISPID_UNKNOWN;
  m_fAdvise = FALSE;
}

/***************************************************************************
 *
 * FUNCTION:    ~CfpAtlFont()
 *
 * CLASS:       CfpAtlFont
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
CfpAtlFont::~CfpAtlFont()
{
  if (CfpAtlComFontPtr::p)
  {
    if (m_fAdvise)
      AtlUnadvise(CfpAtlComFontPtr::p, IID_IPropertyNotifySink, 
        m_dwConnectCookie);
//  CfpAtlComFontPtr::p->Release();
  }
}

/***************************************************************************
 *
 * FUNCTION:    Init()
 *
 * CLASS:       CfpAtlFont
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
void CfpAtlFont::Init(CfpAtlBase *lpCtl, DISPID dispID)
{
  _ASSERT(m_pOwner == NULL);  // We should only set this once!
  m_pOwner = lpCtl;
  m_dispID = dispID;
}

/***************************************************************************
 *
 * FUNCTION:    operator=()
 *
 * CLASS:       CfpAtlFont
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
IFontDisp* CfpAtlFont::operator=(IFontDisp* lp)
{
  // if advise is ON, Unadvise connection
  if (CfpAtlComFontPtr::p && m_fAdvise)
  {
    AtlUnadvise(CfpAtlComFontPtr::p, IID_IPropertyNotifySink, m_dwConnectCookie);
    m_fAdvise = FALSE;
  }

  if (lp)
  {
    if (AtlAdvise(lp, this, IID_IPropertyNotifySink, &m_dwConnectCookie)
         == S_OK)
      m_fAdvise = TRUE;
  }

  return CfpAtlComFontPtr::operator=(lp);
}

/***************************************************************************
 *
 * FUNCTION:    OnChanged()
 *
 * CLASS:       CfpAtlFont
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpAtlFont::OnChanged(DISPID dispID)
{
  m_pOwner->fpAtlOnFontChanged(m_dispID, *this);  // NOTE: ignore dispID parameter
  m_pOwner->fpAtlSetModifiedFlag();
  m_pOwner->fpAtlFireOnChanged(m_dispID);
  return S_OK;
}

/***************************************************************************
 *
 * FUNCTION:    OnRequestEdit()
 *
 * CLASS:       CfpAtlFont
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *
 **************************************************************************/
HRESULT CfpAtlFont::OnRequestEdit(DISPID dispID)
{
  return S_OK;
}
