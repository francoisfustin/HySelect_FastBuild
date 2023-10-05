#pragma once


class CDlgDpSVisioUpgradeFinished : public CDlgDpSVisioBase
{
	DECLARE_DYNAMIC( CDlgDpSVisioUpgradeFinished )

public:
	enum { IDD = IDD_DLGDPSVISIO_UPGRADEFINISHED };

	CDlgDpSVisioUpgradeFinished( CDlgDpSVisio *pParent );
	virtual ~CDlgDpSVisioUpgradeFinished();

// Public methods.
public:
	// Overrides 'CDlgDpSVisioBase' public methods.
	virtual void SetParam( LPARAM lpParam );

// Protected methods.
protected:
	virtual void DoDataExchange( CDataExchange *pDX );
	virtual BOOL OnInitDialog();

// Private methods.
private:

// Private variables.
private:
	CDlgDpSVisio::UpgradeStatus m_rUpgradeStatus;
	CStatic m_BitmapUpgradeStatus;
	HICON m_hIconUpgradeStatusOK;
	HICON m_hIconUpgradeStatusFailed;
};
