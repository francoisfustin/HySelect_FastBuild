#pragma once


class CSelProdPageTCFloorHeatingControl : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_FloorHeatingControl = 1
	};

	CSelProdPageTCFloorHeatingControl( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageTCFloorHeatingControl(void) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector &vecHMList );
	virtual bool Init( bool bResetOrder = false, bool bPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABFLOORHEATINGCONTROL; }

// Protected variables.
protected:
	CData *m_pSelected;

// Private methods.
private:
	void _InitColHeader( CSheetDescription *pclSheetDescription );

	long _FillRowFloorHeatingManifold( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelFloorHeatingManifold *pclSSelFloorHeatingManifold, CDB_FloorHeatingManifold *pclFloorHeatingManifold );
	long _FillRowFloorHeatingManifoldProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_FloorHeatingManifold *pclFloorHeatingManifold, int iQuantity );

	long _FillRowFloorHeatingValve( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelFloorHeatingValve *pclSSelFloorHeatingValve, CDB_FloorHeatingValve *pclFloorHeatingValve );
	long _FillRowFloorHeatingValveProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_FloorHeatingValve *pclFloorHeatingValve, int iQuantity );

	long _FillRowFloorHeatingController( CSheetDescription *pclSheetDescription, long lRow, CDS_SSelFloorHeatingController *pclSSelFloorHeatingController, CDB_FloorHeatingController *pclFloorHeatingController );
	long _FillRowFloorHeatingControllerProduct( CSheetDescription *pclSheetDescription, long lRow, CDB_FloorHeatingController *pclFloorHeatingController, int iQuantity );
	
	void _AddArticleList( CDB_Product *pclProduct, int iQuantity );
	void _AddArticleList( CDS_SSel *pclSSelFloorHeatingManifold );
};
