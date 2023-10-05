#include <windows.h>
#include "toolbox.h"

#if !defined(WIN32)
#include <dos.h>
#endif

//--------------------------------------------------------------------
//
//  The SysGetDate() function retrieves the current date.
//

VOID DLLENTRY SysGetDate(LPINT lpDay, LPINT lpMonth, LPINT lpYear)
{
  #if defined(WIN32)

  SYSTEMTIME sysTime;
  GetLocalTime(&sysTime);
  *lpDay = sysTime.wDay;
  *lpMonth = sysTime.wMonth;
  *lpYear = sysTime.wYear;

  #else

  static struct _dosdate_t dosDate;
  _dos_getdate(&dosDate);
  *lpDay = dosDate.day;
  *lpMonth = dosDate.month;
  *lpYear = dosDate.year;

  #endif
}


//--------------------------------------------------------------------
//
//  The SysGetTime() function retrieves the current time.
//

VOID DLLENTRY SysGetTime(LPINT lpHour, LPINT lpMinute, LPINT lpSeconds)
{
  #if defined(WIN32)

  SYSTEMTIME sysTime;
  GetLocalTime(&sysTime);
  *lpHour = sysTime.wHour;
  *lpMinute = sysTime.wMinute;
  *lpSeconds = sysTime.wSecond;

  #else

  static struct _dostime_t dosTime;
  _dos_gettime(&dosTime);
  *lpHour = dosTime.hour;
  *lpMinute = dosTime.minute;
  *lpSeconds = dosTime.second;

  #endif
}
