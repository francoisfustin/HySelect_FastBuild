#include "stdafx.h"
#include "Packet.h"
#include "SerialPort.h"

using namespace TASCOPE;
using namespace std;

CPacket::CPacket(int iSize)
{
	m_eCommand = E_ERROR;
	
	// create data buffer
	m_arData.SetSize(iSize);

	// set padding
	m_bNrPadBytes = (BYTE)((((iSize + 6) >= 64) && (((iSize + 6) % 64) == 0)) ? 1 : 0);
}

CPacket::CPacket(ProtocolCommand eCommand, int iSize)
{
	// set command
	m_eCommand = eCommand;

	// create data buffer
	m_arData.SetSize(iSize);

	// set padding
	m_bNrPadBytes = (BYTE)((((iSize + 6) >= 64) && (((iSize + 6) % 64) == 0)) ? 1 : 0);
}
 
CPacket::CPacket(ProtocolCommand eCommand, CArray<BYTE>& arData)
{
	// set command
	m_eCommand = eCommand;

	// create data buffer
	m_arData.Copy(arData);

	// set padding
	m_bNrPadBytes = (BYTE)((((arData.GetSize() + 6) >= 64) && (((arData.GetSize() + 6) % 64) == 0)) ? 1 : 0);
}

CPacket::CPacket(ProtocolCommand eCommand, string text)
{
	// set command
	m_eCommand = eCommand;

	// create data buffer, +1 due to null-termination
	int iSize = (int)text.size() + 1;
	m_arData.SetSize(iSize);

	// set padding
	m_bNrPadBytes = (BYTE)((((iSize + 6) >= 64) && (((iSize + 6) % 64) == 0)) ? 1 : 0);

	// convert string to byte array
	CArray<BYTE> arTemp;
	StrToByteArray(text, arTemp);

	// copy data
	for(int i = 0; i < (int)text.size(); i++)
		m_arData[i] = arTemp[i];
	
	// add null-termination
	m_arData[text.size()] = 0;
}

CPacket::CPacket(const CPacket &clPacket)
{
	this->m_eCommand = clPacket.m_eCommand;
	this->m_arData.SetSize(clPacket.m_arData.GetSize());
	this->m_arData.Copy(clPacket.m_arData);
	this->m_bNrPadBytes = (BYTE)((((this->m_arData.GetSize() + 6) >= 64) && (((this->m_arData.GetSize() + 6) % 64) == 0)) ? 1 : 0);
}
        
CPacket::~CPacket()
{
}

CPacket &CPacket::operator=(CPacket &clPacket)
{
	this->m_eCommand = clPacket.m_eCommand;
	this->m_arData.SetSize(clPacket.m_arData.GetSize());
	this->m_arData.Copy(clPacket.m_arData);
	this->m_bNrPadBytes = clPacket.m_bNrPadBytes;
	return *this;
}

bool CPacket::writeToSerialPort(CSerialPort& serialPort)
{
	// set checksum
	calculateChecksum();

	// create tx buffer
	CArray<BYTE> arTxBuffer;
	arTxBuffer.SetSize(6 + m_arData.GetSize() + m_bNrPadBytes);

	// set command
	arTxBuffer[0] = (BYTE)m_eCommand;

	// set data length
	arTxBuffer[1] = (BYTE)((m_arData.GetSize() & 0xFF00) >> 8);		// MSB
	arTxBuffer[2] = (BYTE)(m_arData.GetSize()  & 0x00FF);			// LSB

	// copy data
	for (int i = 0; i < m_arData.GetSize(); i++)
		arTxBuffer[3 + i] = m_arData[i];

	// padding length
	arTxBuffer[3 + m_arData.GetSize()] = m_bNrPadBytes;

	// copy padding bytes
	for (int i = 0; i < m_bNrPadBytes; i++)
		arTxBuffer[4 + m_arData.GetSize() + i] = 0;

	// set checksum
	arTxBuffer[4 + m_arData.GetSize() + m_bNrPadBytes] = (m_uiCheckSum & 0xFF00) >> 8;		// MSB
	arTxBuffer[5 + m_arData.GetSize() + m_bNrPadBytes] = m_uiCheckSum  & 0x00FF;			// LSB

	// transmit data
	return serialPort.WriteBuffer((const UINT8*)arTxBuffer.GetData(), (unsigned int)arTxBuffer.GetSize());
}

