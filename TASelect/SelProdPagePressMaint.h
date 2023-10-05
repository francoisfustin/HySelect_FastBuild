#pragma once


class CSelProdPagePressMaint : public CSelProdPageBase
{
public:
	// 'PM' is for 'Pressure Maintenance'.
	// 'CPM' is for 'Computed Pressure Maintenance'.
	enum SheetDescription
	{
		SD_PMPageTitle			= 0x01000000,
		SD_CPMInputOutput		= 0x02000000,
		SD_CPMProduct			= 0x04000000,
		SD_DirSelPressMaint		= 0x08000000,
		SD_PlenoVentoIndSel		= 0x10000000,
		SD_Graphs				= 0x20000000,
		SD_Remarks				= 0x00000001,
	};

	// 'PMPT' is for 'Pressure Maintenance Page Title'.
	enum RowDescription_PMPT
	{
		RD_PMPT_FirstRow = 1,
		RD_PMPT_PageName
	};

	enum ColumnDescription_PMPT
	{
		CD_PMPT_Header = 1,
		CD_PMPT_Title,
		CD_PMPT_Footer,
		CD_PMPT_LastColumn,
	};

	// 'CPMIO' is for 'Computed Pressure Maintenance Input and Output (Calculated data)'.
	enum RowDescription_CPMIO
	{
		RD_CPMIO_FirstRow = 1,
		RD_CPMIO_GroupName,					// 'Calculated pressure maintenance selection'.
		RD_CPMIO_TitleIO,					// 'Input data' and 'Calculated data'.
		RD_CPMIO_ColName,
		RD_CPMIO_FirstAvailRow
	};

	enum ColumnDescription_CPMIO
	{
		CD_CPMIO_Header = 1,
		CD_CPMIO_IDescription1,
		CD_CPMIO_IDescription2,
		CD_CPMIO_IValue,
		CD_CPMIO_ODescription,
		CD_CPMIO_OValue,
		CD_CPMIO_Footer,
		CD_CPMIO_LastColumn
	};

	enum DefaultColumnWidth_CPMIO
	{
		DCW_CPMIO_Description1	= 10,
		DCW_CPMIO_Description2	= 25,
		DCW_CPMIO_Value			= 15
	};

	// 'CPMP' is for 'Computed Pressure Maintenance Product'.
	enum RowDescription_CPMP
	{
		RD_CPMP_Title = 1,					// 'Product'.
		RD_CPMP_ColName,
		RD_CPMP_FirstAvailRow
	};

	enum ColumnDescription_CPMP
	{
		CD_CPMP_Header = 1,
		CD_CPMP_Reference1,
		CD_CPMP_Reference2,
		CD_CPMP_Product,
		CD_CPMP_ArticleNumber,
		CD_CPMP_Quantity,
		CD_CPMP_UnitPrice,
		CD_CPMP_TotalPrice,
		CD_CPMP_Remark,
		CD_CPMP_Footer,
		CD_CPMP_Pointer
	};

	// 'DSPM' is for 'Direct Selection Pressure Maintenance'.
	enum RowDescription_DSPM
	{
		RD_DSPM_FirstRow = 1,
		RD_DSPM_GroupName,					// 'Calculated pressure maintenance selection'.
		RD_DSPM_ColName,
		RD_DSPM_FirstAvailRow
	};

	// 'PMR' is for 'Pressure Maintenance Remarks'.
	enum RowDescription_PMR
	{
		RD_PMR_FirstRow = 1
	};

	enum ColumnDescription_PMR
	{
		CD_PMR_Header = 1,
		CD_PMR_Title,
		CD_PMR_Footer,
		CD_PMR_LastColumn,
	};

	// 'G' is for 'Graphs'.
	enum RowDescription_G
	{
		RD_G_Header = 1,
		RD_G_Graphs
	};

	enum ColumnDescription_G
	{
		CD_G_Header = 1,
		CD_G_Separator1,
		CD_G_Bitmap1,
		CD_G_Separator2,
		CD_G_Bitmap2,
		CD_G_Separator3,
		CD_G_Footer,
		CD_G_LastColumn
	};

