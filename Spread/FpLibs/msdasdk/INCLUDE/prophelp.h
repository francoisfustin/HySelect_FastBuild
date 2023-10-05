//=--------------------------------------------------------------------------=
// PROPHELP.H:	Definition of property page and property browse helpers
//=--------------------------------------------------------------------------=
// Copyright  1997  Microsoft Corporation.  All Rights Reserved.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//=--------------------------------------------------------------------------=
//
#ifndef _PROPHELP_H_
#define _PROPHELP_H_

// kinda need these
//
#include <PropPage.H>

// Define this to your property page title for Error macros
// Example
//
// #undef IDS_SHOW_ERROR_TITLE
// #define IDS_SHOW_ERROR_TITLE IDS_MY_PROP_PAGE_TITLE
//
#define IDS_SHOW_ERROR_TITLE    0

//
// Error macros for property pages
//

// Set the HRESULT buffer if it not NULL or bad write pointer
//
#define ON_ERROR_SET(phr,hr) \
	if (!(NULL == (phr) || IsBadWritePtr((void *)(phr), sizeof(HRESULT)))) \
		 *((HRESULT *)(phr)) = (hr); \
	else (hr) \

// Displays error and continue
//
#define ERROR_ON_FAILURE(hr) \
    if (FAILED(hr)) { \
        ShowError(hr, IDS_SHOW_ERROR_TITLE); \
        ON_ERROR_SET(lParam, hr); \
    } \

// Executes block if no error, otherwise displays the error
// and executes optional else block
//
#define IF_SUCCEEDED(hr) \
    if (FAILED(hr)) { \
        ShowError(hr, IDS_SHOW_ERROR_TITLE); \
        ON_ERROR_SET(lParam, hr); \
    } else if (SUCCEEDED(hr)) \

// Displays error and return a value
//
#define ON_ERROR_RETURN(hr,ret) \
    if (FAILED(hr)) { \
        ShowError(hr, IDS_SHOW_ERROR_TITLE); \
        ON_ERROR_SET(lParam, hr); \
        return (##ret); \
    } \

// Displays error and goto a label
//
#define ON_ERROR_GOTO(hr,label) \
    if (FAILED(hr)) { \
        ShowError(hr, IDS_SHOW_ERROR_TITLE); \
        ON_ERROR_SET(lParam, hr); \
        goto label; \
    } \

// Displays error, sets focus to edit control and goto label
//
#define ON_ERROR_EDIT_GOTO(hr,id,label) \
    if (FAILED(hr)) { \
        ShowError(hr, IDS_SHOW_ERROR_TITLE); \
        SetFocusToEdit(m_hwnd, IDS_SHOW_ERROR_TITLE, id); \
        ON_ERROR_SET(lParam, hr); \
        goto label; \
    } \

// Displays error, sets focus to edit control and return a value
//
#define ON_ERROR_EDIT_RETURN(hr,id,ret) \
    if (FAILED(hr)) { \
        ShowError(hr, IDS_SHOW_ERROR_TITLE); \
        SetFocusToEdit(m_hwnd, IDS_SHOW_ERROR_TITLE, id); \
        ON_ERROR_SET(lParam, hr); \
        return (##ret); \
    } \

// Display invalid entry message if FALSE and return value
//
#define ON_INVALID_ENTRY_RETURN(f,id,ret) \
    if (!(f)) { \
        SetFocusToEdit(m_hwnd, IDS_SHOW_ERROR_TITLE, id, TRUE); \
        ON_ERROR_SET(lParam, E_ABORT); \
        return (##ret); \
    } \

// Display invalid entry message if FALSE and goto to label
//
#define ON_INVALID_ENTRY_GOTO(f,id,label) \
    if (!(f)) { \
        SetFocusToEdit(m_hwnd, IDS_SHOW_ERROR_TITLE, id, TRUE); \
        ON_ERROR_SET(lParam, E_ABORT); \
        goto label; \
    } \

// Error handler helpers
//
void ShowError(HRESULT hrInput, WORD idPropPageTitle);
void GetControlTag(HWND hwnd, UINT uCtlID, char *szBuf, int cbBuf);
void SetFocusToEdit(HWND hwnd, UINT uIDSTitle, UINT uCtlID, BOOL fError = FALSE);

// Relationship of resource ids and typelib guids used to fill in 
// lists and combos in the Property Page
// 
struct COMBOTOGUID 
{
    WORD        id;
    const GUID *pGuid;

};

HRESULT FillAllComboBoxes(HWND hwnd, int iObjType, const COMBOTOGUID *pComboToGuid);
HRESULT FillComboBox(HWND hwnd, ITypeInfo *pTypeInfo);

// IPerPropertyBrowse helpers
//
HRESULT GetStrings(CALPOLESTR *prgOleStr, CADWORD *prgCookie, int iObjectType, REFIID riid, int ctItems = 0);
HRESULT GetValue(VARIANT *pVarOut, DWORD dwCookie);

/////////////////////////////////////////////////////////////////////////////
// CCtlPropertyPage
//
class CCtlPropertyPage : public CPropertyPage, 
						public IPropertyNotifySink
{
  public:
    // constructor and destructor
    //
    CCtlPropertyPage(IUnknown *pUnkOuter, int nObjIndex);
    virtual ~CCtlPropertyPage();

    // IUnknown
    //
    DECLARE_STANDARD_UNKNOWN();

	// IPropertyNofitySink
	//
	STDMETHOD(OnChanged)(DISPID dispid);
	STDMETHOD(OnRequestEdit)(DISPID dispid);

  // Implementation
  protected:
    DWORD       m_dwNotifyCookie;
    IUnknown   *m_pUnkAdvise;
    BOOL        m_fInApply;
	BOOL		m_fOnChanged;
	
	void SetDirtyFlag(BOOL fDirty = TRUE);
	BOOL IsDirty() {return S_OK == IsPageDirty();}
	void EnableModeless(BOOL fEnable = TRUE);

    HRESULT Advise(IUnknown *pUnkAdvise);
    virtual BOOL DialogProc(HWND, UINT, WPARAM, LPARAM);

  // Overridables
  protected:
    virtual HRESULT InternalQueryInterface(REFIID, void **);
    virtual BOOL PageDialogProc(HWND, UINT, WPARAM, LPARAM) PURE;
    virtual HRESULT OnPropertyChanged(IUnknown *pUnkSrc, DISPID dispid) {return S_OK;}
    virtual HRESULT OnPropertyRequestEdit(IUnknown *pUnkSrc, DISPID dispid) {return S_OK;}
};

#endif

