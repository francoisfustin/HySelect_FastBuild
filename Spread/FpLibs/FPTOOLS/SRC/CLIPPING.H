#ifndef CLIPPING_H
#define CLIPPING_H
//---------------------------------------------------------------------------
//        Copyright (C) 1992-3, Microsoft Corporation
//---------------------------------------------------------------------------
// Clipping.h
//---------------------------------------------------------------------------
// Contains clipping code for Utopia apps.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Registered messages
//---------------------------------------------------------------------------
#ifdef UCLIPPING
UINT nRgMsgGetRgn; 
#define INIT_CLIPPING nRgMsgGetRgn = RegisterWindowMessage("UtopiaGetRgn") 
#endif
extern UINT nRgMsgGetRgn;

//---------------------------------------------------------------------------
// Exported routines
//---------------------------------------------------------------------------

HDC _export UGetDC(HWND hwnd);
HDC _export UBeginPaint(HWND hwnd, PAINTSTRUCT FAR * pps);
HDC _export UGetWindowDC(HWND hwnd);
void _export ClipWnds(HWND hwnd, HRGN hrgn, BOOL fAll);
void _export ClipWnds(HWND hwnd, HDC hdc, BOOL fAll);

#endif // CLIPPING_H
