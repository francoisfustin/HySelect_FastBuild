/***************
*  SS_DATA.H
***************/

#ifndef SS_DATA_H
#define SS_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

TBGLOBALHANDLE SS_GetValue(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
int DLLENTRY SS_GetValueEx(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPTSTR lpBuffer, int iMaxLen);
BOOL SS_SetValue(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPCTSTR lpData);
BOOL SS_SetValueRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, SS_COORD Col2, SS_COORD Row2, LPCTSTR lpData);
BOOL SS_SetData(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPCTSTR lpData);

#if defined(_WIN64) || defined(_IA64)
LONG_PTR SS_GetUserData(LPSPREADSHEET lpSS);
LONG_PTR SS_SetUserData(LPSPREADSHEET lpSS, LONG_PTR lUserData);
#else
LONG SS_GetUserData(LPSPREADSHEET lpSS);
LONG SS_SetUserData(LPSPREADSHEET lpSS, LONG lUserData);
#endif

int  SS_GetDataLen(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
int  SS_GetDataEx(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPTSTR lpBuffer, int iMaxLen);
int  SS_GetValueLen(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row);
BOOL SS_SetFloatRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                      SS_COORD Col2, SS_COORD Row2, double dfValue);
BOOL SS_GetFloat(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                 LPDOUBLE lpdfValue);
BOOL SS_SetIntegerRange(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                        SS_COORD Col2, SS_COORD Row2, long lValue);
BOOL SS_GetInteger(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row,
                   LPLONG lplValue);

//>>99918148 Leon 20101104
GLOBALHANDLE SS_DoubleQuotes_Encapsulation(LPTSTR lpData);
TBGLOBALHANDLE SS_DoubleQuotes_Decapsulation(TBGLOBALHANDLE hTextInput, int newsize);
//<<99918148 Leon 20101104
#if SS_V80
//BOOL SS_GetHandle(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPHANDLE pValue, LPBOOL pfOwn, SS_FREEHANDLE_FUNC *ppFreeFunc);
//BOOL SS_SetHandle(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, HANDLE hValue, BOOL fOwn, SS_FREEHANDLE_FUNC pFreeFunc);
BOOL SS_DataToCTValue(LPSS_CT_VALUE Value, LPSS_DATA DataItem);
BOOL SS_CTValueToData(LPSS_CT_VALUE Value, LPSS_DATA DataItem);
void SS_FreeCTValue(LPSS_CT_VALUE Value);
BOOL SS_GetBuffer(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPVOID lpBuffer, long *plSize);
BOOL SS_SetBuffer(LPSPREADSHEET lpSS, SS_COORD Col, SS_COORD Row, LPVOID lpBuffer, long lSize);
#endif
#ifdef __cplusplus
}
#endif

#endif
