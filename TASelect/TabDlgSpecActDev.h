#pragma once


#include "DialogExt.h"
#include <sqlite3.h> 

class CTabDlgSpecActDev : public CDialogExt
{
public:
	CTabDlgSpecActDev( CWnd *pParent = NULL );

	enum { IDD = IDD_TABSPECACT_DEV };

	static int cbFillVesselFamiliesMap( void *NotUsed, int argc, char **argv, char **azColName );
	static int cbFillVesselMap( void *pthis, int argc, char **argv, char **azColName );
	static bool SortVessels(CDB_Vessel *pVssl1, CDB_Vessel *pVssl2);

	static bool SortTecboxes(CDB_TecBox *pTB1, CDB_TecBox *pTB2);
	static int cbFillTecboxFamiliesMap(void *pthis, int argc, char **argv, char **azColName);
	static int cbFillTecBoxesMap(void *pthis, int argc, char **argv, char **azColName);


protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();
	virtual afx_msg void OnBnClickedRun();
	virtual afx_msg void OnBnClickedClear();

	void ComboFctExtractPipeSeries();
	void ComboFctTestHydronicScheme();
	void ComboFctGenerateTAPTxt();
	void ComboFctExtractAllArticleNumbers();
	void ComboFctPMExtractToSQL();
	void ComboFctShowHydronicSchView();
	void ComboFctButtonSearchDBObject();
	void ComboFctHyToolsPMCompare();
	void ComboFctReformatFilterTabFiles();
	void ComboFctExtractAllPMProduct();
	void ComboFctMarkAllProductAsDeleted();
	void ComboFctMarkAllProductAsUndeleted();
	void ComboFctTest6WayValveSelection();
	void ComboFctTestSTAVSelection();
	void ComboFctComputeCRC();
	void ComboFctExtractArticleWithoutPrice();

// Private methods.
private:
	void _ExtractPipeSeries( CFileTxt *pOutf, CTable *pTab = NULL );
	void _ExtractPipes( CFileTxt *pOutf, CTable *pTab = NULL );
	void _ExtractCDBTAP( void );
	void _TestArticleNumberDuplication();
	void _ExtractUniversalValves( void );
	void _ExtractKvVsOpeningForMVValves( void );
	void _ExtractFlowAndDpmin( void );
	void _ExtractKvVsOpeningForTAFUSIONC( void );
	void _CreateSQLTable( sqlite3 *pdb, char *sql );
	void _ExtractAllArticleNumbersAndVVS();
	
	void _SQLiteError( sqlite3 *pSQLD, CString strErr );
	CStringA _NormalizeName( CStringA strA );
	bool _ExtractVessels( sqlite3 *pDb );
	bool _ExtractTecBoxes( sqlite3 *pDb );
	void _ReplaceAll( std::string &str, const std::string &from, const std::string &to );
	void _PrepareInsertQuery( std::string &str );
	CStringA _GetInstallationLayout( int iInstallationLayout );

	//int _GetConnection(CDB_Connect *pConnect);
	int _GetConnectionSizes(sqlite3 *pDb, CDB_Vessel* pVssl);

// Private variables.
private:
	CComboBox m_clComboFunctions;

	typedef void (CTabDlgSpecActDev::*pFunction)();
	class CComboInfo
	{
	public:
		CComboInfo( CString strText, pFunction pComboFct ) { m_strComboText = strText; m_pFunction = pComboFct; }
		CString m_strComboText;
		pFunction m_pFunction;
	};
	std::vector<CComboInfo> m_vecComboInfos;

	CListBox m_List;
	CButton m_clButtonClear;

	CTemplateMap < CString, int> m_mapPKVessel;
	CTemplateMap < CString, int> m_mapPKVesselFamilyName;
	CTemplateMap < int, CString> m_mapPKPrimVslIDSecVsl;

	CTemplateMap < CString, int> m_mapPKTecboxFamilyName;
	CTemplateMap < CString, int> m_mapPKTecbox;

	CTable *m_pTab;
};
