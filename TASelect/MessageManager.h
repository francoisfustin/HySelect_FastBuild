#pragma once


#include <map>
#include <vector>

// FF: This class will help us to define structure to exchange message between some classes.
//     It's not yet the final version, it must be completed and surely to IMPROVE

class CMessageManager
{
public:
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Defines all classes that are potentially able to send message data to other windows.
	enum ClassID
	{
		CID_Undefined					= -1,
		CID_First						= CID_Undefined + 1,
		CID_CSSheet						= CID_First,
		CID_CRViewBase					= CID_CSSheet + 1,
		CID_CDlgIndSelDpC				= CID_CRViewBase + 1,
		CID_CDlgOutputSpreadContainer	= CID_CDlgIndSelDpC + 1,
		CID_CMultiSpreadBase			= CID_CDlgOutputSpreadContainer + 1,
		CID_Last						= CID_CMultiSpreadBase
	};
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	class CMessage;
	CMessageManager( ClassID eClassID );
	virtual ~CMessageManager();

	// Register an handler to manage notification initiated by 'CDlgOutputSpreadContainer'.
	void MM_RegisterNotificationHandler( CWnd* pWnd, long lNotificationHandlerFlags );
	
	// Unregister a specific notification for a specific window.
	void MM_UnregisterNotificationHandler( CWnd* pWnd, long lNotificationHandlerFlags );

	// Unregister all notifications for a specific window.
	void MM_UnregisterAllNotificationHandlers( CWnd* pWnd );

	bool MM_IsNotificationHandlerRegistered( long lNotificationHandlerFlags );

	bool MM_PostMessage( long lNotification, CMessage* pclMessage );

	CMessage* MM_ReadMessage( WPARAM wParam );

	WPARAM MM_CopyMessage( WPARAM wParam );
	void MM_DestroyMessage( WPARAM wParam );

	class CMessage
	{
	public:
		CMessage() { m_iClassID = -1; m_iMessageType = -1; }
		virtual int GetSizeNeeded( void ) { return ( 2 * sizeof( int ) ); }
		int GetClassID( void ) { return m_iClassID; }
		int GetMessageType( void ) { return m_iMessageType; }
		static void GetClassID( LPBYTE pbBuffer, int* piClassID )
			{
				if( NULL != piClassID && NULL != pbBuffer )
					CopyMemory( (void*)piClassID, (void*)pbBuffer, sizeof( int ) );
			}
		static void GetMessageType( LPBYTE pbBuffer, int* piMessageType )
			{
				if( NULL != pbBuffer && NULL != piMessageType )
				{
					pbBuffer += sizeof( int );
					CopyMemory( (void*)piMessageType, (void*)pbBuffer, sizeof( int ) );
				}
			}
		virtual LPBYTE WriteBuffer( LPBYTE pbBuffer )
			{
				pbBuffer = WriteData( pbBuffer, (LPBYTE)&m_iClassID, sizeof( int ) );
				return WriteData( pbBuffer, (LPBYTE)&m_iMessageType, sizeof( int ) );
			}
		virtual LPBYTE ReadBuffer( LPBYTE pbBuffer )
			{
				pbBuffer = ReadData( (LPBYTE)&m_iClassID, pbBuffer, sizeof( int ) );
				return ReadData( (LPBYTE)&m_iMessageType, pbBuffer, sizeof( int ) );
			}
	protected:
		CMessage( int iClassID, int iMessageType ) { m_iClassID = iClassID; m_iMessageType = iMessageType; }
		LPBYTE WriteData( LPBYTE pDst, LPBYTE pSrc, int iLength )
			{ 
				if( NULL != pDst && NULL != pSrc )
				{
					CopyMemory( (void*)pDst, (void*)pSrc, iLength );
					pDst += iLength;
				}
				return pDst;
			}
		LPBYTE ReadData( LPBYTE pDst, LPBYTE pSrc, int iLength )
			{
				if( NULL != pDst && NULL != pSrc )
				{
					CopyMemory( (void*)pDst, (void*)pSrc, iLength );
					pSrc += iLength;
				}
				return pSrc;
			}
	private:
		int m_iClassID;
		int m_iMessageType;
	};
	typedef CMessage*(*pfnNewMessage)(void);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Start definition of all data relative to 'ClassID::CID_SSheet'.
	enum SSheetNHFlags
	{
		SSheetNHF_First						= 0x0001,
		SSheetNHF_MouseMove					= SSheetNHF_First,
		SSheetNHF_MouseLButtonDown			= ( SSheetNHF_MouseMove << 1 ),
		SSheetNHF_MouseLButtonDblClk		= ( SSheetNHF_MouseLButtonDown << 1 ),
		SSheetNHF_MouseLButtonUp			= ( SSheetNHF_MouseLButtonDblClk << 1 ),
		SSheetNHF_MouseRButtonDown			= ( SSheetNHF_MouseLButtonUp << 1 ),
		SSheetNHF_MouseWheel				= ( SSheetNHF_MouseRButtonDown << 1 ),
		SSheetNHF_MouseCaptureChanged		= ( SSheetNHF_MouseWheel << 1 ),
		SSheetNHF_KeyboardShortcut			= ( SSheetNHF_MouseCaptureChanged << 1 ),
		SSheetNHF_KeyboardVirtualKeyDown	= ( SSheetNHF_KeyboardShortcut << 1 ),
		SSheetNHF_KeyboardVirtualKeyUp		= ( SSheetNHF_KeyboardVirtualKeyDown << 1 ),
		SSheetNHF_ColWidthChanged			= ( SSheetNHF_KeyboardVirtualKeyUp << 1 ),
		SSheetNHF_SizeChanged				= ( SSheetNHF_ColWidthChanged << 1 ),
		SSheetNHF_NavigateOccurs			= ( SSheetNHF_SizeChanged << 1 ),
		SSheetNHF_Last						= SSheetNHF_NavigateOccurs
	};

