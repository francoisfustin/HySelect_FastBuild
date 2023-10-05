/*    PortTool v2.2     FPTOOL32.C          */

//
// International Settings  (Date/Time)
//
//

#include "fptools.h"
#include <tchar.h>

static unsigned char fIsLoaded = 0L;

//
// [Intl] - date & time formats
//
#define  SSHORTDATE       0x01   // "sShortDate"
#define  SLONGDATE        0x02   // "sLongDate"
#define  ITIME            0x04   // "iTime"
#define  STIME            0x08   // "sTime"
#define  S1159            0x10   // "s1159"
#define  S2359            0x20   // "s2359"
#define  ITLZERO          0x40   // "iTLZero"
#define  SDATE            0x80   // "sDate"
#define  STIMEFORMAT      0x100  // "sTimeFormat"

// International Date Formats
// NEED TO START READING THESE FROM REGISTRY - RegQueryValue?

void FPLIB fpResetIntlDate()
{
	fIsLoaded = 0L;
}
// Short Date Format
//
static TCHAR sShortDate[32];
LPTSTR FPLIB fpGetIntl_sShortDate()
{
   if ((fIsLoaded & SSHORTDATE) == 0)
      {
      GetProfileString(_T("Intl"), _T("sShortDate"), _T("mm/dd/yy"), sShortDate,
         STRING_SIZE(sShortDate));
      fIsLoaded |= SSHORTDATE;
      }
   return sShortDate;
}

// Long Date Format
//
static TCHAR sLongDate[32];
LPTSTR FPLIB fpGetIntl_sLongDate()
{
   if ((fIsLoaded & SLONGDATE) == 0)
      {
      GetProfileString(_T("Intl"), _T("sLongDate"), _T("dd mmm, yyyy"), sLongDate,
         STRING_SIZE(sLongDate));
      fIsLoaded |= SLONGDATE;
      }
   return sLongDate;
}

// Date Separator
//
static TCHAR sDate[2];
LPTSTR FPLIB fpGetIntl_sDate()
{
   if ((fIsLoaded & SDATE) == 0)
      {
      GetProfileString (_T("Intl"), _T("sDate"), _T("/"), sDate, STRING_SIZE(sDate));
      fIsLoaded |= SDATE;
      }
   return sDate;
}

// Time Style 
// 0 - 12 Hour, 1 - 24 Hour
static int iTime;
int FPLIB fpGetIntl_iTime()
{
   if ((fIsLoaded & ITIME) == 0)
      {
      iTime = GetProfileInt (_T("intl"), _T("iTime"), 0);
      fIsLoaded |= ITIME;
      }
   return iTime;
}


// Time Separator
//
static TCHAR sTime[2];
LPTSTR FPLIB fpGetIntl_sTime()
{
   if ((fIsLoaded & STIME) == 0)
      {
      GetProfileString (_T("intl"), _T("sTime"), _T(":"), sTime, STRING_SIZE(sTime));
      fIsLoaded |= STIME;
      }
   return sTime;
}

// AM String
//
static TCHAR s1159[12];
LPTSTR FPLIB fpGetIntl_s1159()
{
   if ((fIsLoaded & S1159) == 0)
      {
      GetProfileString (_T("intl"), _T("s1159"), _T("AM"), s1159, STRING_SIZE(s1159));
      fIsLoaded |= S1159;
      }
   return s1159;
}

// PM String
//
static TCHAR s2359[12];
LPTSTR FPLIB fpGetIntl_s2359()
{
   if ((fIsLoaded & S2359) == 0)
      {
      GetProfileString (_T("intl"), _T("s2359"), _T("PM"), s2359, STRING_SIZE(s2359));
      fIsLoaded |= S2359;
      }
   return s2359;
}
 
// Time Leading Zero 
// 
static int iTLZero;
int FPLIB fpGetIntl_iTLZero()
{
   if ((fIsLoaded & ITLZERO) == 0)
      {
      iTLZero = GetProfileInt (_T("intl"), _T("iTLZero"), 0);
      fIsLoaded |= ITLZERO;
      }
   return iTLZero;
}


// Time Format
//
static TCHAR sTimeFormat[32];
LPTSTR FPLIB fpGetIntl_sTimeFormat()
{
   if ((fIsLoaded & STIMEFORMAT) == 0)
      {
      GetProfileString (_T("intl"), _T("sTimeFormat"), _T(""), sTimeFormat, STRING_SIZE(sTimeFormat));
      fIsLoaded |= STIMEFORMAT;
      }
   return sTimeFormat;
}
