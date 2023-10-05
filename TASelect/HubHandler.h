#pragma once
#include "EnBitmapRow.h"
#include "HMInclude.h"

// Schemes Resource ID for HUB
#define GIF_STATION_PLUG					1100
#define GIF_END								1300
#define GIF_HUB_OFFSET_MVSECONDARY			0x1
#define GIF_STATION_OFFSET_EXT				0x1		// offset for extension Pipe
#define GIF_STATION_OFFSET_TSE				0x2		// offset for TSE Actuator
#define GIF_STATION_OFFSET_TSEM				0x4		// offset for TSEM Actuator

#define GIF_BOX_4_ARROWS					5000
#define GIF_BOX_6_ARROWS					5001
#define GIF_BOX_8_ARROWS					5002

#define GIF_VERTICALWHITELINE				7000
#define GIF_DIGIT1					        7001
#define GIF_DIGIT2					        7002
#define GIF_DIGIT3					        7003
#define GIF_DIGIT4					        7004
#define GIF_DIGIT5					        7005
#define GIF_DIGIT6							7006
#define GIF_DIGIT7							7007
#define GIF_DIGIT8							7008
#define GIF_LABELTAHUB						7010

class CHubHandler
{

public:
	CHubHandler( void );
	~CHubHandler( void );

	// Attach HubHandler to an existing Hub, return IDPTR of Parent Table
	IDPTR Attach( CString strHubID );
	
	// 
	void Detach() { m_HubIDPtr = _NULL_IDPTR; }
	
	// Return ID of the current attached Hub
	IDPTR GetHubIDPtr() { return m_HubIDPtr; }
	
	// Create a new Hub in Piping Tab and attach it
	// Return IDPTR of created Hub
	// Water char is set by default to PARAM_WC
	IDPTR CreateHub(	CString strTabID, CDS_HmHub::Application Application, UINT uiStations, CDS_HmHub::MainFeedPos MainFeedPos, 
						bool fBypass, CString strBalTypeID, IDPTR StationsConnectIDPtr,	CDS_HmHub::SelMode SelMode, bool fNew = true, CString strID_WC = CString( _T("PARAM_WC") ) );

	IDPTR GetStationIDPTR( int iStationPosition );

	// Copy current hub into the pTab, return IDPTR of copied Hub
	IDPTR Copy( CString ID );
	IDPTR Copy( CTable *pTab );
	
	// Delete attached hub
	void Delete( IDPTR IDPtr );
	
	// Return Hub Image
	CEnBitmap *GetHubImg() { return m_EnBmp; }
	bool GetHubImg( CEnBitmap *pEnBmp );
	
	bool BuildHubImg();
	
	bool GetBoxImg( CEnBitmap *pEnBmp );
	
	// Move the current Hub from the temporary to the selected Tab
	bool MoveToSelectedTab();
	
	// Return number of empty slots, will be filled with plug
	UINT GetNumberOfEmptySlots();
	
	// Re-edit a selected Hub for modification
	CDS_HmHub* ModifySelectedProduct();

protected:			
	IDPTR			 m_HubIDPtr;
	CTADatabase		*m_pTADB;
	CTADatastruct	*m_pTADS;
	CEnBitmap		*m_EnBmp;
};
