#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <tchar.h>
#include "stdafx.h"

#include "csvparser.h"


CsvParser *CsvParser_new( const char *filePath, const char *delimiter, int firstLineIsHeader )
{
	CsvParser *csvParser = ( CsvParser * )malloc( sizeof( CsvParser ) );

	if( filePath == NULL )
	{
		csvParser->filePath_ = NULL;
	}
	else
	{
		int filePathLen = strlen( filePath );
		csvParser->filePath_ = ( char * )malloc( ( filePathLen + 1 ) );
		strcpy_s( csvParser->filePath_, filePathLen + 1, filePath );
	}

	csvParser->firstLineIsHeader_ = firstLineIsHeader;
	csvParser->errMsg_ = NULL;

	if( delimiter == NULL )
	{
		csvParser->delimiter_ = ',';
	}
	else if( _CsvParser_delimiterIsAccepted( delimiter ) )
	{
		csvParser->delimiter_ = *delimiter;
	}
	else
	{
		csvParser->delimiter_ = '\0';
	}

	csvParser->header_ = NULL;
	csvParser->fileHandler_ = NULL;
	csvParser->fromString_ = 0;
	csvParser->csvString_ = NULL;
	csvParser->csvStringIter_ = 0;

	return csvParser;
}

CsvParser *CsvParser_new_from_string( const TCHAR *csvString, const char *delimiter, int firstLineIsHeader )
{
	CsvParser *csvParser = CsvParser_new( NULL, delimiter, firstLineIsHeader );
	csvParser->fromString_ = 1;

	if( csvString != NULL )
	{
		int csvStringLen = _tcslen( csvString );
		csvParser->csvString_ = ( TCHAR * )malloc( ( csvStringLen + 1 ) * sizeof( TCHAR ) );
		_tcscpy_s( csvParser->csvString_,csvStringLen + 1, csvString );
	}

	return csvParser;
}

void CsvParser_destroy( CsvParser *csvParser )
{
	if( csvParser == NULL )
	{
		return;
	}

	if( csvParser->filePath_ != NULL )
	{
		free( csvParser->filePath_ );
	}

	if( csvParser->errMsg_ != NULL )
	{
		free( csvParser->errMsg_ );
	}

	if( csvParser->fileHandler_ != NULL )
	{
		fclose( csvParser->fileHandler_ );
	}

	if( csvParser->header_ != NULL )
	{
		CsvParser_destroy_row( csvParser->header_ );
	}

	if( csvParser->csvString_ != NULL )
	{
		free( csvParser->csvString_ );
	}

	free( csvParser );
}

void CsvParser_destroy_row( CsvRow *csvRow )
{
	int i;

	for( i = 0 ; i < csvRow->numOfFields_ ; i++ )
	{
		free( csvRow->fields_[i] );
		csvRow->fields_[i] = NULL;
	}

	free( csvRow->fields_ );
	csvRow->fields_ = NULL;
	free( csvRow );
	csvRow = NULL;
}

CsvRow *CsvParser_getHeader( CsvParser *csvParser )
{
	if( ! csvParser->firstLineIsHeader_ )
	{
		_CsvParser_setErrorMessage( csvParser, "Cannot supply header, as current CsvParser object does not support header" );
		return NULL;
	}

	if( csvParser->header_ == NULL )
	{
		csvParser->header_ = _CsvParser_getRow( csvParser );
	}

	return csvParser->header_;
}

CsvRow *CsvParser_getRow( CsvParser *csvParser )
{
	if( csvParser->firstLineIsHeader_ && csvParser->header_ == NULL )
	{
		csvParser->header_ = _CsvParser_getRow( csvParser );
	}

	return _CsvParser_getRow( csvParser );
}

int CsvParser_getNumFields( CsvRow *csvRow )
{
	return csvRow->numOfFields_;
}

TCHAR **CsvParser_getFields( CsvRow *csvRow )
{
	return csvRow->fields_;
}

