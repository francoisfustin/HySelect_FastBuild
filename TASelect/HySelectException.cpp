#include "stdafx.h"


#include <atlsimpstr.h>
#include "HySelectException.h"
#include "TASelect.h"

CHySelectException::CHySelectException()
{
	m_vecMsgIter = m_vecMsg.end();
}

CHySelectException::CHySelectException( CString strMessage, int iLine, CStringA strFile, ... )
{
	va_list arglist;
	va_start( arglist, strFile );
	CString strAllText;
	strAllText.FormatV( strMessage, arglist );
	va_end( arglist );

	AddMessage( strAllText, iLine, strFile );
}

CHySelectException::CHySelectException( CHySelectException &clHySelectException )
{
	*this = clHySelectException;
}

CHySelectException::CHySelectException( CHySelectException &clHySelectException, CString strMessage, int iLine, CStringA strFile, ... )
{
	*this = clHySelectException;

	va_list arglist;
	va_start( arglist, strFile );
	CString strAllText;
	strAllText.FormatV( strMessage, arglist );
	va_end( arglist );

	AddMessage( strAllText, iLine, strFile );
}

CHySelectException::CHySelectException( CHySelectException const &clHySelectException )
{
	*this = clHySelectException;
}

CHySelectException &CHySelectException::operator=( const CHySelectException &clHySelectException )
{
	const std::vector<msginfo> *pvecMsg = clHySelectException.GetMessageList();

	if( NULL != pvecMsg )
	{
		m_vecMsg = *pvecMsg;
	}

	return *this;
}

void CHySelectException::ClearMessages()
{
	m_vecMsg.clear();
}

void CHySelectException::AddMessage( CString strMessage, int iLine, CStringA strFile, ... )
{
	va_list arglist;
	va_start( arglist, strFile );
	CString strAllText;
	strAllText.FormatV( strMessage, arglist );
	va_end( arglist );

	m_vecMsg.push_back( msginfo( strAllText, iLine, strFile ) );
}

void CHySelectException::GetFirstMessage( CString &strMsg, int &iLine, CString &strFile )
{
	strMsg = _T("");
	iLine = -1;
	strFile = _T("");

	m_vecMsgIter = m_vecMsg.begin();

	if( m_vecMsgIter != m_vecMsg.end() )
	{
		strMsg = m_vecMsgIter->m_strMsg;
		iLine = m_vecMsgIter->m_iLine;
		strFile = m_vecMsgIter->m_strFile;
	}
}

void CHySelectException::GetNextMessage( CString &strMsg, int &iLine, CString &strFile )
{
	strMsg = _T("");
	iLine = -1;
	strFile = _T("");

	if( m_vecMsgIter != m_vecMsg.end() )
	{
		m_vecMsgIter++;
	}

	if( m_vecMsgIter != m_vecMsg.end() )
	{
		strMsg = m_vecMsgIter->m_strMsg;
		iLine = m_vecMsgIter->m_iLine;
		strFile = m_vecMsgIter->m_strFile;
	}
}
