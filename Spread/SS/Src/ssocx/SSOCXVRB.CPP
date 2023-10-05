// ssocxvrb.cpp : Implementation of the CBtnCtrl OLE control class.

// *** NOTE: IGNORE FILE IF WIN32 ***
#ifndef WIN32

#ifndef _DEBUG
#pragma warning(disable:4702)   // inline expansion finds unreachable code
#endif

// Set data macros.  MFC data macros set to their internal values, since
// we will be linking them into the DLL with the controls code.

#ifdef _WIN32
  #define AFX_CORE_DATA   AFX_DATA_EXPORT
  #define AFX_CORE_DATADEF
  #define AFX_OLE_DATA    AFX_DATA_EXPORT
  #define AFX_OLE_DATADEF

  #define AFX_CTL_DATA    AFX_DATA_EXPORT
  #define AFX_CTL_DATAMOD
  #define AFX_CTL_DATADEF
#else
  #define AFXAPI_DATA     __based(__segname("_DATA"))
  #define AFXAPIEX_DATA   __based(__segname("_DATA"))

  #define AFX_CTL_DATA    __based(__segname("_DATA"))
  #define AFX_CTL_DATAMOD __based(__segname("_DATA"))
  #define AFX_CTL_DATADEF
#endif

#define CURSORFILES
#define _AFX_NOFORCE_LIBS

//#include <afxctl.h>         // MFC support for OLE Custom Controls
#include "stdafx.h"

//#include <windowsx.h>

#include <afxdlgs.h>
#include <afxext.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef _WIN32
#include "oleimpl.h"
#include "ctlimpl.h"
#else
//#include "\win32app\msvc15\mfc\src\oleimpl.h"
//#include "\win32app\msvc15\cdk16\src\ctlimpl.h"
#include "\msvc\mfc\src\oleimpl.h"
#include "\msvc\cdk16\src\ctlimpl.h"
#endif

#include <afxpriv.h>
#if defined(_WIN32)
#undef _istleadbyte
#include <afximpl.h>
#include <afxdisp.h>
#else
//#include "\win32app\msvc15\mfc\src\auxdata.h"
#include "\msvc\mfc\src\auxdata.h"
#endif

#ifndef RELEASE
#define RELEASE(p)  if ((p) != NULL) { (p)->Release(); (p) = NULL; };
#endif

#ifndef ASSERT_POINTER
#define ASSERT_POINTER(p, itf) \
  ASSERT(((p) != NULL) && AfxIsValidAddress((p), sizeof(itf), FALSE))
#endif

#ifndef ASSERT_NULL_OR_POINTER
#define ASSERT_NULL_OR_POINTER(p, itf) \
  ASSERT(((p) == NULL) || AfxIsValidAddress((p), sizeof(itf), FALSE))
#endif

/****************************************
* Change this define to your class name
* and include any headers that you need
****************************************/

#define CCtrl  CSsocxCtrl

#include "ssocxctl.h"

/***************************************/

#undef AFX_DATADEF
#define AFX_DATADEF AFX_CTL_DATADEF
#undef AFXAPP_DATA
#define AFXAPP_DATA AFX_CTL_DATAMOD


BOOL CCtrl::OnVerbUiActivate(LPMSG lpMsg, HWND, LPCRECT lprcPosRect)
{
if (m_bOpen)
   ResultFromScode(OLE_E_NOT_INPLACEACTIVE);    // Already open
else
   {
   CopyRect(&m_rcPos, lprcPosRect);
   MyOnActivateInPlace(TRUE, lpMsg);
   }

return (TRUE);
}


BOOL CCtrl::OnVerbInPlaceActivate(LPMSG lpMsg, HWND, LPCRECT lprcPosRect)
{
if (m_bOpen)
   ResultFromScode(OLE_E_NOT_INPLACEACTIVE);    // Already open
else
   {
   CopyRect(&m_rcPos, lprcPosRect);
   MyOnActivateInPlace(FALSE, lpMsg);
   }

return (TRUE);
}


BOOL CCtrl::OnVerbPrimary(LPMSG lpMsg, HWND, LPCRECT lprcPosRect)
{
CopyRect(&m_rcPos, lprcPosRect);
MyOnActivateInPlace(TRUE, lpMsg);

return (TRUE);
}


