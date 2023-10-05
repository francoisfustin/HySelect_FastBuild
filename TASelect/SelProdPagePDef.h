#pragma once


class CSelProdPagePDef : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_PartDef = 1,
	};

	CSelProdPagePDef( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPagePDef(void) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector& vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABPARTDEF; }

// Protected variables.
protected:
	CData *m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription* pclSheetDescription );
	long _FillRow( CSheetDescription* pclSheetDescription, long lRow, CDS_DirectSel* pclPartDef );
	long _FillRow( CSheetDescription* pclSheetDescription, long lRow, CDS_HydroMod* pHM );
};
