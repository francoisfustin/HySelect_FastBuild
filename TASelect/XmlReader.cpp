#include "StdAfx.h"


#include "XmlReader.h"

CXmlReader::CXmlReader()
{
	m_pBuf = NULL;
	m_pPosBuf = NULL;
}

CXmlReader::~CXmlReader()
{
	if( NULL != m_pBuf )
	{
		delete []m_pBuf;
	}

	m_inpf.close();
}

int	CXmlReader::Open( std::string filename )
{
	m_inpf.open( filename.c_str(), std::ifstream::in | std::ifstream::binary );
	m_pBuf = NULL;
	m_pPosBuf = NULL;

	if( !( m_inpf.rdstate() & std::ifstream::failbit ) )
	{
		// Get length of file.
		m_inpf.seekg( 0, std::ios::end );
		int Length = ( int )m_inpf.tellg();
		m_inpf.seekg( 0, std::ios::beg );

		// Allocate memory.
		m_pBuf = new char [Length + 2];
		m_pPosBuf = ( WCHAR * )m_pBuf;
		
		// Read data as a block.
		m_inpf.read( m_pBuf, Length );
		m_pBuf[Length] = 0;
		m_pBuf[Length + 1] = 0;
		m_inpf.close();
		return Length;
	}

	return 0;
}

void CXmlReader::Close()
{
	m_inpf.close();
}

WCHAR *CXmlReader::GetStringTo( WCHAR wcMark )
{
	const int maxlength = 1024;
	static WCHAR buf[maxlength];
	WCHAR *pb = buf;
	int iLen = 0;
	buf[0] = 0;

	while( _T('\0') != *m_pPosBuf )
	{
		*pb = '\0';

		if( *m_pPosBuf == wcMark || *m_pPosBuf == 0 )
		{
			return buf;
		}
		else
		{
			*pb = *m_pPosBuf;
		}

		++iLen;
		ASSERT( iLen < maxlength );
		++m_pPosBuf;
		++pb;
	};

	return buf;
}

// Return next tag found.
WCHAR *CXmlReader::GetNextElement()
{
	// LookUp Entry field
	static WCHAR wc = '\0';

	do
	{
		GetStringTo( _T('<') );			// Found tag entry.
		
		// 2016-11-28: at the end of the file, we have no more new tag '<blabla>'. For example, 'TASTips.xml' is finished
		//             with </string> and </NewDataSet>. In that case, the 'GetStringTo' method will reach end of file (the 'm_pPosBuf' 
		//             points on the last double '\0'). If we increment one more here, the result must be unpredictable in regards to the
		//             memory state after the end of the buffer where has been saved the file.
		if( _T('\0') != *m_pPosBuf )
		{
			++m_pPosBuf;
		}

		if( _T('\0') == *m_pPosBuf )
		{
			return &wc;
		}
	}
	while( _T('/') == *m_pPosBuf );

	WCHAR *pBuf = m_pPosBuf;
	GetStringTo( _T('>') );			// Found tag out.
	*m_pPosBuf = 0;
	m_pPosBuf++;

	return pBuf;
}

// return text included between <BaliseID> and <\BaliseID>
WCHAR *CXmlReader::GetElementContent( WCHAR *pElement, WCHAR *pEndTable )
{
	WCHAR *pBalFound = NULL;

	if( _T('\0') != *pElement )
	{
		do
		{
			pBalFound = GetNextElement();

			if( 0 == wcscmp( pBalFound, pElement ) )
			{
				break;
			}

			if( NULL != pEndTable && 0 == wcscmp( pBalFound, pEndTable ) )
			{
				return NULL;
			}
		}
		while( _T('\0') != *pBalFound );
	}

	// Extract string.
	return GetStringTo( _T('<') );			// Found Balise out
}

WCHAR CXmlReader::ProcessXMLSpecChar( WCHAR **pos )
{
	static WCHAR amp[] = {'a', 'm', 'p', '\0'};
	static WCHAR lt[] = {'l', 't', '\0'};
	static WCHAR gt[] = {'g', 't', '\0'};
	static WCHAR dquot[] = {'q', 'u', 'o', 't', '\0'};
	static WCHAR quot[] = {'a', 'p', 'o', 's', '\0'};
	WCHAR XMLspec[6];
	( *pos )++;
	int i = 0;

	for( ; *pos && **pos && **pos != ';' && i < 5; i++, ( *pos )++ )
	{
		XMLspec[i] = **pos;
	}

	XMLspec[i] = 0;

	if( _T(';') == **pos )	// End of special character detected
	{
		if( 0 == wcscmp( XMLspec, amp ) )
		{
			return ( _T('&') );
		}
		else if( 0 == wcscmp( XMLspec, lt ) )
		{
			return ( _T('<') );
		}
		else if( 0 == wcscmp( XMLspec, gt ) )
		{
			return ( _T('>') );
		}
		else if( 0 == wcscmp( XMLspec, dquot ) )
		{
			return ( _T('\"') );
		}
		else if( 0 == wcscmp( XMLspec, quot ) )
		{
			return ( _T('\'') );
		}
	}

	// ASSERT(0);
	return _T(' ');
}

void CXmlReader::ProcessNewLineChar( WCHAR **pos, WCHAR **posBuf )
{
	if( _T('\\') == **pos )
	{
		( *pos )++;

		if( _T('n') == ( **pos ) )
		{
			**posBuf = _T('\n');
			( *posBuf )++;
		}
		else if( _T('r') == ( **pos ) )
		{
			**posBuf = _T('\r');
			( *posBuf )++;
		}
		else if( _T('t') == ( **pos ) )
		{
			**posBuf = _T('\t');
			( *posBuf )++;
		}
		else
		{
			ASSERT( 0 );
			( *pos )--;
			( **posBuf ) = ( **pos );
			( *posBuf )++;
		}
	}
	else
	{
		( **posBuf ) = ( **pos );
		( *posBuf )++;
	}
}
