//--------------------------------------------------------------------
//
//  File: cal_oper.h
//
//  Description: Built-in operators
//

#ifndef CAL_OPER_H
#define CAL_OPER_H

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
//
//  The following constants represent the various operator tokens.
//
//  Note: The order of the constants must match the OperTable array
//        in the CAL_OPER.C file.  This allows quick lookups.
//

#define CALC_OPER_NULL      0
#define CALC_OPER_RANGE     1
#define CALC_OPER_NEG       2
#define CALC_OPER_EXPON     3
#define CALC_OPER_MULT      4
#define CALC_OPER_DIV       5
#define CALC_OPER_ADD       6
#define CALC_OPER_SUB       7
#define CALC_OPER_CONCAT    8
#define CALC_OPER_EQUAL     9
#define CALC_OPER_NOTEQUAL  10
#define CALC_OPER_LESS      11
#define CALC_OPER_GREATER   12
#define CALC_OPER_LESSEQ    13
#define CALC_OPER_GREATEREQ 14
#define CALC_OPER_AND       15
#define CALC_OPER_OR        16
#define CALC_OPER_NOT       17

//--------------------------------------------------------------------
//
//  The CALC_OPERATOR structure holds information about an operator.
//

typedef struct tagCALC_OPERATOR
{
  LPTSTR lpszText;  // text representation of operator
  BOOL  bBinary;   // determines if operator is unary or binary
  int   nPred;     // precedence level of the operator
  CALC_EVALPROC lpfnProc;  // procedure used during expression evaluation
} CALC_OPERATOR, far *LPCALC_OPERATOR;

//--------------------------------------------------------------------
//
//  The following functions retrieve information about an operator.
//

int  OperLookup(LPCTSTR lpszText, BOOL bBinary);
int  OperFirst(void);
int  OperNext(int nId);
BOOL OperIsBinary(int nId);
int  OperGetText(int nId, LPTSTR lpszText, int nLen);
int  OperGetTextLen(int nId);
int  OperGetPred(int nId);
CALC_EVALPROC OperGetProc(int nId);

//--------------------------------------------------------------------
//
//  The following functions are used to evaluate an operator.  The
//  implementation of these function can be found in the "cal_func.c"
//  file.
//

BOOL CALLBACK Calc_range(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_neg(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_expon(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_mult(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_div(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_add(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_sub(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_concat(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_equal(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_notequal(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_less(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_greater(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_lesseq(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_greatereq(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_not(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_and(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);
BOOL CALLBACK Calc_or(LPCALC_VALUE lpResult, LPCALC_VALUE lpArg, short nArgs);

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
