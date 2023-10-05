/************
* SS_FUNC.H
************/

#ifndef SS_FUNC_H
#define SS_FUNC_H

#ifdef __cplusplus
extern "C" {
#endif

extern SSX_CALCFUNCTION SS_CalcFunctions[];

/**********************
* Function Prototypes
**********************/

short SSx_CalcFunctionsCnt(void);
BOOL FAR PASCAL SS_FuncAdd(LPSS_VALUE Result, LPSS_VALUE Values,
                                  short ValueCnt);
BOOL FAR PASCAL SS_FuncIf(LPSS_VALUE Result, LPSS_VALUE Values,
                                 short ValueCnt);
BOOL FAR PASCAL SS_FuncIsEmpty(LPSS_VALUE Result, LPSS_VALUE Values,
                                      short ValueCnt);
BOOL FAR PASCAL SS_FuncNeg(LPSS_VALUE Result, LPSS_VALUE Values,
                                  short ValueCnt);
BOOL FAR PASCAL SS_FuncAbs(LPSS_VALUE Result, LPSS_VALUE Values,
                                  short ValueCnt);
BOOL FAR PASCAL SS_FuncNot(LPSS_VALUE Result, LPSS_VALUE Values,
                                  short ValueCnt);
BOOL FAR PASCAL SS_FuncPmt(LPSS_VALUE Result, LPSS_VALUE Values,
                                  short ValueCnt);
BOOL FAR PASCAL SS_FuncRoundUp(LPSS_VALUE Result, LPSS_VALUE Values,
                                      short ValueCnt);
BOOL FAR PASCAL SS_FuncRound(LPSS_VALUE Result, LPSS_VALUE Values,
                                    short ValueCnt);
BOOL FAR PASCAL SS_FuncTruncate(LPSS_VALUE Result, LPSS_VALUE Values,
                                       short ValueCnt);

#ifdef __cplusplus
}
#endif

#endif