HRESULT CCtrl::MyOnActivateInPlace(BOOL bUIActivate, LPMSG pMsg)
{
  if (m_bOpen)
  {
    m_pWndOpenFrame->SetActiveWindow();
    SendAdvise(OBJECTCODE_SHOWWINDOW);
    return NOERROR;
  }

  if (m_bOpen)
    return NOERROR;

  if ((m_bInPlaceActive && !bUIActivate) || m_bUIActive)
  {
    CWnd* pWndOuter = GetOuterWindow();
    HWND hwndParent;
    if ((pWndOuter != NULL) && 
      SUCCEEDED(m_pInPlaceSite->GetWindow(&hwndParent)) &&
      (hwndParent == ::GetParent(pWndOuter->m_hWnd)))
    {
      return NOERROR;
    }
  }

  HRESULT hr = ResultFromScode(E_FAIL);
  if (m_pInPlaceSite != NULL)
    hr = m_pInPlaceSite->CanInPlaceActivate();

  if (FAILED(hr) || (GetScode(hr) == S_FALSE))
  {
    //  Site doesn't allow in-place activation.
    return OnOpen(FALSE, pMsg);
  }

  if (! m_bInPlaceActive)
    m_pInPlaceSite->OnInPlaceActivate();

  HWND hwndParent = NULL;

  if (SUCCEEDED(m_pInPlaceSite->GetWindow(&hwndParent)))
  {
    RECT rcClip;
    m_frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);

    RELEASE(m_pInPlaceFrame);
    RELEASE(m_pInPlaceDoc);

    if (SUCCEEDED(m_pInPlaceSite->GetWindowContext(
            &m_pInPlaceFrame, &m_pInPlaceDoc,
            &m_rcPos, &rcClip, &m_frameInfo)))
    {
      ASSERT(m_pInPlaceFrame != NULL);

            if (MyCreateControlWindow(hwndParent, m_rcPos))
      {
        m_bInPlaceActive = TRUE;

        if (bUIActivate)
        {
          BuildSharedMenu();

          m_pInPlaceSite->OnUIActivate();
          m_bUIActive = TRUE;

          m_pInPlaceFrame->SetActiveObject(
            &m_xOleInPlaceActiveObject, NULL);

          if (m_pInPlaceDoc != NULL)
            m_pInPlaceDoc->SetActiveObject(
              &m_xOleInPlaceActiveObject, NULL);

          BOOL bHandles = AmbientShowGrabHandles();
          BOOL bHatching = AmbientShowHatching();

#ifndef SPREAD_JPN
          if (bHandles || bHatching)
            CreateTracker(bHandles, bHatching);
#endif

          AddFrameLevelUI();

          if (m_hWnd != NULL)
            SetFocus();

        }

        // Pass thru the window message that caused us to be activated

        if ((m_hWnd != NULL) && (pMsg != NULL))
          ForwardActivationMsg(pMsg);

        //  Send appropriate notifications...

        SendAdvise(OBJECTCODE_SHOWOBJECT);

        return NOERROR;
      }
    }
  }

  RELEASE(m_pInPlaceFrame);
  RELEASE(m_pInPlaceDoc);

  return ResultFromScode(E_FAIL);
}


BOOL CCtrl::MyCreateControlWindow(HWND hWndParent, const CRect& rcPos)
{
  if (m_hWnd == NULL)
  {
    // If window doesn't exist, create it.

    // Test if:
    //  we're not subclassing a Windows control, or
    //  container reflects messages for us...

    DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS;
    if (m_sBorderStyle)
      dwStyle |= WS_BORDER;
    if (!m_bEnabled)
      dwStyle |= WS_DISABLED;

    if ((GetSuperWndProcAddr() == CWnd::GetSuperWndProcAddr()) ||
      ContainerReflectsMessages())
    {
      // Just create the control's window.
      CreateEx(0, NULL, m_strText, dwStyle,
          rcPos.left, rcPos.top,
          rcPos.right - rcPos.left, rcPos.bottom - rcPos.top,
          hWndParent, 0);

      m_pReflect = NULL;

      if (m_hWnd != NULL)
      {
        ShowWindow(SW_SHOWNORMAL);
      }
    }
    else    // ...we're subclassing a Windows control.
    {
      if (m_pReflect == NULL)
      {
        // Create a window to reflect notification messages.
        m_pReflect = new CReflectorWnd;

        if (!m_pReflect->Create(rcPos, hWndParent))
        {
          // If m_pReflect->Create failed, then m_pReflect deleted itself.
          m_pReflect = NULL;
        }
      }
      else
      {
        // Reflector window already exists... just reparent it.
        if (m_pReflect->m_hWnd != NULL)
          ::SetParent(m_pReflect->m_hWnd, hWndParent);
      }

      if (m_pReflect != NULL)
      {
        if (m_pReflect->m_hWnd != NULL)
        {
          // Create the control's window.
          CreateEx(0, NULL, m_strText, dwStyle,
            0, 0,
            rcPos.right - rcPos.left, rcPos.bottom - rcPos.top,
            m_pReflect->m_hWnd, 0);
        }

        if (m_hWnd != NULL)
        {
          //  Make both windows visible.
          m_pReflect->ShowWindow(SW_SHOWNORMAL);
          ShowWindow(SW_SHOWNORMAL);
        }
        else
        {
          // Window creation failed: cleanup.
          if (m_pReflect != NULL)
            m_pReflect->DestroyWindow();

          m_pReflect = NULL;
        }
      }
    }

    // Set the new window's font.
    //OnFontChanged();  // BJO 10Nov97 GRB5822 - Removed
  }
  else
  {
    // If window does exist, reparent and reposition it.
    CWnd* pwndOuter = GetOuterWindow();
    ASSERT(pwndOuter != NULL);

    if (::GetParent(pwndOuter->m_hWnd) != hWndParent)
    {
//            if (GetSuperWndProcAddr() == CWnd::GetSuperWndProcAddr())
//            {
        // Non-subclassed control: just reparent.
        ::SetParent(pwndOuter->m_hWnd, hWndParent);
//            }
//            else
//            {
        // Subclassed control caches parent window, so destroy
        // and create again.
//                DestroyWindow();
//                CreateControlWindow(hWndParent, rcPos);
//            }
    }

    pwndOuter->ShowWindow(SW_SHOWNORMAL);
    pwndOuter->MoveWindow(rcPos, TRUE);
  }

  ASSERT(m_hWnd != NULL);

  return (m_hWnd != NULL);
}
#endif    // not WIN32