	enum SSheetMessageType
	{
		SSheetMST_MouseEvent,
		SSheetMST_KeyboardEvent,
		SSheetMST_ColumnWidthChanged,
		SSheetMST_SizeChanged,
		SSheetMST_NavigateOccurs
	};

	enum SSheetMET
	{
		MET_Undefined = -1,
		MET_MouseMove,
		MET_LButtonDown,
		MET_LButtonDblClk,
		MET_LButtonUp,
		MET_RButtonDown,
		MET_MouseWheel,
		MET_MouseCaptured
	};

	typedef struct _SSheetMouseEventParams
	{
		HWND m_hSheet;
		SSheetMET m_eMouseEventType;
		long m_lColumn;
		long m_lRow;
		CPoint m_ptMouse;
		int m_iDelta;
		HWND m_hWnd;
		struct _SSheetMouseEventParams()
		{
			Reset();
		}
		void Reset()
		{
			m_hSheet = (HWND)0;
			m_eMouseEventType = MET_Undefined;
			m_lColumn = 0;
			m_lRow = 0;
			m_ptMouse = 0;
			m_iDelta = 0;
			m_hWnd = (HWND)0;
		}
	}SSheetMEParams;

	enum SSheetKET
	{
		KET_Undefined = -1,
		KET_Shortcut,
		KET_VirtualKeyDown,
		KET_VirtualKeyUp,
	};

	typedef struct _SSheetKeyboardEventParams
	{
		HWND m_hSheet;
		SSheetKET m_eKeyboardEventType;
		int m_iKeyCode;
		struct _SSheetKeyboardEventParams()
		{
			Reset();
		}
		void Reset()
		{
			m_hSheet = (HWND)0;
			m_eKeyboardEventType = KET_Undefined;
			m_iKeyCode = 0;
		}
	}SSheetKEParams;

	typedef struct _SSheetColumnWidthChangedParams
	{
		HWND m_hSheet;
		long m_lColumn;
		struct _SSheetColumnWidthChangedParams()
		{
			Reset();
		}
		void Reset()
		{
			m_hSheet = (HWND)0;
			m_lColumn = 0;
		}
	}SSheetCWCParams;

	typedef struct _SSheetSizeChangedParams
	{
		HWND m_hSheet;
		UINT m_nType;
		int m_cx;
		int m_cy;
		struct _SSheetSizeChangedParams()
		{
			Reset();
		}
		void Reset()
		{
			m_hSheet = (HWND)0;
			m_nType = 0;
			m_cx = 0;
			m_cy = 0;
		}
	}SSheetSCParams;