CsvRow *_CsvParser_getRow( CsvParser *csvParser )
{
	int accceptedFields = 64;
	int acceptedCharsInField = MAX_PATH;

	if( csvParser->filePath_ == NULL && ( ! csvParser->fromString_ ) )
	{
		_CsvParser_setErrorMessage( csvParser, "Supplied CSV file path is NULL" );
		return NULL;
	}

	if( csvParser->csvString_ == NULL && csvParser->fromString_ )
	{
		_CsvParser_setErrorMessage( csvParser, "Supplied CSV string is NULL" );
		return NULL;
	}

	if( csvParser->delimiter_ == '\0' )
	{
		_CsvParser_setErrorMessage( csvParser, "Supplied delimiter is not supported" );
		return NULL;
	}

	if( ! csvParser->fromString_ )
	{
		if( csvParser->fileHandler_ == NULL )
		{
			csvParser->fileHandler_ = fopen( csvParser->filePath_, "r" );

			if( csvParser->fileHandler_ == NULL )
			{
				int errorNum = errno;
				const char *errStr = strerror( errorNum );
				char *errMsg = ( char * )malloc( 1024 + strlen( errStr ) );
				strcpy_s( errMsg,1024 + strlen( errStr ), "" );
				sprintf( errMsg, "Error opening CSV file for reading: %s : %s", csvParser->filePath_, errStr );
				_CsvParser_setErrorMessage( csvParser, errMsg );
				free( errMsg );
				return NULL;
			}
		}
	}
	else
	{
		// Avoid BOM
		if( 0 == csvParser->csvStringIter_ && 0xfeff == csvParser->csvString_[csvParser->csvStringIter_] )
		{
			csvParser->csvStringIter_++;
		}
	}

	CsvRow *csvRow = ( CsvRow * )malloc( sizeof( CsvRow ) );
	csvRow->fields_ = ( TCHAR ** )malloc( accceptedFields * sizeof( TCHAR * ) );
	memset(csvRow->fields_, 0, accceptedFields * sizeof(TCHAR *));
	csvRow->numOfFields_ = 0;
	int fieldIter = 0;
	TCHAR *currField = ( TCHAR * )malloc( acceptedCharsInField * sizeof( TCHAR ) );
	currField[0] = _T('\0');
	int inside_complex_field = 0;
	int currFieldCharIter = 0;
	int seriesOfQuotesLength = 0;
	int lastCharIsQuote = 0;
	int isEndOfFile = 0;

	while( 1 )
	{
		TCHAR currChar = ( csvParser->fromString_ ) ? csvParser->csvString_[csvParser->csvStringIter_] : fgetc( csvParser->fileHandler_ );
		csvParser->csvStringIter_++;
		int endOfFileIndicator;

		if( csvParser->fromString_ )
		{
			endOfFileIndicator = ( currChar == _T('\0') );
		}
		else
		{
			endOfFileIndicator = feof( csvParser->fileHandler_ );
		}

		if( endOfFileIndicator )
		{
			if( currFieldCharIter == 0 && fieldIter == 0 )
			{
				_CsvParser_setErrorMessage( csvParser, "Reached EOF" );
				CsvParser_destroy_row( csvRow );
				free( currField );
				return NULL;
			}

			csvParser->csvStringIter_--;
			currChar = _T( '\n' );
			isEndOfFile	= 1;
		}

		if( currChar == _T( '\r' ) && inside_complex_field == 0 )
		{
			continue;
		}

		if( currFieldCharIter == 0  && ! lastCharIsQuote )
		{
			if( currChar == _T( '\"' ) )
			{
				inside_complex_field = 1;
				lastCharIsQuote = 1;
				continue;
			}
		}
		else if( currChar == _T( '\"' ) )
		{
			seriesOfQuotesLength++;
			inside_complex_field = ( seriesOfQuotesLength % 2 == 0 );

			if( inside_complex_field )
			{
				currFieldCharIter--;
			}
		}
		else
		{
			seriesOfQuotesLength = 0;
		}

		if( isEndOfFile || ( ( currChar == csvParser->delimiter_ || currChar == _T('\n') ) && ! inside_complex_field ) )
		{
			currField[lastCharIsQuote ? currFieldCharIter - 1 : currFieldCharIter] = _T( '\0' );
			csvRow->fields_[fieldIter] = ( TCHAR * )malloc( ( currFieldCharIter + 1 ) * sizeof( TCHAR ) );
			_tcscpy_s( csvRow->fields_[fieldIter],  currFieldCharIter + 1, currField );
			free( currField );
			csvRow->numOfFields_++;

			if( currChar == '\n' )
			{
				return csvRow;
			}

			acceptedCharsInField = MAX_PATH;
			currField = ( TCHAR * )malloc( acceptedCharsInField * sizeof( TCHAR ) );
			currFieldCharIter = 0;
			fieldIter++;
			inside_complex_field = 0;
		}
		else
		{
			currField[currFieldCharIter] = currChar;
			currFieldCharIter++;

			if( currFieldCharIter == acceptedCharsInField - 1 )
			{
				acceptedCharsInField *= 2;
				TCHAR* newField = NULL;
				newField = ( TCHAR * )realloc( currField, acceptedCharsInField * sizeof( TCHAR ) );
				
				if (newField != NULL)
				{
					currField = newField;
				}
				else
				{
					_CsvParser_setErrorMessage(csvParser, "Realloc failure");
					CsvParser_destroy_row(csvRow);
					free(currField);
					return NULL;
				}
			}
		}

		lastCharIsQuote = ( currChar == _T('\"') ) ? 1 : 0;
	}
}

int _CsvParser_delimiterIsAccepted( const char *delimiter )
{
	char actualDelimiter = *delimiter;

	if( actualDelimiter == '\n' || actualDelimiter == '\r' || actualDelimiter == '\0' ||
		actualDelimiter == '\"' )
	{
		return 0;
	}

	return 1;
}

void _CsvParser_setErrorMessage( CsvParser *csvParser, const char *errorMessage )
{
	if( csvParser->errMsg_ != NULL )
	{
		free( csvParser->errMsg_ );
	}

	int errMsgLen = strlen( errorMessage );
	csvParser->errMsg_ = ( char * )malloc( errMsgLen + 1 );
	strcpy_s( csvParser->errMsg_,errMsgLen + 1, errorMessage );
}

const char *CsvParser_getErrorMessage( CsvParser *csvParser )
{
	return csvParser->errMsg_;
}
