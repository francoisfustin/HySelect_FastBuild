//--------------------------------------------------------------------
//
//  File: ftdattim.h
//
//  Description: Date and time routines
//
//  Copyright (c) 1999 by FarPoint Technologies, Inc.
//
//  All rights reserved.  No part of this source code may be
//  copied, modified or reproduced in any form without retaining
//  the above copyright notice.  This source code, or source code
//  derived from it, may not be redistributed without express
//  written permission of FarPoint Technologies, Inc.
//
//--------------------------------------------------------------------

#if !defined(FTDATTIM_H)
#define FTDATTIM_H

#if defined(__cplusplus)
extern "C" {
#endif

BOOL ftDoubleToDate(double dfVal, short *pnYear, short *pnMon, short *pnDay);
BOOL ftDateToDouble(int nYear, int nMon, int nDay, double *pdfVal);

BOOL ftDoubleToTime(double dfVal, short *pnHour, short *pnMin, short *pnSec);

// Same as above but includes Milliseconds
BOOL ftDoubleToTimeEx(double dfVal, short *pnHour, short *pnMin, 
  short *pnSec, short *pnMilliSec);

BOOL ftTimeToDouble(int nHour, int nMin, int nSec, double *pdfVal);

void ftDoubleToWeekday(double dfVal, short *pnWeekday);

int LastDayOfMonth(int iMonth, int iYear);


#if defined(__cplusplus)
}
#endif

#endif
