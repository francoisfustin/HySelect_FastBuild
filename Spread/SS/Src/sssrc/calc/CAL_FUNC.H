//--------------------------------------------------------------------
//
//  File: cal_func.h
//
//  Description: Built-in functions
//

#ifndef CAL_FUNC_H
#define CAL_FUNC_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The following constants represent the various built in functions.
//
//  Note: The order of the constants must match the FuncTable array
//        in the CAL_FUNC.C file.  This allows quick lookups.
//

#define CALC_FUNC_NULL      0
#define CALC_FUNC_ABS       1
#define CALC_FUNC_ADD       2
#ifdef SS_V35
#define CALC_FUNC_AND_OLD   3
#else
#define CALC_FUNC_AND       3
#endif
#define CALC_FUNC_IF        4
#define CALC_FUNC_ISEMPTY   5
#define CALC_FUNC_MAX       6
#define CALC_FUNC_MIN       7
#define CALC_FUNC_NEG       8
#define CALC_FUNC_NOT       9
#ifdef SS_V35
#define CALC_FUNC_OR_OLD   10
#else
#define CALC_FUNC_OR       10
#endif
#define CALC_FUNC_ROUND    11
#define CALC_FUNC_ROUNDUP  12
#define CALC_FUNC_SUM      13
#define CALC_FUNC_TRUNCATE 14
#define CALC_FUNC_PMT      15
#define CALC_FUNC_DDELINK  16
#define CALC_FUNC_ACOS     17
#define CALC_FUNC_ASIN     18
#define CALC_FUNC_ATAN     19
#define CALC_FUNC_ATAN2    20
#define CALC_FUNC_AVERAGE  21
#define CALC_FUNC_COS      22
#define CALC_FUNC_COSH     23
#define CALC_FUNC_DEGREES  24
#define CALC_FUNC_EXP      25
#define CALC_FUNC_FACT     26
#define CALC_FUNC_INVERSE  27
#define CALC_FUNC_LN       28
#define CALC_FUNC_LOG      29
#define CALC_FUNC_LOG10    30
#define CALC_FUNC_MEDIAN   31
#define CALC_FUNC_MOD      32
#define CALC_FUNC_PI       33
#define CALC_FUNC_POWER    34
#define CALC_FUNC_RADIANS  35
#define CALC_FUNC_RANK     36
#define CALC_FUNC_SIN      37
#define CALC_FUNC_SINH     38
#define CALC_FUNC_SQRT     39
#define CALC_FUNC_SQUARE   40
#define CALC_FUNC_STDEV    41
#define CALC_FUNC_TAN      42
#define CALC_FUNC_TANH     43
#define CALC_FUNC_XROOT    44
#define CALC_FUNC_COMBIN   45
#define CALC_FUNC_CEILING  46
#define CALC_FUNC_FLOOR    47
#define CALC_FUNC_PRODUCT  48
#define CALC_FUNC_EVEN     49
#define CALC_FUNC_ODD      50
#define CALC_FUNC_INT      51
#define CALC_FUNC_RAND     52
#define CALC_FUNC_MODE     53
#define CALC_FUNC_STDEVP   54
#define CALC_FUNC_VAR      55
#define CALC_FUNC_VARP     56
#define CALC_FUNC_SLN      57
#define CALC_FUNC_SYD      58
#define CALC_FUNC_SUMSQ    59
#define CALC_FUNC_FV       60
#define CALC_FUNC_PV       61
#define CALC_FUNC_NPER     62
#define CALC_FUNC_DB       63
#define CALC_FUNC_DDB      64
#define CALC_FUNC_TRUE     65
#define CALC_FUNC_FALSE    66
#define CALC_FUNC_SIGN     67
#define CALC_FUNC_PERMUT   68
#define CALC_FUNC_ISEVEN   69
#define CALC_FUNC_ISODD    70
#define CALC_FUNC_ISNUMBER 71
#define CALC_FUNC_ISTEXT   72
#define CALC_FUNC_ISNONTEXT 73
#define CALC_FUNC_ISREF    74
#define CALC_FUNC_URL		   75
#define CALC_FUNC_ACOSH    76
#ifdef SS_V35
#define CALC_FUNC_AND      77
#endif
#define CALC_FUNC_ASINH    78
#define CALC_FUNC_ATANH    79
#define CALC_FUNC_CHAR     80 
#define CALC_FUNC_CLEAN    81
#define CALC_FUNC_CODE     82
#define CALC_FUNC_CONCATENATE 83
#define CALC_FUNC_EXACT    84
#define CALC_FUNC_FIND     85
#define CALC_FUNC_ISBLANK  86
#define CALC_FUNC_DATE     87
#define CALC_FUNC_DAY      88
#define CALC_FUNC_HOUR     89
#define CALC_FUNC_LEFT     90
#define CALC_FUNC_LEN      91
#define CALC_FUNC_LOWER    92
#define CALC_FUNC_MID      93
#define CALC_FUNC_MINUTE   94 
#define CALC_FUNC_MONTH    95
#define CALC_FUNC_NOW      96
#ifdef SS_V35
#define CALC_FUNC_OR       97
#endif
#define CALC_FUNC_PROPER   98
#define CALC_FUNC_REPLACE  99 
#define CALC_FUNC_REPT     100
#define CALC_FUNC_RIGHT    101
#define CALC_FUNC_ROUNDDOWN 102
#define CALC_FUNC_SECOND   103 
#define CALC_FUNC_SUBSTITUTE 104
#define CALC_FUNC_TIME     105
#define CALC_FUNC_TODAY    106
#define CALC_FUNC_TRIM     107
#define CALC_FUNC_TRUNC    108
#define CALC_FUNC_UPPER    109
#define CALC_FUNC_WEEKDAY  110
#define CALC_FUNC_YEAR     111
#ifdef SS_V80
#define CALC_FUNC_AVEDEV   112
#define CALC_FUNC_BINOMDIST 113
#define CALC_FUNC_CRITBINOM 114
#define CALC_FUNC_NEGBINOMDIST 115
#define CALC_FUNC_SQRTPI 116
#define CALC_FUNC_QUOTIENT 117
#define CALC_FUNC_DEVSQ 118
#define CALC_FUNC_NPV 119
#define CALC_FUNC_IPMT 120
#define CALC_FUNC_PPMT 121
#define CALC_FUNC_VDB 122
#define CALC_FUNC_ISPMT 123
#define CALC_FUNC_FVSCHEDULE 124
#define CALC_FUNC_EFFECT 125
#define CALC_FUNC_GCD 126
#define CALC_FUNC_LCM 127
#define CALC_FUNC_NOMINAL 128
#define CALC_FUNC_COUNT 129
#define CALC_FUNC_COUNTA 130
#define CALC_FUNC_SUBTOTAL 131
#define CALC_FUNC_SUMPRODUCT 132
#define CALC_FUNC_SERIESSUM 133
#define CALC_FUNC_MULTINOMIAL 134
#define CALC_FUNC_BETADIST 135
#define CALC_FUNC_FDIST 136
#define CALC_FUNC_NORMSDIST 137
#define CALC_FUNC_NORMDIST 138
#define CALC_FUNC_NORMSINV 139
#define CALC_FUNC_NORMINV 140
#define CALC_FUNC_TDIST 141
#define CALC_FUNC_TINV 142
#define CALC_FUNC_SKEW 143
#define CALC_FUNC_POISSON 144
#define CALC_FUNC_WEIBULL 145
#define CALC_FUNC_GAMMALN 146
#define CALC_FUNC_GAMMADIST 147
#define CALC_FUNC_GAMMAINV 148
#define CALC_FUNC_FACTDOUBLE 149
#define CALC_FUNC_FISHER 150
#define CALC_FUNC_FISHERINV 151
#define CALC_FUNC_LOGNORMDIST 152
#define CALC_FUNC_LOGINV 153
#define CALC_FUNC_BETAINV 154
#define CALC_FUNC_FINV 155
#define CALC_FUNC_STANDARDIZE 156
#define CALC_FUNC_EXPONDIST 157
#define CALC_FUNC_CONFIDENCE 158
#define CALC_FUNC_HYPGEOMDIST 159
#define CALC_FUNC_CHIDIST 160
#define CALC_FUNC_CHIINV 161
#define CALC_FUNC_DAYS360 162
#define CALC_FUNC_ACCRINT 163
#define CALC_FUNC_ACCRINTM 164
#define CALC_FUNC_YEARFRAC 165
#define CALC_FUNC_DISC 166
#define CALC_FUNC_INTRATE 167
#define CALC_FUNC_RECEIVED 168
#define CALC_FUNC_TBILLEQ 169
#define CALC_FUNC_TBILLPRICE 170
#define CALC_FUNC_TBILLYIELD 171
#define CALC_FUNC_PRICE 172
#define CALC_FUNC_PRICEDISC 173
#define CALC_FUNC_PRICEMAT 174
#define CALC_FUNC_CUMIPMT  175
#define CALC_FUNC_CUMPRINC 176
#define CALC_FUNC_COUPDAYBS 177
#define CALC_FUNC_COUPDAYS 178
#define CALC_FUNC_COUPDAYSNC 179
#define CALC_FUNC_COUPNCD 180
#define CALC_FUNC_COUPNUM  181
#define CALC_FUNC_COUPPCD 182
#define CALC_FUNC_IRR 183
#define CALC_FUNC_MIRR 184
#define CALC_FUNC_SUMIF 185
#define CALC_FUNC_COUNTIF 186
#define CALC_FUNC_PMTXL 187
#endif

