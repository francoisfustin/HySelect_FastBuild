#pragma once


class CSelProdPageAirVentSep : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_AirVentSep = 1,
	};

	CSelProdPageAirVentSep( CArticleGroupList* pclArticleGroupList );
	virtual ~CSelProdPageAirVentSep(void) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector& vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABAIRVENTSEPARATOR; }

// Protected variables.
protected:
	CData* m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription* pclSheetDescription );
	long _FillRowAirVent( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelAirVentSeparator* pclSSelAirVentSep, CDB_AirVent* pclAirVent );
	long _FillRowSeparator( CSheetDescription* pclSheetDescription, long lRow, CDS_SSelAirVentSeparator* pclSSelAirVentSep, CDB_Separator* pclSeparator );
	long _FillRowSeparator( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod* pclHM );
	long _FillRowAirVentProduct( CSheetDescription* pclSheetDescription, long lRow, CDB_AirVent* pclAirVent, int iQuantity );
	long _FillRowSeparatorProduct( CSheetDescription* pclSheetDescription, long lRow, CDB_Separator* pclSeparator, int iQuantity );
	long _FillRowSeparatorInfo( CSheetDescription* pclSheetDescription, long lRow, double dQ, double dDp, CString strPowerDt );
	void _AddArticleList( CDB_TAProduct* pclTAP, int iQuantity );
	void _AddArticleList( CDS_SSelAirVentSeparator* pclSSelAirVentSep );
};
