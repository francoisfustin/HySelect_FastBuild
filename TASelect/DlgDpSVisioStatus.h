#pragma once


class CDlgDpSVisioBase;
class CDlgDpSVisioStatus : public CDlgDpSVisioBase
{
	DECLARE_DYNAMIC( CDlgDpSVisioStatus )

public:
	enum { IDD = IDD_DLGDPSVISIO_STATUS };

	CDlgDpSVisioStatus( CDlgDpSVisio *pParent );
	virtual ~CDlgDpSVisioStatus();

// Public methods.
public:
	// Overrides 'CDlgDpSVisioBase' public methods.
	virtual void SetParam( LPARAM lpParam ) {}
	virtual void OnDpSVisioConnected( CDlgDpSVisio::WhichDevice eWhichDevice );
	virtual void OnDpSVisioDisconnected( CDlgDpSVisio::WhichDevice eWhichDevice );

// Protected methods.
protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

// Private methods.
private:
	void _FillDpSVisioInformation( void );
	void _ResetDpSVisioInformation( void );

// Private variables.
private:
	HICON m_hIconStatusGreen;
	HICON m_hIconStatusRed;
	CStatic m_IconStatus;
};
