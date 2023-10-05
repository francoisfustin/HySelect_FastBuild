#ifndef NO_MFC
#include <afxctl.h>                 // MFC support for OLE Custom Controls
#endif
#include <afxpriv.h>
#include "fpprpbag.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define ASSERT_BUFFER_VALID(p, cb, bWrite) \
	ASSERT(AfxIsValidAddress(p, cb, bWrite))
#else
#define ASSERT_BUFFER_VALID(p, cb, bWrite)
#endif

// helper for reliable and small Release calls
//DWORD AFXAPI _AfxRelease(LPUNKNOWN* plpUnknown);
//#ifndef _DEBUG
// generate smaller code in release build
//#define RELEASE(lpUnk) _AfxRelease((LPUNKNOWN*)&lpUnk)
//#else
// generate larger but typesafe code in debug build
#define RELEASE(lpUnk) do \
	{ if ((lpUnk) != NULL) { (lpUnk)->Release(); (lpUnk) = NULL; } } while (0)
//#endif


static const FONTDESC _fdDefault =
	{ sizeof(FONTDESC), OLESTR("MS Sans Serif"), FONTSIZE(12), FW_NORMAL,
	  DEFAULT_CHARSET, FALSE, FALSE, FALSE };


static size_t PASCAL _AfxGetSizeOfVarType(VARTYPE vt)
{
	switch (vt)
	{
	case VT_I2:
	case VT_BOOL:
		return 2;

	case VT_I4:
	case VT_R4:
		return 4;

	case VT_R8:
		return 8;

	case VT_CY:
		return sizeof(CURRENCY);

	case VT_BSTR:
		return sizeof(BSTR);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// _AfxInitBlob

BOOL AFXAPI _AfxInitBlob(HGLOBAL* phDst, void* pvSrc)
{
	BOOL bResult = FALSE;
	ULONG cb;
	if ((cb = *(long*)pvSrc) > 0)
	{
		ASSERT_BUFFER_VALID(pvSrc, sizeof(cb) + cb, TRUE);
		*phDst = GlobalAlloc(GMEM_MOVEABLE, sizeof(cb) + cb);
		if (*phDst != NULL)
		{
			void* pvDst = GlobalLock(*phDst);
			ASSERT(pvDst != NULL);
			memcpy(pvDst, pvSrc, sizeof(cb) + cb);
			bResult = TRUE;
			GlobalUnlock(*phDst);
		}
	}
	return bResult;
}

BOOL AFXAPI _AfxIsSameFont(CFontHolder& font, const FONTDESC* pFontDesc,
	LPFONTDISP pFontDispAmbient)
{
	if (font.m_pFont == NULL)
		return FALSE;

	BOOL bSame = FALSE;

	if (pFontDispAmbient != NULL)
	{
		LPFONT pFontAmbient;
		if (SUCCEEDED(pFontDispAmbient->QueryInterface(IID_IFont,
			(LPVOID*)&pFontAmbient)))
		{
			ASSERT_POINTER(pFontAmbient, IFont);
			bSame = pFontAmbient->IsEqual(font.m_pFont) == S_OK;
			pFontAmbient->Release();
		}
	}
	else
	{
		if (pFontDesc == NULL)
			pFontDesc = &_fdDefault;

		bSame = TRUE;
		BOOL bFlag;

		font.m_pFont->get_Italic(&bFlag);
		bSame = (bFlag == pFontDesc->fItalic);

		if (bSame)
		{
			font.m_pFont->get_Underline(&bFlag);
			bSame = (bFlag == pFontDesc->fUnderline);
		}

		if (bSame)
		{
			font.m_pFont->get_Strikethrough(&bFlag);
			bSame = (bFlag == pFontDesc->fStrikethrough);
		}

		if (bSame)
		{
			short sCharset;
			font.m_pFont->get_Charset(&sCharset);
			bSame = (sCharset == pFontDesc->sCharset);
		}

		if (bSame)
		{
			short sWeight;
			font.m_pFont->get_Weight(&sWeight);
			bSame = (sWeight == pFontDesc->sWeight);
		}

		if (bSame)
		{
			CURRENCY cy;
			font.m_pFont->get_Size(&cy);
			bSame = (memcmp(&cy, &pFontDesc->cySize, sizeof(CURRENCY)) == 0);
		}

		if (bSame)
		{
			BSTR bstrName;
			font.m_pFont->get_Name(&bstrName);
			CString strName1(bstrName);
			CString strName2(pFontDesc->lpstrName);
			bSame = (strName1 == strName2);
			SysFreeString(bstrName);
		}
	}

	return bSame;
}

BOOL AFXAPI _AfxIsSamePropValue(VARTYPE vtProp, const void* pv1, const void* pv2)
{
	if (pv1 == pv2)
		return TRUE;

	if ((pv1 == NULL) || (pv2 == NULL))
		return FALSE;

	BOOL bSame = FALSE;

	switch (vtProp)
	{
	case VT_BSTR:
		bSame = ((CString*)pv1)->Compare(*(CString*)pv2) == 0;
		break;

	case VT_BOOL:
	case VT_I2:
	case VT_I4:
	case VT_CY:
	case VT_R4:
	case VT_R8:
		bSame = memcmp(pv1, pv2, _AfxGetSizeOfVarType(vtProp)) == 0;
		break;
	}

	return bSame;
}

/////////////////////////////////////////////////////////////////////////////
// _AfxCopyPropValue

BOOL AFXAPI _AfxCopyPropValue(VARTYPE vtProp, void* pvDest, const void * pvSrc)
{
	ASSERT(AfxIsValidAddress(pvDest, 1));

	if (pvSrc != NULL)
	{
		ASSERT(AfxIsValidAddress(pvSrc, 1, FALSE));

		switch (vtProp)
		{
		case VT_I2:
			*(short*)pvDest = *(short*)pvSrc;
			break;
		case VT_I4:
			*(long*)pvDest = *(long*)pvSrc;
			break;
		case VT_BOOL:
			*(BOOL*)pvDest = *(BOOL*)pvSrc;
			break;
		case VT_BSTR:
			*(CString*)pvDest = *(CString*)pvSrc;
			break;
		case VT_CY:
			*(CY*)pvDest = *(CY*)pvSrc;
			break;
		case VT_R4:
			*(_AFX_FLOAT*)pvDest = *(_AFX_FLOAT*)pvSrc;
			break;
		case VT_R8:
			*(_AFX_DOUBLE*)pvDest = *(_AFX_DOUBLE*)pvSrc;
			break;
		default:
			return FALSE;
		}
	}
	return pvSrc != NULL;
}

/////////////////////////////////////////////////////////////////////////////
// _AfxCopyBlob

BOOL AFXAPI _AfxCopyBlob(HGLOBAL* phDst, HGLOBAL hSrc)
{
	BOOL bResult = FALSE;
	void* pvSrc = GlobalLock(hSrc);
	if (pvSrc != NULL)
	{
		bResult = _AfxInitBlob(phDst, pvSrc);
		GlobalUnlock(hSrc);
	}
	return bResult;
}

BOOL AFXAPI _AfxIsSameUnknownObject(REFIID iid, LPUNKNOWN pUnk1, LPUNKNOWN pUnk2)
{
	if (pUnk1 == pUnk2)
		return TRUE;

	if (pUnk1 == NULL || pUnk2 == NULL)
		return FALSE;

	LPUNKNOWN pI1 = NULL;
	LPUNKNOWN pI2 = NULL;
	BOOL bResult = FALSE;
	if (SUCCEEDED(pUnk1->QueryInterface(iid, (void**)&pI1)))
	{
		ASSERT_POINTER(pI1, IUnknown);
		if (SUCCEEDED(pUnk2->QueryInterface(iid, (void**)&pI2)))
		{
			ASSERT_POINTER(pI2, IUnknown);
			bResult = (pI1 == pI2);
			pI2->Release();
		}
		pI1->Release();
	}
	return bResult;
}

CBlobProperty::CBlobProperty(HGLOBAL hBlob) :
	m_hBlob(hBlob),
	m_dwRef(1)
{
	m_bPropSet = FALSE;
}

HGLOBAL CBlobProperty::GetBlob()
{
	return m_hBlob;
}

void CBlobProperty::SetPropSetFlag(BOOL bFlag)
{
	m_bPropSet = bFlag;
}

STDMETHODIMP_(ULONG) CBlobProperty::AddRef()
{
	return InterlockedIncrement(&m_dwRef);
}

STDMETHODIMP_(ULONG) CBlobProperty::Release()
{
	if (InterlockedDecrement(&m_dwRef) > 0)
		return m_dwRef;

	delete this;
	return 0;
}

STDMETHODIMP CBlobProperty::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
	if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_IPersist) ||
		IsEqualIID(riid, IID_IPersistStream))
	{
		AddRef();
		*ppvObj = this;
		return S_OK;
	}

	*ppvObj = NULL;
	return E_NOINTERFACE;
}