	CSelProdPagePressMaint( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPagePressMaint( void ) {}

	// Overrides 'CSelProdPageBase' public virtual methods.
	virtual void Reset( void );
	virtual bool PreInit( HMvector &vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void )
	{
		return HasSomethingToDisplay();
	}
	virtual UINT GetTabTitleID( void )
	{
		return IDS_SSHEETSELPROD_TABPRESSMAINT;
	}
	virtual long FillAccessory( CSheetDescription *pclSheetDescription, long lRow, CDB_Product *pAccessory, bool fWithArticleNumber, int iQuantity,
								CProductParam *pclProductParam = NULL );

	// Protected variables.
protected:
	CData *m_pSelected;

	// Private methods.
private:
	void _FillPMTitle( void );

	void _FillComputedInputOutput( CDS_SSelPMaint *pclSSelPressMaint );
	void _FillComputedInputData( CSheetDescription *pclSDInputOutput, CDS_SSelPMaint *pclSSelPressMaint );
	void _FillComputedOutputData( CSheetDescription *pclSDInputOutput, CDS_SSelPMaint *pclSSelPressMaint );

	void _FillComputedProduct( CDS_SSelPMaint *pclSSelPressMaint );

	void _InitDirectSel( void );
	void _FillDirectSel( CDS_SSelPMaint *pclSSelPressMaint, bool fFirstPass );

	void _InitPlenoVentoIndSel( void );
	void _FillPlenoVentoIndSel( CDS_SSelPMaint *pclSSelPressMaint, bool fFirstPass );

	enum
	{
		NoTitle,
		FillStaticoVessel,
		FillPrimaryVessel,
		FillSecondaryVessel,
		FillIntermediateVessel
	};
	// Param: 'fSelIncludeColTitleIfNeeded'. If 'false', we don't include column title with the selection. If 'true', we include the column title in the selection
	//        if the concerned product is the first one displayed.
	long _FillRowVesselProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_Vessel *pclVessel, CDS_SSelPMaint *pclSSelPressMaint, int iQuantity,
			bool bIntegratedBuffer = false, CProductParam *pclProductParam = NULL );

	long _FillRowCprssoTrsfroProduct( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelPMaint *pclSSelPMaint, bool fSelIncludeColTitleIfNeeded = false,
			CProductParam *pclProductParam = NULL );

	// Returns the last row written and not the next available row.
	long _FillRowTecBoxPlenoProduct( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelPMaint *pclSSelPMaint, bool fSelIncludeColTitleIfNeeded = false,
			bool fAddRemarks = false, CProductParam *pclProductParam = NULL );

	long _FillRowTBPlenoProtectionProduct( CSheetDescription * pclSheetDescription, long lRow, CDS_SSelPMaint * pclSSelPMaint, bool fSelIncludeColTitleIfNeeded = false, 
			CProductParam * pclProductParam = NULL );

	// Returns the last row written and not the next available row.
	long _FillRowPlenoRefillProduct( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelPMaint *pclSSelPMaint, bool fSelIncludeColTitleIfNeeded = false,
			bool fAddRemarks = false, CProductParam *pclProductParam = NULL );

	// Returns the last row written and not the next available row.
	long _FillRowTecBoxVentoProduct( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelPMaint *pclSSelPMaint, bool fSelIncludeColTitleIfNeeded = false,
			CProductParam *pclProductParam = NULL );

	void _FillRowGen( CSheetDescription *pclSheetDescription, CDS_SSelPMaint *pclSSelPMaint, long lFirstRow, long lLastRow );

	void _FillQtyPriceCol( CSheetDescription *pclSheetDescription, long lFirstRow, CData *pclProduct, int iQuantity );

	void _FillRemarks();

	void _AddArticleList( CDS_SSelPMaint *pclSSelPressMaint );

	long _AddSpecialStaticText( CSheetDescription *pclSheetDescription, long lColumn, long lRow, CString strText, CString strIndex, CString strValue );

	void _FillCurves( CDS_SSelPMaint *pclSSelPressMaint );

	// Private variables.
private:
	enum PMColumnWidth
	{
		PMCW_Header			= 2,
		PMCW_Reference1		= 10,
		PMCW_Reference2		= 10,
		PMCW_Product		= 34,
		PMCW_ArticleNumber	= 10,
		PMCW_Quantity		= 5,
		PMCW_UnitPrice		= 7,
		PMCW_TotalPrice		= 7,
		PMCW_Remark			= 2,
		PMCW_Footer			= 2,
		PMCW_Pointer		= 0
	};

	std::vector<CDS_SSelPMaint *> m_vecComputedPressMaint;
	std::vector<CDS_SSelPMaint *> m_vecDirSelPressMaint;
	CProductParam m_clCPMIOParam;		// Computed pressurization maintenance input/output.
	CProductParam m_clCPMPParam;		// Computed pressurization maintenance product.
	CProductParam m_clCPMGParam;		// Graphs for pressurization maintenance product.
	CProductParam m_clDSPMParam;		// Direct selection of pressurization maintenance product.
	CProductParam m_clPVISParam;		// Pleno/Vento from individual selection.
	UINT m_uiSDComputedIOCount;
	UINT m_uiSDComputedPCount;

	enum GraphColumnWidth
	{
		GCW_Header = 1,
		GCW_Separator1 = 1,
		GCW_Bitmap1 = 100,
		GCW_Separator2 = 1,
		GCW_Bitmap2 = 75,
		GCW_Separator3 = 1,
		GCW_Footer = 1,
	};
	UINT m_uiSDGraphsCount;

	typedef std::vector<HANDLE> vecHandle;
	typedef vecHandle::iterator vecHandleIter;
	vecHandle m_vecImageList;
	vecHandle m_vecImageSelectedList;
};