//--------------------------------------------------------------------
//
//  The CALC_FUNCTION structure represents a built-in function.
//

typedef struct tagCALC_FUNCTION
{
  LPCTSTR lpszText;             // text representation of function
  CALC_EVALPROC    lpfnProc;    // evaluation procedure
  short nMinArgs;               // min number of arguements
  short nMaxArgs;               // max number of arguements
  long lFlags;                  // want cell and/or range references
} CALC_FUNCTION, FAR* LPCALC_FUNCTION;

//--------------------------------------------------------------------
//
//  The following functions are used to handle the table of built-in
//  functions.
//

int FuncLookup(LPCTSTR lpszName);
int FuncFirst(void);
int FuncNext(LPCTSTR lpszName);

//--------------------------------------------------------------------
//
//  The following functions are used to handle an individual built-in
//  functions.
//
int FuncGetText(int nId, LPTSTR lpszText, int nLen);
int FuncGetTextLen(int nId);
short FuncGetMinArgs(int nId);
short FuncGetMaxArgs(int nId);
long FuncGetFlags(int nId);
CALC_EVALPROC FuncGetProc(int nId);

//--------------------------------------------------------------------

#if !defined(WCHAR)
  typedef wchar_t WCHAR;
  typedef WCHAR FAR* LPWSTR;
