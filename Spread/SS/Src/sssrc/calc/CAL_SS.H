//---------*---------*---------*---------*---------*---------*---------*-----
//
// MODULE:		calc_ss.h
//
// PURPOSE:		Constants and structures which will be added to or modified
//              in the TOOLBOX.H header file.
//
// COPYRIGHT:   Copyright (c)1995 - FarPoint Technologies and Bunka Orient Co. 
//              All rights reserved.
//
//				No part of this source code may be copied, modified or 
//				reproduced in any form without retaining the above copyright 
//				notice.  This source code, or source code derived from it, 
//				may not be redistributed without express written permission 
//				of FarPoint Technologies.
//
// UPDATE LOG:
//
//		<flag>	<fixed date>	<bug #> - Description
//
//---------*---------*---------*---------*---------*---------*---------*-----


#ifndef CALC_SS_H
#define CALC_SS_H

#ifndef RC_INVOKED

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  This will be the new SS_VALUE structure (extended to handle cell
//  and range references).
//

#define CALC_VALUE_STATUS_OK    0
#define CALC_VALUE_STATUS_ERROR 1
#define CALC_VALUE_STATUS_EMPTY 2
#define CALC_VALUE_STATUS_CLEAR 3
#define CALC_VALUE_STATUS_NONE  4
        
#define CALC_VALUE_TYPE_LONG   0
#define CALC_VALUE_TYPE_DOUBLE 1
#define CALC_VALUE_TYPE_STR    2
#define CALC_VALUE_TYPE_CELL   3
#define CALC_VALUE_TYPE_RANGE  4
#define CALC_VALUE_TYPE_DDE    5

#define CALC_WANTCELLREF  1
#define CALC_WANTRANGEREF 2
#define CALC_WANTDDEREF   4

#if defined(_WIN64) || defined(_IA64)
#pragma pack(8)
#else
#pragma pack(1)
#endif

typedef struct tagCALC_CELLREF
{
  #ifdef SS_V70
  short nSheet;
  #endif
  long Col;
  long Row;
  union
  {
    HWND hwndSS;
#if defined(_WIN64) || defined(_IA64)
	LONG_PTR lInfo;
#else
    long lInfo;
#endif
  } u;
} CALC_CELLREF, FAR* LPCALC_CELLREF;

typedef struct tagCALC_RANGEREF
{
  #ifdef SS_V70
  short nSheet;
  #endif
  long Col1;
  long Row1;
  long Col2;
  long Row2;
  union
  {
    HWND hwndSS;
#if defined(_WIN64) || defined(_IA64)
	LONG_PTR lInfo;
#else
    long lInfo;
#endif
  } u;
} CALC_RANGEREF, FAR* LPCALC_RANGEREF;

typedef struct tagCALC_DDEREF
{
  CALC_HANDLE hDde;
  long lCol;
  long lRow;
} CALC_DDEREF, FAR* LPCALC_DDEREF;

typedef struct tagCALC_VALUE
{
  BYTE Status;
  BYTE Type;
  union
  {
    HGLOBAL       hValStr;
    long          ValLong;
    double        ValDouble;
    CALC_CELLREF  ValCell;
    CALC_RANGEREF ValRange;
    CALC_DDEREF   ValDde;
  } Val;
} CALC_VALUE, FAR* LPCALC_VALUE;


#ifdef SS_V80
typedef struct tagGOAL_SEEK
{
    double xmin;
    double xmax;
    double precision;
    BOOL havexpos;
    double xpos;
    double ypos;
    BOOL havexneg;
    double xneg;
    double yneg;
    double root;

} GOAL_SEEK, FAR* LPGOAL_SEEK;

#endif
#pragma pack()

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif

#endif
