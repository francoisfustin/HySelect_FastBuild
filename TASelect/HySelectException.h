#pragma once

#include <vector>


class CHySelectException
{
public:
	CHySelectException();
	CHySelectException( CString strMessage, int iLine, CStringA strFile, ... );
	CHySelectException( CHySelectException &clHySelectException );
	CHySelectException( CHySelectException &clHySelectException, CString strMessage, int iLine, CStringA strFile, ... );
	CHySelectException( CHySelectException const &clHySelectException );
	CHySelectException &operator=( const CHySelectException &clHySelectException );
	virtual ~CHySelectException() {}

	typedef struct _msginfo
	{
		struct _msginfo( CString strMsg, int iLine, CStringA strFile ) { m_strMsg = strMsg, m_iLine = iLine; USES_CONVERSION; m_strFile = A2T( strFile ); }
		CString m_strMsg;
		int m_iLine;
		CString m_strFile;
	}msginfo;

	void ClearMessages();
	void AddMessage( CString strMessage, int iLine, CStringA strFile, ... );
	std::vector<msginfo> const *GetMessageList() const { return &m_vecMsg; }
	void GetFirstMessage( CString &strMsg, int &iLine, CString &strFile );
	void GetNextMessage( CString &strMsg, int &iLine, CString &strFile );

private:

	std::vector<msginfo> m_vecMsg;
	std::vector<msginfo>::iterator m_vecMsgIter;
};
