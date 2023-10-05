#pragma once


class CDpSVisioBaseCommand;
class CDpSVisioHIDCommand : public CDpSVisioBaseCommand
{
public:
	CDpSVisioHIDCommand() : CDpSVisioBaseCommand() {}
	~CDpSVisioHIDCommand() {}

	void SetNewBuffer( UINT8 *pBuffer );
	UINT8 GetCategoryCommand();
	UINT8 GetCommand();
	UINT16 GetFullCommand();
	UINT8 GetPayloadSize();
	UINT8 *GetPayLoadPointer();
	bool GetPayloadData( int iIndex, UINT8 *pData, UINT8 uiSize );
	bool IsError( UINT16 &uiCommand, UINT16 &uiCode );

// Private variables.
private:
	UINT8 m_Buffer[USBD_CUSTOMHID_OUTREPORT_BUF_SIZE];
};