// Calculates and updates packet checksum.
UINT16 CPacket::calculateChecksum()
{
	UINT16 uiCheckSum = 0;

	uiCheckSum += (UINT16)m_eCommand;
	uiCheckSum += (UINT16)m_arData.GetSize();

	for(int i = 0; i < m_arData.GetSize(); i++)
		uiCheckSum += (UINT16)m_arData[i];
		
	uiCheckSum += (UINT16)m_bNrPadBytes;
	
	m_uiCheckSum = uiCheckSum;
	return m_uiCheckSum;
}

// Receive packet from serialport.
bool CPacket::readFromSerialPort(CSerialPort& serialPort, CPacket& clPacket)
{
	UINT8 bResult;

	while(true)
	{
		// read the protocol command           
		if(!serialPort.ReadChar(bResult))
			return false;

		// TODO: restart if invalid command
		clPacket.m_eCommand = (ProtocolCommand)bResult;

		// read the data length (big endian byte order)
		if(!serialPort.ReadChar(bResult))
			return false;
		UINT16 uiDataLength = (UINT16)bResult << 8;
		if(!serialPort.ReadChar(bResult))
			return false;
		uiDataLength += (UINT16)bResult;

		if(uiDataLength > MAX_DATA_LENGTH)
		{
			// restart if invalid data length
			continue;
		}

		// allocate memory for the packet
		clPacket.m_arData.SetSize(uiDataLength);

		// read the data into the byte array
		for(int i = 0; i < clPacket.m_arData.GetSize(); i++)
		{
			if(!serialPort.ReadChar(bResult))
				return false;

			clPacket.m_arData[i] = bResult;
		}

		// read padding information
		if(!serialPort.ReadChar(bResult))
			return false;
		clPacket.m_bNrPadBytes = bResult;

		// if padding enabled, read pad byte(s)
		if(clPacket.m_bNrPadBytes != 0)
		{
			for (int i = 0; i < clPacket.m_bNrPadBytes; i++)
			{
				if(!serialPort.ReadChar(bResult))
					return false;
			}
		}

		// read checksum (big endian byte order)
		if(!serialPort.ReadChar(bResult))
			return false;
		UINT16 uiCheckSum = (UINT16)bResult << 8;	
		if(!serialPort.ReadChar(bResult))
			return false;	
		uiCheckSum += (UINT16)bResult;
		if(clPacket.calculateChecksum() != uiCheckSum)
		{
			// restart if checksum doesn't match
			continue;
		}

		// a valid packet has been created!
		return true;
	}

	return false;
}

// Converts a string to a byte array. Note that the string will
// not be null-terminated in the array.
void CPacket::StrToByteArray(string str, CArray<BYTE>& arData)
{
	arData.SetSize(str.size());
	string::iterator it;
	int i;
	for(i = 0, it = str.begin(); it != str.end(); i++, it++)
		arData[i] = (BYTE)*it;
}

bool CPacket::createPacket_F_WRITE(UINT32 uiFileSize, string filename, bool fSecure, CPacket &clPacket)
{
	// +4 due to filesize, +1 due to null-termination
	clPacket.m_arData.SetSize(4 + (int)filename.size() + 1);
	
	clPacket.m_eCommand = fSecure ? F_WRITE_SECURE : F_WRITE;

	clPacket.m_arData[0] = (uiFileSize & 0xFF000000) >> 24;
	clPacket.m_arData[1] = (uiFileSize & 0x00FF0000) >> 16;
	clPacket.m_arData[2] = (uiFileSize & 0x0000FF00) >> 8;
	clPacket.m_arData[3] = uiFileSize & 0x000000FF;

	CArray<BYTE> arTemp;
	StrToByteArray(filename, arTemp);
	for (int i = 0; i < arTemp.GetSize(); i++)
		clPacket.m_arData[4 + i] = arTemp[i];
	
	// add null-termination
	clPacket.m_arData[4 + filename.size()] = 0;

	return true;
}
