#pragma once


#include "utilities.h"


class CDocDb
{
public:
	struct treeNode
	{
		HTREEITEM nodeTreeItem;
		_string nodeId;
		_string nodeName;
		std::vector<_string> nodeText;
	};

// 	struct pair
// 	{
// 		const wchar_t strHYS[16];
// 		const wchar_t strDOC[16];
// 	};


	CDocDb();
	~CDocDb();

	void SetCsvPath( const TCHAR *csvPath );
	// HYS-1057: to set m_csvPathEn
	void SetCsvPathEn( const TCHAR *csvPath );
	_string GetPathFromArticleNumber( _string artNum, _string lang );
	_string GetPathFromHTreeItem( HTREEITEM item, _string lang );
	HTREEITEM GetHTreeItemFromArticleNumber( _string artNum );
	// HYS-1057: to return the index in docdb.csv of the node that 
	// contains article number (artNum)
	int GetHTreeItemIndexFromArticleNumber( _string artNum );
	size_t Size();
	_string GetLangDoc( _string lang );
	const _string &GetNameAt( int pos );
	_string &GetIdAt( int pos );
	const _string &GetTextAt( int pos, _string lang );
	HTREEITEM &GetTreeItemAt( int pos );

private:
	// *** METHODS ***
	// HYS-1057: Add a parameter csvPath to identify which docdb.csv is used
	void _LoadDocDb( const TCHAR *csvPath );
	int _FoundLangIndex( _string lang );

	// *** VARIABLES ***
	TCHAR *m_csvPath;
	TCHAR *m_csvPathEn;
	std::vector<treeNode> m_vNode;
	const _string m_strEmpty;

};