	typedef struct _SSheetNavigateOccursParams
	{
		HWND m_hSheet;
		long m_lOldCol;
		long m_lOldRow;
		long m_lNewCol;
		long m_lNewRow;
		struct _SSheetNavigateOccursParams()
		{
			Reset();
		}
		void Reset()
		{
			m_hSheet = (HWND)0;
			m_lOldCol = 0;
			m_lOldRow = 0;
			m_lNewCol = 0;
			m_lNewRow = 0;
		}
	}SSheetNOParams;

	// Sheet 'Mouse Event' message.
	class CSSheetMEMsg : public CMessage
	{
	public:
		CSSheetMEMsg() : CMessage( CID_CSSheet, SSheetMST_MouseEvent ) { m_rParams.Reset(); }
		virtual int GetSizeNeeded( void ) { return ( CMessage::GetSizeNeeded() + sizeof( SSheetMEParams ) ); }
		static CMessage* NewMessage( void ) { return new CSSheetMEMsg(); }
		SSheetMEParams* GetParams() { return &m_rParams; }
		virtual LPBYTE WriteBuffer( LPBYTE pbBuffer )
			{
				pbBuffer = CMessage::WriteBuffer( pbBuffer );
				return WriteData( pbBuffer, (LPBYTE)&m_rParams, sizeof( SSheetMEParams ) );
			}
		virtual LPBYTE ReadBuffer( LPBYTE pbBuffer )
			{
				pbBuffer = CMessage::ReadBuffer( pbBuffer );
				return ReadData( (LPBYTE)&m_rParams, pbBuffer, sizeof( SSheetMEParams ) );
			}
	private:
		SSheetMEParams m_rParams;
	};

	// Sheet 'Keyboard Event' message.
	class CSSheetKEMsg : public CMessage
	{
	public:
		CSSheetKEMsg() : CMessage( CID_CSSheet, SSheetMST_KeyboardEvent ) { m_rParams.Reset(); }
		virtual int GetSizeNeeded( void ) { return ( CMessage::GetSizeNeeded() + sizeof( SSheetKEParams ) ); }
		static CMessage* NewMessage( void ) { return new CSSheetKEMsg(); }
		SSheetKEParams* GetParams() { return &m_rParams; }
		virtual LPBYTE WriteBuffer( LPBYTE pbBuffer )
			{
				pbBuffer = CMessage::WriteBuffer( pbBuffer );
				return WriteData( pbBuffer, (LPBYTE)&m_rParams, sizeof( SSheetKEParams ) );
			}
		virtual LPBYTE ReadBuffer( LPBYTE pbBuffer )
			{
				pbBuffer = CMessage::ReadBuffer( pbBuffer );
				return ReadData( (LPBYTE)&m_rParams, pbBuffer, sizeof( SSheetKEParams ) );
			}
	private:
		SSheetKEParams m_rParams;
	};

	// Sheet 'Column Width Changed' message.
	class CSSheetCWCMsg : public CMessage
	{
	public:
		CSSheetCWCMsg() : CMessage( CID_CSSheet, SSheetMST_ColumnWidthChanged ) { m_rParams.Reset(); }
		virtual int GetSizeNeeded( void ) { return ( CMessage::GetSizeNeeded() + sizeof( SSheetCWCParams ) ); }
		static CMessage* NewMessage( void ) { return new CSSheetCWCMsg(); }
		SSheetCWCParams* GetParams() { return &m_rParams; }
		virtual LPBYTE WriteBuffer( LPBYTE pbBuffer )
			{
				pbBuffer = CMessage::WriteBuffer( pbBuffer );
				return WriteData( pbBuffer, (LPBYTE)&m_rParams, sizeof( SSheetCWCParams ) );
			}
		virtual LPBYTE ReadBuffer( LPBYTE pbBuffer )
			{
				pbBuffer = CMessage::ReadBuffer( pbBuffer );
				return ReadData( (LPBYTE)&m_rParams, pbBuffer, sizeof( SSheetCWCParams ) );
			}
	private:
		SSheetCWCParams m_rParams;
	};