static const CLSID _clsidBlobProperty =
{ 0xf6f07540, 0x42ec, 0x11ce, { 0x81, 0x35, 0x0, 0xaa, 0x0, 0x4b, 0xb8, 0x51 } };

STDMETHODIMP CBlobProperty::GetClassID(LPCLSID pClsid)
{
	*pClsid = _clsidBlobProperty;
	return S_OK;
}

STDMETHODIMP CBlobProperty::IsDirty()
{
	return S_OK;
}

STDMETHODIMP CBlobProperty::Load(LPSTREAM pStream)
{
	ULONG cb;
	ULONG cbRead;
	HRESULT hr;
	LARGE_INTEGER   li;
	ULARGE_INTEGER  uli;

	if (m_bPropSet)
	{

  	   LISet32(li, -(LONG)(sizeof(CLSID) + sizeof(ULONG)));

       hr = pStream->Seek(li, STREAM_SEEK_CUR, &uli);
	
	   hr = pStream->Read(&cb, sizeof(ULONG), &cbRead);

	}
	else
	{
	   CLSID Clsid, Clsid2;
  	   LISet32(li, -(LONG)(sizeof(CLSID)));
       hr = pStream->Seek(li, STREAM_SEEK_CUR, &uli);
       GetClassID(&Clsid);
   	   hr = pStream->Read(&Clsid2, sizeof(CLSID), &cbRead);
	   if (IsEqualCLSID(Clsid, Clsid2))
		  hr = pStream->Read(&cb, sizeof(ULONG), &cbRead);
   	   else
		  return E_FAIL;
	}
	
	if (FAILED(hr))
		return hr;

	if (sizeof(ULONG) != cbRead)
		return E_FAIL;

	HGLOBAL hBlobNew = GlobalAlloc(GMEM_MOVEABLE, sizeof(ULONG)+cb);
	if (hBlobNew == NULL)
		return E_OUTOFMEMORY;

	void* pBlobNew = GlobalLock(hBlobNew);
	*(ULONG*)pBlobNew = cb;
	hr = pStream->Read(((ULONG*)pBlobNew)+1, cb, &cbRead);
	GlobalUnlock(hBlobNew);

	if (FAILED(hr))
	{
		GlobalFree(hBlobNew);
		return hr;
	}
	if (cb != cbRead)
	{
		GlobalFree(hBlobNew);
		return E_FAIL;
	}

	if (m_hBlob) // don't free blob unless it exists -scl
		GlobalFree(m_hBlob);
	m_hBlob = hBlobNew;
	return S_OK;
}

