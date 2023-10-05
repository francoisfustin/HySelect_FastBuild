#pragma once


class CSelProdPagePipeList : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_PipeList = 1,
	};

	CSelProdPagePipeList( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPagePipeList(void) {}
		
	virtual void Reset( void );
	virtual bool PreInit( HMvector& vecHMList );
	virtual bool Init( bool fResetOrder = false, bool fPrint = false );
	virtual bool HasSomethingToDisplay( void );
	virtual bool HasSomethingToPrint( void ) { return HasSomethingToDisplay(); }
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABPIPELIST; }

// Protected variables.
protected:
	typedef struct _PipeInfo
	{
		CDB_Pipe* m_pclPipe;
		double m_dLength;
		struct _PipeInfo(){ m_pclPipe = NULL; m_dLength = 0.0; }
		struct _PipeInfo( CDB_Pipe* pclPipe, double dLength ) { m_pclPipe = pclPipe; m_dLength = dLength; }
	}PipeInfo;
	std::map<CString, PipeInfo> m_mapPipes;
	
	enum ColumnDescriptionPipeList
	{
		Header		= 1,
		PipeSerie	= 2,
		PipeSize	= 3,
		TotalLength	= 4,
		FluidVolume	= 5,
		Footer		= 6,
		Pointer		= 7
	};

// Private methods.
private:
	void _FillPipeList( CTable* pTab );
	void _InitColHeader( CSheetDescription* pclSheetDescription );
	long _FillRow( CSheetDescription* pclSheetDescription, long lRow, CTable* pTabSerie, CDB_Pipe* pPipe, double dLength, double &dVolumeTot );
};
