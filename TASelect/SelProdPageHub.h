#pragma once


class CSelProdPageHub : public CSelProdPageBase
{
public:
	enum SheetDescription
	{
		SD_Hub = 1,
	};

	CSelProdPageHub( CArticleGroupList *pclArticleGroupList );
	virtual ~CSelProdPageHub(void) {}
		
	virtual void Reset( void );
	virtual bool HasSomethingToDisplay( HMvector& vecHMList );
	virtual UINT GetTabTitleID( void ) { return IDS_SSHEETSELPROD_TABHUB; }

	bool Init( bool fResetOrder = false, bool fPrint = false );

// Private methods.
private:

// Protected variables.
protected:
	CData *m_pSelected;
};