STDMETHODIMP CBlobProperty::Save(LPSTREAM pStream, BOOL)
{
	void* pBlob = GlobalLock(m_hBlob);
	if (pBlob == NULL)
		return E_OUTOFMEMORY;

	ULONG cb = sizeof(ULONG) + *(ULONG*)pBlob;
	ULONG cbWritten;
	HRESULT hr = pStream->Write(pBlob, cb, &cbWritten);

	GlobalUnlock(m_hBlob);

	if (FAILED(hr))
		return hr;

	if (cb != cbWritten)
		return E_FAIL;

	return S_OK;
}

STDMETHODIMP CBlobProperty::GetSizeMax(ULARGE_INTEGER* pcbSize)
{
	void* pBlob = GlobalLock(m_hBlob);
	if (pBlob == NULL)
		return E_OUTOFMEMORY;

	pcbSize->HighPart = 0;
	pcbSize->LowPart = sizeof(ULONG) + *(ULONG*)pBlob;

	GlobalUnlock(m_hBlob);
	return S_OK;
}

CfpPropbagPropExchange::CfpPropbagPropExchange(LPPROPERTYBAG pPropBag,
	LPERRORLOG pErrorLog, BOOL bLoading, BOOL bSaveAllProperties) :
	m_pPropBag(pPropBag),
	m_pErrorLog(pErrorLog),
	m_bSaveAllProperties(bSaveAllProperties)
{
	m_bLoading = bLoading;
	if (pPropBag != NULL)
		pPropBag->AddRef();
	if (pErrorLog != NULL)
		pErrorLog->AddRef();
}

CfpPropbagPropExchange::~CfpPropbagPropExchange()
{
	RELEASE(m_pPropBag);
	RELEASE(m_pErrorLog);
}

