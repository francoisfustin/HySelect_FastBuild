#pragma once


#include "afxwin.h"
#include "MyToolTipCtrl.h"
#include "DlgOutput.h"
#include "tchart.h"

#define DLGPMOID	0x02
#define DLGPMMARGIN 10

class CDlgOutput;
class CDlgPMGraphsOutput : public CDlgOutput
{
	DECLARE_DYNAMIC( CDlgPMGraphsOutput )

public:
	CDlgPMGraphsOutput();
	virtual ~CDlgPMGraphsOutput();

private:
	class CPMMessage;
public:
	class CPMInterface : public CBaseInterface
	{
	public:
		CPMInterface();
		virtual ~CPMInterface() {}

		virtual bool Init( void );

		bool AddBitmap( HBITMAP hBitmap );
		bool AddTChartDialog( CDialogEx *pclTChartDialog );
		bool GetCurrentHeight( long *plHeight );
		CWnd *GetCWndPMGraphsOutput( void );

	private:
		CDlgPMGraphsOutput *m_pDlgPMGraphsOutput;
	};

	virtual bool ClearOutput( bool fAlsoHeader = false, int iOutputID = -1 );

	bool AddBitmap( HBITMAP hBitmap );
	bool AddTChartDialog( CDialogEx *pclTChartDialog );
	long GetCurrentHeight( void );
	// HYS-1348
	CDialogEx* GetTChartDialog() { return m_pclTChartDialog; }

protected:
	virtual BOOL OnInitDialog();

	// Overrides 'CDlgOutput' protected virtual methods.
	virtual void OnDlgOutputAdded( int iOutputID , CDlgOutputSpreadContainer *pclDlgOutputSpreadContainer );


	// Private variables.
private:
	CStatic *m_pclBitmapContainer;
	HBITMAP m_hBitmap;
	CTChart *m_pclTChart;
	CDialogEx *m_pclTChartDialog;

	int m_iCurrentX;
	long m_lCurrentHeight;
};
