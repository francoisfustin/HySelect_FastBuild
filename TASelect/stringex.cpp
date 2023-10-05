#include "stdafx.h"
#include "stringex.h"

std::string FormatString( const char *szFormat, va_list &arg_ptr )
{
	if(szFormat==NULL) AfxThrowUserException();		
	std::string strRet;
	int nSize=_vscprintf(szFormat,arg_ptr);			
	char *szRet= new char[nSize+1];
	vsprintf_s(szRet,sizeof(szRet),szFormat,arg_ptr);
	strRet.assign(szRet);
	delete []szRet;
	return strRet;
}
std::string FormatString( const char *szFormat, ... )
{
	if(szFormat==NULL) AfxThrowUserException();
	std::string strRet;
	va_list arg_ptr;
	va_start(arg_ptr,szFormat);	
	strRet.swap(FormatString( szFormat,arg_ptr ));
	va_end(arg_ptr);	
	return strRet;
}
std::wstring FormatString(const wchar_t *szFormat,va_list &arg_ptr )
{
	if(szFormat==NULL) AfxThrowUserException();
	std::wstring strRet;
	int nSize=_vscwprintf(szFormat,arg_ptr);			
	wchar_t *szRet= new wchar_t[nSize+1];
	
	vswprintf(szRet,sizeof(szRet),szFormat,arg_ptr);

	strRet.assign(szRet);
	delete []szRet;
	return strRet;
		
}
std::wstring FormatString(const wchar_t *szFormat,... )
{
	if(szFormat==NULL) throw (new CInvalidArgException());
	std::wstring strRet;
	va_list arg_ptr;
	va_start(arg_ptr,szFormat);	
	
	strRet.swap(FormatString(szFormat,arg_ptr ));
	va_end(arg_ptr);
	return strRet;
		
}