BOOL CfpPropbagPropExchange::ExchangeProp(LPCTSTR pszPropName, VARTYPE vtProp,
	void* pvProp, const void* pvDefault)
{
	USES_CONVERSION;

	ASSERT_POINTER(m_pPropBag, IPropertyBag);
	ASSERT(AfxIsValidString(pszPropName));
	ASSERT(AfxIsValidAddress(pvProp, 1, FALSE));
	ASSERT((pvDefault == NULL) || AfxIsValidAddress(pvDefault, 1, FALSE));

	if (m_pPropBag == NULL)
		return FALSE;

	BOOL bSuccess = TRUE;
	VARIANTARG var;
	AfxVariantInit(&var);
	V_VT(&var) = vtProp;

	if (vtProp == VT_LPSTR)
       V_VT(&var) = VT_BSTR;

	if (m_bLoading)
	{
		if (FAILED(m_pPropBag->Read(T2COLE(pszPropName), &var, m_pErrorLog)))
			return _AfxCopyPropValue(vtProp, pvProp, pvDefault);

		switch (vtProp)
		{
		case VT_UI1:
			*(BYTE*)pvProp = V_UI1(&var);
			break;

		case VT_I2:
			*(short*)pvProp = V_I2(&var);
			break;

		case VT_I4:
			*(long*)pvProp = V_I4(&var);
			break;

		case VT_BOOL:
			*(BOOL*)pvProp = (BOOL)V_BOOL(&var);
			break;

		case VT_LPSTR:
		case VT_BSTR:
			*(CString*)pvProp = OLE2CT(V_BSTR(&var));
			break;

		case VT_CY:
			*(CY*)pvProp = V_CY(&var);
			break;

		case VT_R4:
			memcpy(pvProp, &V_R4(&var), sizeof(float));
			break;

		case VT_R8:
			memcpy(pvProp, &V_R8(&var), sizeof(double));
			break;

		default:
			bSuccess = FALSE;
		}
	}
	else
	{
		if (m_bSaveAllProperties ||
			!_AfxIsSamePropValue(vtProp, pvProp, pvDefault))
		{
			switch (vtProp)
			{
			case VT_UI1:
				V_UI1(&var) = *(BYTE*)pvProp;
				break;

			case VT_I2:
				V_I2(&var) = *(short*)pvProp;
				break;

			case VT_I4:
				V_I4(&var) = *(long*)pvProp;
				break;

			case VT_BOOL:
				V_BOOL(&var) = (VARIANT_BOOL)*(BOOL*)pvProp;
				break;

		    case VT_LPSTR:
			case VT_BSTR:
				V_BSTR(&var) = SysAllocString(T2COLE(*(CString*)pvProp));
				break;

			case VT_CY:
				V_CY(&var) = *(CY*)pvProp;
				break;

			case VT_R4:
				memcpy(&V_R4(&var), pvProp, sizeof(float));
				break;

			case VT_R8:
				memcpy(&V_R8(&var), pvProp, sizeof(double));
				break;

			default:
				return FALSE;
			}
			bSuccess = SUCCEEDED(m_pPropBag->Write(T2COLE(pszPropName), &var));
            ASSERT(bSuccess != 0);
		}
	}

	VariantClear(&var);
	return bSuccess;
}

BOOL CfpPropbagPropExchange::ExchangeBlobProp(LPCTSTR pszPropName,
	HGLOBAL* phBlob, HGLOBAL hBlobDefault)
{
	USES_CONVERSION;

	ASSERT_POINTER(m_pPropBag, IPropertyBag);
	ASSERT(AfxIsValidString(pszPropName));
	ASSERT_POINTER(phBlob, HGLOBAL);

	BOOL bSuccess = FALSE;
	VARIANT var;
	AfxVariantInit(&var);
	V_VT(&var) = VT_UNKNOWN;

	if (m_bLoading)
	{
		if (*phBlob != NULL)
		{
			GlobalFree(*phBlob);
			*phBlob = NULL;
		}

		CBlobProperty* pBlobProp = new CBlobProperty;
		V_UNKNOWN(&var) = pBlobProp;

		if (SUCCEEDED(m_pPropBag->Read(T2COLE(pszPropName), &var, m_pErrorLog)))
		{
			*phBlob = pBlobProp->GetBlob();
			bSuccess = TRUE;
		}
		else
		{
            pBlobProp->SetPropSetFlag(TRUE);
		    
			if (SUCCEEDED(m_pPropBag->Read(T2COLE(pszPropName), &var, m_pErrorLog)))
     		{
	           *phBlob = pBlobProp->GetBlob();
			   bSuccess = TRUE;
		    }
            else
			{
			   if (hBlobDefault != NULL)
			      bSuccess = _AfxCopyBlob(phBlob, hBlobDefault);
			}
			
            pBlobProp->SetPropSetFlag(FALSE);
  
		}

		pBlobProp->Release();
	}
	else
	{
		CBlobProperty* pBlobProp = new CBlobProperty(*phBlob);
		V_UNKNOWN(&var) = pBlobProp;
		bSuccess = SUCCEEDED(m_pPropBag->Write(T2COLE(pszPropName), &var));
		pBlobProp->Release();
	}
	return bSuccess;
}