	// Sheet 'Size Changed' message.
	class CSSheetSCMsg : public CMessage
	{
	public:
		CSSheetSCMsg() : CMessage( CID_CSSheet, SSheetMST_SizeChanged ) { m_rParams.Reset(); }
		virtual int GetSizeNeeded( void ) { return ( CMessage::GetSizeNeeded() + sizeof( SSheetSCParams ) ); }
		static CMessage* NewMessage( void ) { return new CSSheetSCMsg(); }
		SSheetSCParams* GetParams() { return &m_rParams; }
		virtual LPBYTE WriteBuffer( LPBYTE pbBuffer )
			{
				pbBuffer = CMessage::WriteBuffer( pbBuffer );
				return WriteData( pbBuffer, (LPBYTE)&m_rParams, sizeof( SSheetSCParams ) );
			}
		virtual LPBYTE ReadBuffer( LPBYTE pbBuffer )
			{
				pbBuffer = CMessage::ReadBuffer( pbBuffer );
				return ReadData( (LPBYTE)&m_rParams, pbBuffer, sizeof( SSheetSCParams ) );
			}
	private:
		SSheetSCParams m_rParams;
	};

	// Sheet 'Navigation Occurs' message.
	class CSSheetNOMsg : public CMessage
	{
	public:
		CSSheetNOMsg() : CMessage( CID_CSSheet, SSheetMST_NavigateOccurs ) { m_rParams.Reset(); }
		virtual int GetSizeNeeded( void ) { return ( CMessage::GetSizeNeeded() + sizeof( SSheetNOParams ) ); }
		static CMessage* NewMessage( void ) { return new CSSheetNOMsg(); }
		SSheetNOParams* GetParams() { return &m_rParams; }
		virtual LPBYTE WriteBuffer( LPBYTE pbBuffer )
			{
				pbBuffer = CMessage::WriteBuffer( pbBuffer );
				return WriteData( pbBuffer, (LPBYTE)&m_rParams, sizeof( SSheetNOParams ) );
			}
		virtual LPBYTE ReadBuffer( LPBYTE pbBuffer )
			{
				pbBuffer = CMessage::ReadBuffer( pbBuffer );
				return ReadData( (LPBYTE)&m_rParams, pbBuffer, sizeof( SSheetNOParams ) );
			}
	private:
		SSheetNOParams m_rParams;
	};

	bool MM_PM_SSheet_MouseMove( HWND hSSheet, long lColumn, long lRow, CPoint point );
	bool MM_PM_SSheet_MouseLButtonDown( HWND hSSheet, long lColumn, long lRow, CPoint point );
	bool MM_PM_SSheet_MouseLButtonDblClk( HWND hSSheet, long lColumn, long lRow, CPoint point );
	bool MM_PM_SSheet_MouseLButtonUp( HWND hSSheet, long lColumn, long lRow, CPoint point );
	bool MM_PM_SSheet_MouseRButtonDown( HWND hSSheet, long lColumn, long lRow, CPoint point );
	bool MM_PM_SSheet_MouseWheel( HWND hSSheet, int iDelta );
	bool MM_PM_SSheet_MouseCaptureChanged( HWND hSSheet, HWND hWnd );
	bool MM_PM_SSheet_KeyboardShortcut( HWND hSSheet, int iKeyCode );
	bool MM_PM_SSheet_KeyboardVirtualKeyDown( HWND hSSheet, int iKeyCode );
	bool MM_PM_SSheet_KeyboardVirtualKeyUp( HWND hSSheet, int iKeyCode );
	bool MM_PM_SSheet_ColWidthChanged( HWND hSSheet, long lColumn );
	bool MM_PM_SSheet_SizeChanged( HWND hSSheet, UINT uiType, int iCx, int iCy );
	bool MM_PM_SSheet_NavigationOccurs( HWND hSSheet, long lOldCol, long lOldRow, long lNewCol, long lNewRow );
	// End definition of all data relative to 'ClassID::CID_SSheet'.
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Private methods.
private:
	LPBYTE _SetCounter( LPBYTE pbBuffer, short& nCounter, bool fIncrementPointer = true );
	LPBYTE _GetCounter( LPBYTE pbBuffer, short& nCounter, bool fIncrementPointer = true );

// Private variables.
private:
	ClassID m_eClassID;
	std::map<long, std::vector<CWnd*>> m_mapMessageManagerNHList;
	std::map<ClassID, CWnd*> m_mapMethodHandlerList;
	
	typedef std::map<int, pfnNewMessage> mapIntFn;
	std::map<int, mapIntFn>	m_mapMessageCreators;
	
	long m_lFirstFlag;
	long m_lLastFlag;
};
