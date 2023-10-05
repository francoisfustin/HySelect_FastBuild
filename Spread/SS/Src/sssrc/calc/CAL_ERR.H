//--------------------------------------------------------------------
//
//  File: cal_err.h
//
//  Description: Calculation error codes
//

#ifndef CAL_ERR_H
#define CAL_ERR_H

#ifdef __cplusplus
extern "C" {
#endif


//--------------------------------------------------------------------
//
//  The following constants represent the various calculation errors.
//
//  Note: The order of the constants must match the ErrorTable array
//        in the CAL_ERR.C file.  This allows quick lookups.
//

#define CALC_ERROR_DIV0  1
#define CALC_ERROR_NA    2
#define CALC_ERROR_NAME  3
#define CALC_ERROR_NULL  4
#define CALC_ERROR_NUM   5
#define CALC_ERROR_REF   6
#define CALC_ERROR_VALUE 7

//--------------------------------------------------------------------
//
//  The following functions are used to handle calculation errors.
//

int ErrLookup(LPTSTR lpszText, int nTextLen);
int ErrGetText(int nId, LPTSTR lpszText, int nLen);
int ErrGetTextLen(int nId);

//--------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