BOOL CfpPropbagPropExchange::ExchangeFontProp(LPCTSTR pszPropName,
	CFontHolder& font, const FONTDESC* pFontDesc,
	LPFONTDISP pFontDispAmbient)
{
	USES_CONVERSION;

	ASSERT_POINTER(m_pPropBag, IPropertyBag);
	ASSERT(AfxIsValidString(pszPropName));
	ASSERT_POINTER(&font, CFontHolder);
	ASSERT_NULL_OR_POINTER(pFontDesc, FONTDESC);
	ASSERT_NULL_OR_POINTER(pFontDispAmbient, IFontDisp);

	BOOL bSuccess = FALSE;
	VARIANT var;
	AfxVariantInit(&var);
	V_VT(&var) = VT_UNKNOWN;

	if (m_bLoading)
	{
		LPFONT pFont = NULL;

		bSuccess =
			SUCCEEDED(m_pPropBag->Read(T2COLE(pszPropName), &var,
				m_pErrorLog)) &&
			SUCCEEDED(V_UNKNOWN(&var)->QueryInterface(IID_IFont,
				(LPVOID*)&pFont));

		if (bSuccess)
		{
			ASSERT_POINTER(pFont, IFont);
			font.SetFont(pFont);
		}
		else
		{
			// Initialize font to its default state
			font.InitializeFont(pFontDesc, pFontDispAmbient);
		}
		VariantClear(&var);
	}
	else
	{
		if ((font.m_pFont == NULL) ||
			(_AfxIsSameFont(font, pFontDesc, pFontDispAmbient) &&
				!m_bSaveAllProperties))
		{
			bSuccess = TRUE;
		}
		else
		{
			V_UNKNOWN(&var) = font.m_pFont;
			bSuccess = SUCCEEDED(m_pPropBag->Write(T2COLE(pszPropName), &var));
		}
	}

	return bSuccess;
}

BOOL CfpPropbagPropExchange::ExchangePersistentProp(LPCTSTR pszPropName,
	LPUNKNOWN* ppUnk, REFIID iid, LPUNKNOWN pUnkDefault)
{
	USES_CONVERSION;

	ASSERT_POINTER(m_pPropBag, IPropertyBag);
	ASSERT(AfxIsValidString(pszPropName));
	ASSERT_POINTER(ppUnk, LPUNKNOWN);
	ASSERT_NULL_OR_POINTER(pUnkDefault, IUnknown);

	BOOL bSuccess = FALSE;
	VARIANT var;
	AfxVariantInit(&var);
	V_VT(&var) = VT_UNKNOWN;

	if (m_bLoading)
	{
		RELEASE(*ppUnk);
		*ppUnk = NULL;

		bSuccess =
			SUCCEEDED(m_pPropBag->Read(T2COLE(pszPropName), &var,
				m_pErrorLog)) &&
			SUCCEEDED(V_UNKNOWN(&var)->QueryInterface(iid, (LPVOID*)ppUnk));

		if (!bSuccess)
		{
			// Use default value.
			if (pUnkDefault != NULL)
			{
				bSuccess = SUCCEEDED(pUnkDefault->QueryInterface(iid,
					(LPVOID*)ppUnk));
			}
			else
			{
				bSuccess = TRUE;
			}
		}
		VariantClear(&var);
	}
	else
	{
		if ((*ppUnk == NULL) ||
			(_AfxIsSameUnknownObject(iid, *ppUnk, pUnkDefault) &&
				!m_bSaveAllProperties))
		{
			bSuccess = TRUE;
		}
		else
		{
			V_UNKNOWN(&var) = *ppUnk;
			bSuccess = SUCCEEDED(m_pPropBag->Write(T2COLE(pszPropName), &var));
		}
	}

	return bSuccess;
}
