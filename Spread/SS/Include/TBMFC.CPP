//***********************************************************************
//* FarPoint Technologies  (C) Copyright 1995  All Rights Reserved
//*
//*  32-Bit version of ToolBox spreadsheet MFC wrapper.
//***********************************************************************

#include "tbmfc.h"
                   

///////////////////////////////////////////////////////////////////////////////////
// TSpread

IMPLEMENT_DYNAMIC(TSpread, CWnd)

BOOL TSpread::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
  return CWnd::Create("tbSpread", NULL, dwStyle, rect, pParentWnd, nID);
}

WNDPROC* TSpread::GetSuperWndProcAddr()
{
  static WNDPROC NEAR pfnSuper;
  return &pfnSuper;
};

