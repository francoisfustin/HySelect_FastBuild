#ifndef FPMYDISP_CTRL_NAME
#pragma message("*** " __FILE__ " : You MUST define \"FPMYDISP_CTRL_NAME\" ***" ) 
#endif

// MACROS:
#ifndef METHOD_PROLOGUE_EX_
#define METHOD_PROLOGUE_EX_(theClass, localClass)  \
METHOD_PROLOGUE(theClass, localClass)
#endif

/////////////////////////////////////////////
#ifdef WIN32
// Don't try understanding this macro, just accept it! - SCP
#define DEFAULT_XPROPBAG_PTR(pCtl)   \
  ((IPersistPropertyBag FAR *)&(pCtl->m_xPersistPropertyBag))

// DELETE_EXCEPTION() copied from \msvc40\mfc\src\stdafx.h
// 09.18.97 - removed due to warning C4005 (macro redefinition) -scl
// 11/05/97 - TabPro generates an error without this macro    - CTF
#define DELETE_EXCEPTION(e)   do { e->Delete(); } while (0)

#else //WIN16
#define DEFAULT_XPROPBAG_PTR(pCtl)   \
  ((IPersistPropertyBag FAR *)&(pCtl->m_xPersistPropertyBag))

#define DELETE_EXCEPTION(e)
#endif
///////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// FPMYDISP_CTRL_NAME::XfpPersistPropertyBag

STDMETHODIMP_(ULONG) FPMYDISP_CTRL_NAME::XfpPersistPropertyBag::AddRef()
{
	METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, fpPersistPropertyBag)
	return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP_(ULONG) FPMYDISP_CTRL_NAME::XfpPersistPropertyBag::Release()
{
	METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, fpPersistPropertyBag)
	return (ULONG)pThis->ExternalRelease();
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpPersistPropertyBag::QueryInterface(
	REFIID iid, LPVOID* ppvObj)
{
	METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, fpPersistPropertyBag)
	return (HRESULT)pThis->ExternalQueryInterface(&iid, ppvObj);
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpPersistPropertyBag::GetClassID(LPCLSID lpClassID)
{
  METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, fpPersistPropertyBag)

  if (DEFAULT_XPROPBAG_PTR(pThis))
    return DEFAULT_XPROPBAG_PTR(pThis)->GetClassID(lpClassID);
  else
    return (HRESULT)E_NOTIMPL;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpPersistPropertyBag::InitNew()
{
  METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, fpPersistPropertyBag)

  if (DEFAULT_XPROPBAG_PTR(pThis))
    return DEFAULT_XPROPBAG_PTR(pThis)->InitNew();
  else
    return (HRESULT)E_NOTIMPL;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpPersistPropertyBag::Load(LPPROPERTYBAG pPropBag,
	LPERRORLOG pErrorLog)
{
	METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, fpPersistPropertyBag)

	HRESULT hr;

   	AFX_MANAGE_STATE(pThis->m_pModuleState);

	TRY
	{
		CfpPropbagPropExchange px(pPropBag, pErrorLog, TRUE);
		pThis->DoPropExchange(&px);
		hr = S_OK;
	}
	CATCH_ALL(e)
	{
		hr = E_FAIL;
		e->Delete();
	}
	END_CATCH_ALL

	// Clear the modified flag.
	pThis->m_bModified = FALSE;

	// Properties have been initialized
	pThis->m_bInitialized = TRUE;

	return hr;
}

STDMETHODIMP FPMYDISP_CTRL_NAME::XfpPersistPropertyBag::Save(LPPROPERTYBAG pPropBag,
	BOOL fClearDirty, BOOL fSaveAllProperties)
{
	METHOD_PROLOGUE_EX_(FPMYDISP_CTRL_NAME, fpPersistPropertyBag)

	HRESULT hr;

	TRY
	{
		CfpPropbagPropExchange px(pPropBag, NULL, FALSE, fSaveAllProperties);
		pThis->DoPropExchange(&px);
		hr = S_OK;
	}
	CATCH_ALL(e)
	{
		hr = E_FAIL;
		DELETE_EXCEPTION(e);
	}
	END_CATCH_ALL

	// Bookkeeping:  Clear the dirty flag, if requested.
	if (fClearDirty)
		pThis->m_bModified = FALSE;

	return hr;
}

