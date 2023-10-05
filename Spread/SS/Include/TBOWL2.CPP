//--------------------------------------------------------------------
//
//  FarPoint Technologies  (C) Copyright 1995,  All Rights Reserved
//
//  32-Bit version of ToolBox spreadsheet OWL wrapper.
//

#include <owl\owlpch.h>
#include <owl\window.h>

#include <tbowl2.h>

DEFINE_RESPONSE_TABLE1(TSpread,TControl)
  EV_WM_VSCROLL,
  EV_WM_HSCROLL,
END_RESPONSE_TABLE;

void TSpread::EvHScroll(UINT /*scrollCode*/, UINT /*thumbPos*/, HWND /*hWndCtl*/)
{
  DefaultProcessing();
}

void TSpread::EvVScroll(UINT /*scrollCode*/, UINT /*thumbPos*/, HWND /*hWndCtl*/)
{
  DefaultProcessing();
}

