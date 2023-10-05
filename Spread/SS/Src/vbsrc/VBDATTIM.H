/*************
* VBDATTIM.H
*************/

#ifndef VBDATTIM_H
#define VBDATTIM_H

#ifdef __cplusplus
extern "C" {
#endif

/**********************
* Function Prototypes
**********************/

extern BOOL      DateValidate(LPTSTR lpszText, LPTB_DATE lpDate);
extern void      DateValueToDMY(LPTSTR lpszText, LPTB_DATE lpDate);
extern BOOL      DateToString(LPTSTR lpszText, LPTB_DATE lpDate);
extern short     DateIsLeapYear(short Year);
extern short     DateDaysPerMonth(short Month, short Year);
extern long      vbDateDMYToJulian(LPTB_DATE lpDate);
extern LPTB_DATE vbDateJulianToDMY(long jul, LPTB_DATE da);
extern BOOL      DateIsValid(short Month, short Day, short Year);
extern void      DateDefault(LPDATEFORMAT lpDateFormat);

extern BOOL      TimeValidate(LPTSTR lpszText, LPTIME lpTime);
extern BOOL      TimeToString(LPTSTR lpszText, LPTIME lpTime);
extern void      TimeDefault(LPTIMEFORMAT lpTimeFormat);

#ifdef __cplusplus
}
#endif

#endif