#endif

//--------------------------------------------------------------------

void ValFree(LPCALC_VALUE lpValue);

BOOL ValGetBool(LPCALC_VALUE lpValue);
long ValGetLong(LPCALC_VALUE lpValue);
double ValGetDouble(LPCALC_VALUE lpValue);
HGLOBAL ValGetStringHandle(LPCALC_VALUE lpValue);
void ValGetCell(LPCALC_VALUE lpValue, LPCALC_SHEET FAR* lplpSheet, LPLONG lplCol, LPLONG lplRow);
void ValGetRange(LPCALC_VALUE lpValue, LPCALC_SHEET FAR* lplpSheet, LPLONG lplCol1, LPLONG lplRow1, LPLONG lplCol2, LPLONG lplRow2);

void ValSetError(LPCALC_VALUE lpResult);
void ValSetEmpty(LPCALC_VALUE lpResult);
void ValSetValue(LPCALC_VALUE lpResult, LPCALC_VALUE lpValue);
void ValSetBool(LPCALC_VALUE lpResult, BOOL bValue);
void ValSetLong(LPCALC_VALUE lpResult, long lValue);
void ValSetDouble(LPCALC_VALUE lpResult, double dValue);
void ValSetString(LPCALC_VALUE lpResult, LPTSTR lpszValue);
void ValSetStringW(LPCALC_VALUE lpResult, LPWSTR lpszValue);
void ValSetCell(LPCALC_VALUE lpResult, LPCALC_SHEET lpSheet, long lCol, long lRow);
void ValSetRange(LPCALC_VALUE lpResult, LPCALC_SHEET lpSheet, long lCol1, long lRow1, long lCol2, long lRow2);
void ValSetDde(LPCALC_VALUE lpResult, CALC_HANDLE hDde, long lCol, long lRow);

BOOL CheckAnyError(LPCALC_VALUE lpArg, short nArgs);
BOOL CheckAnyEmpty(LPCALC_VALUE lpArg, short nArgs);
BOOL CheckAllNum(LPCALC_VALUE lpArg, short nArgs, BOOL FAR* pbAnyDouble);
BOOL CheckAllBoolOrEmpty(LPCALC_VALUE lpArg, short nArgs, BOOL FAR* pbAnyBool, BOOL FAR* pbAnyEmpty);
BOOL CheckAllNumOrEmpty(LPCALC_VALUE lpArg, short nArgs, BOOL FAR* pbAnyDouble, BOOL FAR* pbAnyLong, BOOL FAR* pbAnyEmpty);
BOOL CheckAllStrOrEmpty(LPCALC_VALUE lpArg, short nArgs, BOOL FAR* pbAnyStr);
BOOL CheckAllCellOrRange(LPCALC_VALUE lpArg, short nArgs);

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
