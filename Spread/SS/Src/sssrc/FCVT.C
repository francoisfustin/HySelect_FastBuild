/*
$Revision:   1.3  $
*/


#include <windows.h>
#include <toolbox.h>
#include <stdlib.h>
#include <tchar.h>

/*
char * cdecl fcvt(double, int, int *, int *);
char * cdecl ecvt(double, int, int *, int *);
*/

#if (!defined(_MSC_VER) | (_MSC_VER <= 600))
#define _fcvt fcvt
#define _ecvt ecvt
#endif

static  int      tmp1;
static  int      tmp2;
static  char    *tmp3;

LPTSTR DLLENTRY FloatToFString(LPTSTR szBuff, double value, int ndec, LPINT decpt, LPINT sign)
{
/* value          - value to be converted
   ndec           - number of digits to be stored
   &tmp1,*decpt   - pointer to integer where position of decimal is returned
   &tmp2,*sign    - pointer to integer where sign of number is returned
*/

   // fcvt will be not correct when ndec is bigger than 16
   // Fengwei
	int i;
	//tmp3 = _fcvt(value, min(16, ndec), &tmp1, &tmp2);
   // RFW - -8/24/04 - 15069
	tmp3 = _fcvt(value, min(16, ndec + 1), &tmp1, &tmp2);
	tmp3[tmp1 + ndec] = '\0';

   *decpt = tmp1;
   *sign = tmp2;
   #if defined(_UNICODE)
   mbstowcs(szBuff, tmp3, _fstrlen(tmp3) + 1);
   #else
   StrCpy(szBuff, tmp3);
   #endif

	//- append 0 at the end of szBuff
	//  Fengwei
	for(i = 0; i < ndec - 16; i++)
		lstrcat(szBuff, _T("0"));
	//-

   return(szBuff);
}


LPTSTR DLLENTRY FloatToEString(LPTSTR szBuff, double value, int ndigit, LPINT decpt, LPINT sign)
{
   tmp3 = _ecvt(value, ndigit, &tmp1, &tmp2);
   *decpt = tmp1;
   *sign = tmp2;
   #if defined(_UNICODE)
   mbstowcs(szBuff, tmp3, _fstrlen(tmp3) + 1);
   #else
   StrCpy(szBuff, tmp3);
   #endif
   return(szBuff);
}

