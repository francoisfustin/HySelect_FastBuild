#include <windows.h>
#include <toolbox.h>
#include <stdarg.h>

/*
$Revision:   1.2  $
*/


far pascal _output(LPTSTR fp, LPCTSTR format, va_list FAR* lpParms);

int   cdecl fsprintf(LPTSTR buff, LPTSTR format,...)
{
   va_list arglist;
   int   count;

   va_start(arglist, format);
   count = _output((LPTSTR)&buff, format, &arglist);
   va_end(arglist);
   *buff = '\0';
   return(count);
}

int FAR cdecl StrPrintf(LPTSTR buff, LPCTSTR format,...)
{
   va_list arglist;
   int   count;

   va_start(arglist, format);
   count = _output((LPTSTR)&buff, format, &arglist);
   va_end(arglist);
   *buff = '\0';
   return(count);
}
