#pragma once


#include <direct.h>
#include <afxhtml.h>
#include "TASelectDoc.h"


class CRViewStartPage : public CHtmlView
{
	DECLARE_DYNCREATE( CRViewStartPage )

	static const int m_columns = 2;
	static const int m_lines = 16;

	// HYS-1149: Add 6-way valve
	// HYS-1936: TA-Smart Dp - 03 - Add the shortcut in the start page
	enum startLineType
	{
		slt_unknown = -1,
		slt_first = 0,
		slt_projectTitle = slt_first,
		slt_newCircuit,
		slt_browseFile,
		slt_previousFile1,
		slt_previousFile2,
		slt_previousFile3,
		slt_previousFile4,
		slt_toolsTitle,
		slt_viscosityCorrection,
		slt_pipesPressureDrop,
		slt_kvValues,
		slt_hydronicCalculator,
		slt_unitConversion,
		slt_taLink,
		slt_selectionTitle,
		slt_separators,
		slt_pressureMaintenance,
		slt_SafetyValve,
		slt_manualBalancing,
		slt_dpControllers,
		slt_balancingAndControl,
		slt_pressIndBalAndControl,
		slt_DpCBCV,
		slt_6WayValves,
		slt_controlValves,
		slt_smartControlValves,
		slt_smartDp,
		slt_radiatorsValves,
		slt_shutoffValves,
		slt_communicationTitle,
		slt_updateHyselect,
		slt_tascopeCommunication,
		slt_tacbiCommunication,
		slt_last
	};

	class startLine
	{
	public:
		startLineType lineType;
		bool title;
		int stringID;
		int iconID;
		int columnNum;
		int lineNum;
		CRect lineBoundingBox;

		startLine( startLineType _slt,
				   bool _title,
				   int _stringID,
				   int _iconID,
				   int _columnNum,
				   int _lineNum )
		{
			lineType = _slt;
			title = _title;
			stringID = _stringID;
			iconID = _iconID;
			columnNum = _columnNum;
			lineNum = _lineNum;
		}
	};

public:
	CRViewStartPage();
	virtual ~CRViewStartPage();

	// Overrides 'CView' method.
	virtual void OnInitialUpdate();

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnMouseMove( UINT nFlags, CPoint point );
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	bool _OpenPreviousFile(int fileID);

	afx_msg void OnSize(UINT nType, int cx, int cy);

	void _DrawLine( CDC &dc, startLine &sl, POINT &p );
	CString GetMRUPath(int sl);
	void _UpdateFont(CDC& dc);

private:
	void _InitLineVector();
	void _UnselectAll();
	CRect _GetWorkspace();
	CRect _GetColumn( unsigned int colNum );
	CRect _GetLine( unsigned int colNum, unsigned int lineNum );
	CRect _GetLine(startLineType sltType);
	CRect _GetSelectedLine();
	CRViewStartPage::startLineType _GetClickedLine(CPoint point);

	int m_xmargin;
	int m_ymargin;
	CRect m_rect;
	CFont *m_pFontTitle;
	CFont *m_pFontLink;
	std::vector<startLine> m_vStartLine;

};

