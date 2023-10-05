#include "fptools.h"

#if (defined(FP_VB) || defined(FP_OCX))
extern PROPINFO fpPropLongWeekDayName;
extern PROPINFO fpPropLongMonthName;
extern PROPINFO fpPropShortWeekDayName;
extern PROPINFO fpPropShortMonthName;
#endif  //not FP_DLL
      
long FPLIB FP_LongWeekDayName (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_LongMonthName (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_ShortWeekDayName (LPVBPROPPARAMS lpPP, BOOL fGetProp);
long FPLIB FP_ShortMonthName (LPVBPROPPARAMS lpPP, BOOL fGetProp);

// wryintl.c
//
LPTSTR FPLIB DateMonthName(short month, BOOL bLong);
LPTSTR FPLIB DateDayName(short month, BOOL bLong);
