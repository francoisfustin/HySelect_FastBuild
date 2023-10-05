/************
* SS_CALC.H
************/

#ifndef SS_CALC_H
#define SS_CALC_H

#ifdef __cplusplus
extern "C" {
#endif

#define OP_NONE           0
#define OP_ADDITION       1
#define OP_SUBTRACTION    2
#define OP_POW            3
#define OP_MULTIPLICATION 4
#define OP_DIVISION       5
#define OP_AND            6
#define OP_OR             7
#define OP_NOTEQUAL       8
#define OP_GREATER        9
#define OP_GREATEREQUAL   10
#define OP_LESS           11
#define OP_LESSEQUAL      12
#define OP_EQUAL          13

#define SS_CALC_PRFX_STR  '\"'
#define SS_CALC_WILDCARD  '#'

#define SS_CALC_ALLOC_CNT 10

#ifdef SS_OLDCALC
  #define SS_CALC_FORMULA_MAXLEN 500
#else
  #define SS_CALC_FORMULA_MAXLEN 1023
#endif

typedef struct ssx_calcfunction
   {
   LPTSTR       Name;
   short       ParameterCnt;
   SS_CALCFUNC Function;
   } SSX_CALCFUNCTION, FAR *LPSSX_CALCFUNCTION;

typedef struct ss_calcfunction
   {
   TBGLOBALHANDLE hName;
   short          ParameterCnt;
   SS_CALCFUNC    Function;
   } SS_CALCFUNCTION, FAR *LPSS_CALCFUNCTION;

/**********************
* Function Prototypes
**********************/

void   SS_Calc(LPSPREADSHEET lpSS, LPTSTR Formula, LPSS_VALUE Result);
BOOL   SS_CalcIsValid(LPSPREADSHEET lpSS, LPCTSTR Formula,
                      TBGLOBALHANDLE hCalc);
LPCTSTR SSx_Calculate(LPSPREADSHEET lpSS, LPCTSTR Formula,
                      LPSS_VALUE Result, TBGLOBALHANDLE ValidateCalc);
void   SSx_Evaluate(short Operation, LPSS_VALUE Val1, LPSS_VALUE Val2,
                    LPSS_VALUE Result);
LPCTSTR SSx_ParseNumber(LPCTSTR Formula, LPSS_VALUE ValTemp);
LPCTSTR SSx_ParseItem(LPSPREADSHEET lpSS, LPCTSTR Formula,
                      LPSS_VALUE Value, TBGLOBALHANDLE ValidateCalc);
LPCTSTR SSx_ParseStr(LPCTSTR Formula, LPSS_VALUE Value);
void   SSx_EvalAdd(LPSS_VALUE Val1, LPSS_VALUE Val2, LPSS_VALUE Result);
void   SSx_EvalDivide(LPSS_VALUE Val1, LPSS_VALUE Val2,
                      LPSS_VALUE Result);
void   SSx_EvalMultiply(LPSS_VALUE Val1, LPSS_VALUE Val2,
                        LPSS_VALUE Result);
void   SSx_EvalSubtract(LPSS_VALUE Val1, LPSS_VALUE Val2,
                        LPSS_VALUE Result);
void   SSx_EvalExponential(LPSS_VALUE Val1, LPSS_VALUE Val2,
                           LPSS_VALUE Result);
void   SSx_EvalEqual(short Operation, LPSS_VALUE Val1, LPSS_VALUE Val2,
                     LPSS_VALUE Result);
void   SSx_EvalAnd(LPSS_VALUE Val1, LPSS_VALUE Val2, LPSS_VALUE Result);
void   SSx_EvalOr(LPSS_VALUE Val1, LPSS_VALUE Val2, LPSS_VALUE Result);
LPCTSTR SSx_ParseFunction(LPSPREADSHEET lpSS, LPCTSTR Formula,
                          LPSS_VALUE Result, TBGLOBALHANDLE ValidateCalc);
LPCTSTR SSx_CalcGetParm(LPSPREADSHEET lpSS, LPCTSTR Formula,
                        LPSS_VALUE Result, BOOL LastParm,
                        BOOL FAR *IsLastParm,
                        TBGLOBALHANDLE ValidateCalc);
double SS_RoundUp(double Val, long Precision);
double SS_Round(double Val, long Precision);
double SS_Truncate(double Val, long Precision);
LPCTSTR SSx_SkipBlanks(LPCTSTR Str);
BOOL   SSx_CalcTableAdd(LPSPREADSHEET lpSS,
                        TBGLOBALHANDLE hCalc);
BOOL   SSx_CalcTableDelete(LPSPREADSHEET lpSS,
                           LPSS_CELLCOORD CellCoord);
BOOL   SS_ReCalcAll(LPSPREADSHEET lpSS);
BOOL   SS_ReCalcCell(LPSPREADSHEET lpSS, SS_COORD lCol, SS_COORD lRow);
BOOL   SS_CalcDependencies(LPSPREADSHEET lpSS,
                           SS_COORD Col, SS_COORD Row);
BOOL   SSx_CalcCell(LPSPREADSHEET lpSS, LPSS_CELLCOORD CellCoord);
void   SS_FreeCalcFunctions(LPSPREADSHEET lpSS);

BOOL SS_BuildDependencies(LPSPREADSHEET lpSS);

BOOL SS_AddCustomFunctionExt(LPSS_BOOK lpBook, LPCTSTR lpszFuncName,
                             short nMinParamCnt, short nMaxParamCnt,
                             FARPROC lpfnFuncProc, long Flags);
BOOL SS_RemoveCustomFunction(LPSS_BOOK lpBook, LPCTSTR lpszFuncName);
int  SS_EnumCustomFunction(LPSS_BOOK lpBook, LPCTSTR lpszPrevFunc,
                           LPTSTR lpszFunc, int nFuncLen);
int  SS_EnumCustomFunctionLen(LPSS_BOOK lpBook, LPCTSTR lpszPrevFunc);
BOOL SS_GetCustomFunction(LPSS_BOOK lpBook, LPCTSTR lpszFunc,
                          LPSHORT lpnMinParamCnt, LPSHORT lpnMaxParamCnt,
                          LPLONG lplFlags);

#ifdef __cplusplus
}
#endif

#endif
