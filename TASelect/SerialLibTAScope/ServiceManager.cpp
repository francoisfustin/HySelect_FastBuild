#include "stdafx.h"

#include <math.h>
#include "ServiceManager.h"
//#include "DpCalibrationImage.h"
//#include "TempSensorCalibrationImage.h"
#include "HiPerfTimer.h"
#include "..\TASelect.h"

using namespace TASCOPE;
using namespace std;

TASCOPE::ErrorCode TASCOPE::ServiceManager::m_LastErrorCode = ErrorCode::NONE;
bool TASCOPE::ServiceManager::m_bComInterruptRequested = false;

UINT16 ServiceManager::wCRCTable[256] = {
           0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
           0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
           0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
           0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
           0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
           0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
           0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
           0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
           0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
           0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
           0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
           0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
           0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
           0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
           0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
           0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
           0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
           0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
           0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
           0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
           0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
           0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
           0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
           0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
           0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
           0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
           0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
           0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
           0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
           0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
           0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
           0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

string ServiceManager::RemovePath(string filename)
{
	// Remove start of name if path present (keep only name)
	int n;
	int iLimit = (int)filename.size() - 1;
	for(n = iLimit; n >= 0; n--)
	{
		if (filename[n] == '\\')
			break;
	}
	n++;
	return filename.substr(n, filename.size() - n);
}


ServiceManager::eComStatus ServiceManager::service_F_DELETE(CSerialPort& serialPort, string filename) 
{
	// create service request packet
	CPacket clPacket = CPacket(F_DELETE, filename);
	if(!sendPacket(serialPort, clPacket))
		return eComStatus::ecsError;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return eComStatus::ecsError;

	// success
	return eComStatus::ecsOKFinish;
}

void ServiceManager::service_F_ERASE_NANDFLASH(CSerialPort& serialPort)
{
	// display message
	//cout << endl << "Erasing NAND Flash...";

	// create service request packet
	CPacket clPacket = CPacket(F_ERASE_NANDFLASH, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// store serialport timeout
	DWORD dwRxTimeout, dwTxTimeout;
	if(!serialPort.GetTimeouts(dwRxTimeout, dwTxTimeout))
		return;

	// increase serialport timeout
	if(!serialPort.SetTimeouts(10000, dwTxTimeout))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
	{
		// restore serialport timeout
		serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);
		
		return;
	}

	// restore serialport timeout
	serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

	// success
	//cout << "Success!" << endl;
}

void ServiceManager::service_F_FORMAT(CSerialPort& serialPort)
{
	// display message
	//cout << endl << "Formating CBX file system...";

	// create service request packet
	CPacket clPacket = CPacket(F_FORMAT, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// store serialport timeout
	DWORD dwRxTimeout, dwTxTimeout;
	if(!serialPort.GetTimeouts(dwRxTimeout, dwTxTimeout))
		return;

	// increase serialport timeout
	if(!serialPort.SetTimeouts(10000, dwTxTimeout))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
	{
		// restore serialport timeout
		serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);
		
		return;
	}

	// restore serialport timeout
	serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

	// success
	//cout << "Success!" << endl;
}



void ServiceManager::service_F_DELETE_BOOTLOADER(CSerialPort& serialPort)
{
	// display message
	//cout << endl << "Deleting CBX bootloader...";

	// create service request packet
	CPacket clPacket = CPacket(F_DELETE_BOOTLOADER, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << "Success!" << endl;
}

void ServiceManager::service_SET_TIME(CSerialPort& serialPort)
{
	// time & localtime
	time_t now;
	time(&now);
	struct tm ptmlocaltime; 
	localtime_s(&ptmlocaltime,&now);
	UINT32 dwUNIXTime = (UINT32)_mkgmtime32(&ptmlocaltime);

	// create service request m_arData
	CArray<BYTE> buffer;
	buffer.SetSize(4);
	for(int i = 0; i < 4; i++)
		buffer[i] = (BYTE)(dwUNIXTime >> (i * 8));
		
	// create service request packet
	CPacket clPacket = CPacket(SET_TIME, buffer);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}


void ServiceManager::service_SET_PANID(CSerialPort& serialPort, UINT64 panId)
{
	// create service request m_arData
	CArray<BYTE> buffer;
	buffer.SetSize(8);
	for(int i = 0; i < 8; i++)
		buffer[i] = (BYTE)(panId >> (i * 8));

	// create service request packet
	CPacket clPacket = CPacket(SET_PANID, buffer);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_GET_PANID(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(GET_PANID, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, GET_PANID, clPacket))
		return;
		
	// get PAN ID 
	UINT64 panId =	(UINT64)clPacket.m_arData[0] << 0 | (UINT64)clPacket.m_arData[1] << 8 | (UINT64)clPacket.m_arData[2] << 16 | 
					(UINT64)clPacket.m_arData[3] << 24 | (UINT64)clPacket.m_arData[4] << 32 | (UINT64)clPacket.m_arData[5] << 40 | 
					(UINT64)clPacket.m_arData[6] << 48 | (UINT64)clPacket.m_arData[7] << 56;

	// success
	char szTemp[128];
	sprintf_s(szTemp,sizeof(szTemp), "%016I64X", panId);
	//cout << endl << (panId & 0xFFFFFFFF) << "   -   Full hex mac: 0x" << szTemp << endl;
}

void ServiceManager::service_SET_SENSOR_MAC(CSerialPort& serialPort, UINT64 mac)
{
	// create service request m_arData
	BYTE buffer[8];
	for(int i = 0; i < 8; i++)
		buffer[i] = (BYTE)(mac >> (i * 8));
		
	// create service request packet
	CPacket clPacket = CPacket(SET_SENSOR_MAC, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// store serialport timeout
	DWORD dwRxTimeout, dwTxTimeout;
	if(!serialPort.GetTimeouts(dwRxTimeout, dwTxTimeout))
		return;

	// increase serialport timeout
	if(!serialPort.SetTimeouts(10000, dwTxTimeout))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
	{
		// restore serialport timeout
		serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);
		
		return;
	}

	// restore serialport timeout
	serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

	// success
	//cout << "Success!" << endl;
}

void ServiceManager::service_GET_SENSOR_MAC(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(GET_SENSOR_MAC, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, GET_SENSOR_MAC, clPacket))
		return;

	// get PAN ID 
	UINT64 mac =	(UINT64)clPacket.m_arData[0] << 0 | (UINT64)clPacket.m_arData[1] << 8 | (UINT64)clPacket.m_arData[2] << 16 | 
					(UINT64)clPacket.m_arData[3] << 24 | (UINT64)clPacket.m_arData[4] << 32 | (UINT64)clPacket.m_arData[5] << 40 | 
					(UINT64)clPacket.m_arData[6] << 48 | (UINT64)clPacket.m_arData[7] << 56;

	// success
	char szTemp[128];
	sprintf_s(szTemp,sizeof(szTemp), "%I64X", mac);
	//cout << endl << (mac & 0xFFFFFFFF) << "   -   Full hex mac: 0x" << szTemp << endl;
}

void ServiceManager::service_SET_INTERNAL_I2C_ON(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(SET_INTERNAL_I2C_ON, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_SET_INTERNAL_I2C_OFF(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(SET_INTERNAL_I2C_OFF, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_SET_SENSOR_INTERNAL_I2C_ON(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(SET_SENSOR_INTERNAL_I2C_ON, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_SET_SENSOR_INTERNAL_I2C_OFF(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(SET_SENSOR_INTERNAL_I2C_OFF, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_SET_EXTERNAL_PWR_ON(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(SET_EXTERNAL_PWR_ON, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_SET_EXTERNAL_PWR_OFF(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(SET_EXTERNAL_PWR_OFF, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_GET_RAW_DATA(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(GET_RAW_DATA, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, GET_RAW_DATA, clPacket))
		return;

	UINT32 dp = (UINT32)clPacket.m_arData[0] << 0 | (UINT32)clPacket.m_arData[1] << 8;
	UINT32 thh = (UINT32)clPacket.m_arData[2] << 0 | (UINT32)clPacket.m_arData[3] << 8;
	UINT32 t1 = (UINT32)clPacket.m_arData[4] << 0 | (UINT32)clPacket.m_arData[5] << 8;
	UINT32 t2 = (UINT32)clPacket.m_arData[6] << 0 | (UINT32)clPacket.m_arData[7] << 8;

	// success
	char szTemp[128];
	//cout << endl;
	sprintf_s(szTemp,sizeof(szTemp), "%u", dp);
	//cout << "DP : " << ((dp == 0xFFFF) ? "---" : szTemp) << endl;
	sprintf_s(szTemp,sizeof(szTemp), "%u", thh);
	//cout << "Thh: " << ((thh == 0xFFFF) ? "---" : szTemp) << endl;
	sprintf_s(szTemp,sizeof(szTemp), "%u", t1);
	//cout << "T1 : " << ((t1 == 0xFFFF) ? "---" : szTemp) << endl;
	sprintf_s(szTemp,sizeof(szTemp), "%u", t2);
	//cout << "T2 : " << ((t2 == 0xFFFF) ? "---" : szTemp) << endl << endl;
}

void ServiceManager::service_GET_PHYSICAL_DATA(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(GET_PHYSICAL_DATA, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, GET_PHYSICAL_DATA, clPacket))
		return;
	
	double ddp;
	memcpy((BYTE*)&ddp, &clPacket.m_arData[0], 8);
	double dthh;
	memcpy((BYTE*)&dthh, &clPacket.m_arData[8], 8);
	double dt1;
	memcpy((BYTE*)&dt1, &clPacket.m_arData[16], 8);
	double dt2;
	memcpy((BYTE*)&dt2, &clPacket.m_arData[24], 8);

	// success
	char szTemp[128];
	//cout << endl;
	sprintf_s(szTemp,sizeof(szTemp), "%f0.", ddp);
	//cout << "DP : " << ((ddp >= 1000000) ? "---" : szTemp) << " Pa" << endl;
	sprintf_s(szTemp,sizeof(szTemp), "%f0.##", dthh);
	//cout << "Thh: " << ((dthh >= 1000000) ? "---" : szTemp) << " °C" << endl;
	sprintf_s(szTemp,sizeof(szTemp), "%f0.##", dt1);
	//cout << "T1 : " << ((dt1 >= 1000000) ? "---" : szTemp) << " °C" << endl;
	sprintf_s(szTemp,sizeof(szTemp), "%f0.##", dt2);
	//cout << "T2 : " << ((dt2 >= 1000000) ? "---" : szTemp) << " °C" << endl << endl;
}

void ServiceManager::service_GET_T3_DATA(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(GET_T3_DATA, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, GET_T3_DATA, clPacket))
		return;

	UINT32 PT = (UINT32)clPacket.m_arData[0] << 0 | (UINT32)clPacket.m_arData[1] << 8;
	UINT32 TC = (UINT32)clPacket.m_arData[2] << 0 | (UINT32)clPacket.m_arData[3] << 8;

	// success
	char szTemp[128];
	//cout << endl;
	sprintf_s(szTemp,sizeof(szTemp), "%u", PT);
	//cout << "PT: " << ((PT == 0xFFFF) ? "---" : szTemp) << endl;
	sprintf_s(szTemp,sizeof(szTemp), "%u", TC);
	//cout << "TC: " << ((TC == 0xFFFF) ? "---" : szTemp) << endl;
}



void ServiceManager::service_SET_CALIB_DP(CSerialPort& serialPort, string filename)
{
/*	// load calibration image
	DpCalibrationImage image;
	if(!DpCalibrationImage::loadImage(filename, image))
		return;

	// update UTC time
	if(image.updateUTC() == false)
		return;

	// save the new image
	if(DpCalibrationImage::saveImage(image, filename) == false)
		return;

	// send the xml file to cbx
	if(service_F_WRITE(serialPort, filename, false) == false)
		return;
		
	// set calibration image
	//cout << endl << "Writing calibration image...";

	// create service request packet
	CPacket clPacket = CPacket(SET_CALIB_DP, RemovePath(filename));
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
	{
		// delete the file
		service_F_DELETE(serialPort, RemovePath(filename));
		// exit
		return;
	}

	// success
	//cout << "Success!" << endl;

	// delete the file
	service_F_DELETE(serialPort, RemovePath(filename));
*/
}

void ServiceManager::service_SET_CALIB_DTS(CSerialPort& serialPort, string filename)
{
/*	// load calibration image
	TempSensorCalibrationImage image;
	if(!TempSensorCalibrationImage::loadImage(filename, image))
		return;

	// update UTC time
	if(image.updateUTC() == false)
		return;

	// save the new image
	if(TempSensorCalibrationImage::saveImage(image, filename) == false)
		return;

	// send the xml file to cbx
	if(service_F_WRITE(serialPort, filename, false) == false)
		return;

	// set calibration image
	//cout << endl << "Writing calibration image...";

	// create service request packet
	CPacket clPacket = CPacket(SET_CALIB_DTS, RemovePath(filename));
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
	{
		// delete the file
		service_F_DELETE(serialPort, RemovePath(filename));
		// exit
		return;
	}

	// success
	//cout << "Success!" << endl;

	// delete the file
	service_F_DELETE(serialPort, RemovePath(filename));
*/}

void ServiceManager::service_GET_CALIB_DP(CSerialPort& serialPort, string filename)
{
/*	// sanity check
	if(filename.substr(filename.size() - 3, 3).compare("xml"))
	{
		// display error message
		//cout << endl << "Bad file extension, must be \".xml\"!" << endl;
		// exit
		return;
	}

	// set calibration image
	//cout << endl << "Reading calibration image...";

	// create service request packet
	CPacket clPacket = CPacket(GET_CALIB_DP, filename);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << "Success!" << endl;

	// read the file
	if(service_F_READ(serialPort, filename) == false)
		return;

	// delete the file on CBX
	service_F_DELETE(serialPort, RemovePath(filename));

	// load calibration image
	DpCalibrationImage image;
	if(!DpCalibrationImage::loadImage(filename, image))
		return;

	// update calibration date
	//cout << endl << "Updating calibration date...";
	if(image.updateDate() == false)
		return;

	// save the new image
	if(DpCalibrationImage::saveImage(image, filename) == false)
		return;

	// success
	//cout << "Success!" << endl;
*/
}

void ServiceManager::service_GET_CALIB_DTS(CSerialPort& serialPort, string filename)
{
/*	// sanity check
	if(filename.substr(filename.size() - 3, 3).compare("xml"))
	{
		// display error message
		//cout << endl << "Bad file extension, must be \".xml\"!" << endl;
		// exit
		return;
	}
	
	// set calibration image
	//cout << endl << "Reading calibration image...";

	// create service request packet
	CPacket clPacket = CPacket(GET_CALIB_DTS, filename);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << "Success!" << endl;

	// read the file
	if(service_F_READ(serialPort, filename) == false)
		return;

	// delete the file on CBX
	service_F_DELETE(serialPort, RemovePath(filename));

	// load calibration image
	TempSensorCalibrationImage image;
	if(!TempSensorCalibrationImage::loadImage(filename, image))
		return;

	// update calibration date
	//cout << endl << "Updating calibration date...";
	if(image.updateDate() == false)
		return;

	// save the new image
	if(TempSensorCalibrationImage::saveImage(image, filename) == false)
		return;

	// success
	//cout << "Success!" << endl;
*/
}

void ServiceManager::service_GET_SENSOR_MAGNET(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(GET_SENSOR_MAGNET, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, GET_SENSOR_MAGNET, clPacket))
		return;

	// success
	//cout << endl << ((clPacket.m_arData[0] == 1) ? "ON" : "OFF") << endl;
}

void ServiceManager::service_SET_KEYBOARD(CSerialPort& serialPort, BYTE value)
{
	// create service request m_arData
	CArray<BYTE> buffer;
	buffer.SetSize(1);
	buffer[0] = value;

	// create service request packet
	CPacket clPacket = CPacket(SET_KEYBOARD, buffer);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_GET_KEYBOARD(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(GET_KEYBOARD, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, GET_KEYBOARD, clPacket))
		return;

	// success
	char szTemp[128];
	sprintf_s(szTemp,sizeof(szTemp), "0x%X", clPacket.m_arData[0]);
	//cout << endl << szTemp << endl;
}

void ServiceManager::service_SET_SENSOR_LED(CSerialPort& serialPort, BYTE value)
{
	// create service request m_arData
	CArray<BYTE> buffer;
	buffer.SetSize(1);
	buffer[0] = value;

	// create service request packet
	CPacket clPacket = CPacket(SET_SENSOR_LED, buffer);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_RUN_SLEEP(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(RUN_SLEEP, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_RUN_BUZZER(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(RUN_BUZZER, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}


void ServiceManager::service_RUN_DIAGNOSTICS(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(RUN_DIAGNOSTICS, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// store serialport timeout
	DWORD dwRxTimeout, dwTxTimeout;
	if(!serialPort.GetTimeouts(dwRxTimeout, dwTxTimeout))
		return;

	// increase serialport timeout
	if(!serialPort.SetTimeouts(20000, dwTxTimeout))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, RUN_DIAGNOSTICS, clPacket))
	{
		// restore serialport timeout
		serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);
		
		return;
	}

	// reset time out
	serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

	// success
	//cout << endl;
	//cout << "Battery Fuel Gauge..." << ((clPacket.m_arData[0] == 1) ? "OK" : "NOK") << endl;
	//cout << "Digital Temp Sensor..." << ((clPacket.m_arData[1] == 1) ? "OK" : "NOK") << endl;
	//cout << "RF Communication..." << ((clPacket.m_arData[2] == 1) ? "OK" : "NOK") << endl;
	//cout << "Wired Communication..." << ((clPacket.m_arData[3] == 1) ? "OK" : "NOK") << endl;
}


void ServiceManager::service_RUN_SENSOR_DIAGNOSTICS(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(RUN_SENSOR_DIAGNOSTICS, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// store serialport timeout
	DWORD dwRxTimeout, dwTxTimeout;
	if(!serialPort.GetTimeouts(dwRxTimeout, dwTxTimeout))
		return;

	// increase serialport timeout
	if(!serialPort.SetTimeouts(20000, dwTxTimeout))
		return;
		
	// wait for valid response
	if(!receivePacket(serialPort, RUN_SENSOR_DIAGNOSTICS, clPacket))
	{
		// restore serialport timeout
		serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);
		
		return;
	}

	// reset time out
	serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

	// success
	//cout << endl;
	//cout << "DP Sensor..." << ((clPacket.m_arData[0] == 1) ? "OK" : "NOK")<< endl;
	//cout << "DP Temp Sensor..." << ((clPacket.m_arData[1] == 1) ? "OK" : "NOK")<< endl;
	//cout << "Digital Temp Sensor 1..." << ((clPacket.m_arData[2] == 1) ? "OK" : "NOK")<< endl;
	//cout << "Digital Temp Sensor 2..." << ((clPacket.m_arData[3] == 1) ? "OK" : "NOK")<< endl;
	//cout << "SPI Memory..." << ((clPacket.m_arData[4] == 1) ? "OK" : "NOK")<< endl;
	//cout << "Battery Fuel Gauge..." << ((clPacket.m_arData[5] == 1) ? "OK" : "NOK")<< endl;
	//cout << "RF Communication..." << ((clPacket.m_arData[6] == 1) ? "OK" : "NOK")<< endl;
	//cout << "Wired Communication..." << ((clPacket.m_arData[7] == 1) ? "OK" : "NOK") << endl << endl;
}

void ServiceManager::service_SET_BATTERY_INSTAL_DATE(CSerialPort& serialPort, UINT32 valueUTC)
{
	// create service request m_arData
	CArray<BYTE> buffer;
	buffer.SetSize(4);
	for (int i = 0; i < 4; i++)
		buffer[i] = (BYTE)(valueUTC >> (i * 8));
		
	// create service request packet
	CPacket clPacket = CPacket(SET_BATTERY_INSTAL_DATE, buffer);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_GET_BATTERY_INSTALLATION_DATE(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(GET_BATTERY_INSTAL_DATE, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, GET_BATTERY_INSTAL_DATE, clPacket))
		return;

	// get Date in UTC 
	UINT32 valueUTC = (UINT32)clPacket.m_arData[0] << 0 | (UINT32)clPacket.m_arData[1] << 8 | (UINT32)clPacket.m_arData[2] << 16 | (UINT32)clPacket.m_arData[3] << 24;

	// Convert
	CTime BatDate(valueUTC);

	// success
	//cout << endl << WString2String((LPCTSTR)BatDate.Format(L"%Y/%m/%d")).c_str() << endl;
}

void ServiceManager::service_SET_SENSOR_BATTERY_INSTAL_DATE(CSerialPort& serialPort, UINT32 valueUTC)
{
	// create service request m_arData
	CArray<BYTE> buffer;
	buffer.SetSize(4);
	for(int i = 0; i < 4; i++)
		buffer[i] = (byte)(valueUTC >> (i * 8));
	
	// create service request packet
	CPacket clPacket = CPacket(SET_SENSOR_BATTERY_INSTAL_DATE, buffer);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return;

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_GET_SENSOR_BATTERY_INSTALLATION_DATE(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(GET_SENSOR_BATTERY_INSTAL_DATE, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, GET_SENSOR_BATTERY_INSTAL_DATE, clPacket))
		return;

	// get date in UTC 
	UINT32 valueUTC = (UINT32)clPacket.m_arData[0] << 0 | (UINT32)clPacket.m_arData[1] << 8 | (UINT32)clPacket.m_arData[2] << 16 | (UINT32)clPacket.m_arData[3] << 24; 

	// Convert
	CTime BatDate(valueUTC);

	// success
	//cout << endl << WString2String((LPCTSTR)BatDate.Format(L"%Y/%m/%d")).c_str() << endl;
}

void ServiceManager::service_RUN_BATTERY_IMPEDANCE_TRACK(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(RUN_BATTERY_IMPEDANCE_TRACK, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// store timeout
	DWORD dwRxTimeout, dwTxTimeout;
	if(!serialPort.GetTimeouts(dwRxTimeout, dwTxTimeout))
		return;

	// increase timeout
	if(!serialPort.SetTimeouts(8000, dwTxTimeout))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
	{
		// restore serialport timeout
		serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);
		
		return;
	}

	// reset time out
	serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

	// success
	//cout << endl << "Success!" << endl;
}

void ServiceManager::service_RUN_SENSOR_BATTERY_IMPEDANCE_TRACK(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(RUN_SENSOR_BATTERY_IMPEDANCE_TRACK, 0);
	if(!sendPacket(serialPort, clPacket))
		return;

	// store serialport timeout
	DWORD dwRxTimeout, dwTxTimeout;
	if(!serialPort.GetTimeouts(dwRxTimeout, dwTxTimeout))
		return;

	// increase serialport timeout
	if(!serialPort.SetTimeouts(10000, dwTxTimeout))
		return;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
	{
		// restore serialport timeout
		serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);
		
		return;
	}

	// restore serialport timeout
	serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

	// success
	//cout << endl << "Success!" << endl;
}

bool ServiceManager::sendPacket(CSerialPort& serialPort, CPacket& clPacket)
{
	if(!clPacket.writeToSerialPort(serialPort))
		return false;

	return true;
}
 
bool ServiceManager::receivePacket(CSerialPort& serialPort, ProtocolCommand eExpectedCommand, CPacket& clPacket)
{
	// create array
	ProtocolCommand expectedCommands[1];

	// copy expected command to array
	expectedCommands[0] = eExpectedCommand;

	// receive packet
	return receivePacket(serialPort, expectedCommands, 1, clPacket);
}

bool ServiceManager::receivePacket(CSerialPort& serialPort, ProtocolCommand *peExpectedCommands, int iCommandSize, CPacket& clPacket)
{
	if(!CPacket::readFromSerialPort(serialPort, clPacket))
	{
		//cout << endl << "ERROR: no response from CBX (timeout)" << endl;

		// request failed
		return false;
	}
	
	// check if the response is valid
	if(!validCommand(clPacket.m_eCommand, peExpectedCommands, iCommandSize))
	{
		if(clPacket.m_eCommand == E_ERROR)
			if (clPacket.m_arData.GetSize())
				printErrorCodeInfo((ErrorCode)clPacket.m_arData[0]);
			else
				printErrorCodeInfo(RESPONSE);
	
		// request failed
		return false;
	}
	
	// success
	return true;
}

bool ServiceManager::validCommand(ProtocolCommand receivedCommand, ProtocolCommand *peExpectedCommands, int iCommandSize)
{
	for(int i = 0; i < iCommandSize; i++)
	{
		if(peExpectedCommands[i] == receivedCommand)
			return true;
	}
	
    // command not found
    return false;
}

void ServiceManager::printErrorCodeInfo(ErrorCode errorCode)
{
	m_LastErrorCode = errorCode;
	switch(errorCode)
	{
	case INPUT:
		//cout << endl << "ERROR: bad input m_arData" << endl;
		break;
		
	case RESPONSE:
		//cout << endl << "ERROR: unexpected response " << endl;
		break;
		
	case UNSUPPORTED:
		//cout << endl << "ERROR: service unsupported " << endl;
		break;
		
	case UNSUPPORTED_BOOT:
		//cout << endl << "ERROR: service unsupported by bootloader" << endl;
		break;
		
	case CHECKSUM:
		//cout << endl << "ERROR: bad checksum " << endl;
		break;
		
	case FILESYSTEM:
		//cout << endl << "ERROR: filesystem error " << endl;
		break;
		
	case INTERNAL:
		//cout << endl << "ERROR: internal error " << endl;
		break;
		
	case INVALID_CRC:
		//cout << endl << "ERROR: invalid CRC " << endl;
		break;
		
	case OFFLINE:
		//cout << endl << "ERROR: offline " << endl;
		break;
		
	case OFFLINE_RS485:
		//cout << endl << "ERROR: sensor not connected via RS485 " << endl;
		break;
		
	default:
		//cout << endl << "ERROR: unknown error code " << endl;
		break;
	}
}



//--------------------------------------------------------------------------------
ServiceManager::eComStatus ServiceManager::service_F_SNAPSHOT(CSerialPort& serialPort)
{
	// display message
	//Console.Write(System.Environment.NewLine + "Updating system files...");

	// create service request packet
	CPacket clPacket(F_SNAPSHOT, 0);

	// send packet
	if(sendPacket(serialPort, clPacket) == false)
		return ecsError;

	// store serialport timeout
	// store serialport timeout
	DWORD dwRxTimeout, dwTxTimeout;
	if(!serialPort.GetTimeouts(dwRxTimeout, dwTxTimeout))
		return ecsError;

	// increase serialport timeout
	if(!serialPort.SetTimeouts(15000, dwTxTimeout))
		return ecsError;


	// wait for valid response
	clPacket = CPacket(E_OK, 0);
	if(!receivePacket(serialPort, E_OK, clPacket))
	{
		// restore serialport timeout
		serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

		// exit
		return ecsError;
	}

	// restore serialport timeout
	serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

	// success
	return ecsOKFinish;
//	Console.Write("Success!" + System.Environment.NewLine);
}




ServiceManager::eComStatus ServiceManager::service_F_LIST(CSerialPort& serialPort, CArray<CString> *parList, CArray<UINT32> *parSize)
{
	// create service request packet
	CPacket clPacket(F_LIST, 0);

	// send packet
	if(sendPacket(serialPort, clPacket) == false)
		return ecsError;

	// wait for valid response
	// FreeSize int32
	// FreeSpace float (4Bytes)
	// Nb Files uint16
	if(true == receivePacket(serialPort, F_FILESYSTEMINFO, clPacket))
	{
		INT32 iFreeSize = clPacket.m_arData[0] << 24;
		iFreeSize += (clPacket.m_arData[1] << 16);
		iFreeSize += (clPacket.m_arData[2] << 8);
		iFreeSize += clPacket.m_arData[3];

		float fFreeSpace = (float) (clPacket.m_arData[4] << 24);
		fFreeSpace += (clPacket.m_arData[5] << 16);
		fFreeSpace += (clPacket.m_arData[6] << 8);
		fFreeSpace += clPacket.m_arData[7];

		UINT16 uiNbFiles = clPacket.m_arData[8] << 8;
		uiNbFiles += clPacket.m_arData[9];
	}
	else
		return ecsError;

	if (parList)
		parList->RemoveAll();
	if (parSize)
		parSize->RemoveAll();
	int iPos = 0;
	while(true)
	{
		// send E_OK
		clPacket = CPacket(E_OK, 0);

		// send packet
		if(sendPacket(serialPort,clPacket) == false)
			break;

		// wait for F_FILEINFO or F_DATA_COMPLETE
		if(!CPacket::readFromSerialPort(serialPort, clPacket))
			return ecsError;
		
		if(clPacket.m_eCommand == F_FILEINFO)
		{
			// Fill data into the parData
			// Get file Size
			if (parSize)
			{
				// Extract filesize
				UINT32 uiFileSize = clPacket.m_arData[0] << 24;
				uiFileSize += (clPacket.m_arData[1] << 16);
				uiFileSize += (clPacket.m_arData[2] << 8);
				uiFileSize += clPacket.m_arData[3];
				parSize->Add(uiFileSize);
			}
			// Get file name
			if (parList)
			{
				// Extract filename
				string szFileName;
				int iSize = (int)clPacket.m_arData.GetSize() - 4;
				for(int i = 0; i < iSize; i++)
					szFileName.push_back(clPacket.m_arData[4 + i]);
				CString str = CString(szFileName.c_str());
				parList->Add(str);
			}
		
			// print size and name
			//cout << "\t" << uiFileSize << "\t\t" << szFileName.c_str() << endl;
		}
		else if(clPacket.m_eCommand == F_DATA_COMPLETE)
			break;
		else
			return ecsError;
	}
	return ecsOKFinish;
}

ServiceManager::eComStatus ServiceManager::service_F_WRITE(CSerialPort& serialPort, string filename, bool fSecure, double *pdCompletion)
{
	m_bComInterruptRequested = false;
	// get filesize in bytes
	CFile clFile;
	CString wszFileName = CStringW(filename.c_str());
	if(!clFile.Open(wszFileName, CFile::modeRead | CFile::typeBinary))
	{
		//cout << "ERROR: The file " << filename.c_str() << " cannot be found!" << endl;
		return ecsError;
	}
	
	UINT32 dwFileSize = (UINT32)clFile.GetLength();
	double dFileSizeKiloBytes = ((double)dwFileSize) / 1024.0;
	int iFileKiloRounded = (int)floor(dFileSizeKiloBytes + 0.5);

	//cout << endl << "Writing file " << filename.c_str() << " to CBX HandHeld (total size: " << iFileKiloRounded << "kB)" << endl;

	// send F_WRITE
	CPacket clPacket;
	if(!CPacket::createPacket_F_WRITE(dwFileSize, RemovePath(filename), fSecure, clPacket))
		return ecsError;

	// send packet
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// wait for valid response
	clPacket = CPacket(E_OK, 0);
	if(!receivePacket(serialPort, E_OK, clPacket))
		return ecsError;

	CHiPerfTimer clTimer;
	clTimer.Start();

	// start the file transfer
	UINT32 dwTotalSizeWrite = 0;
	//cout << "File writing progress:   0%%";
	while(!m_bComInterruptRequested)
	{
		// read a from file
		CArray<BYTE> arBuffer;
		arBuffer.SetSize(CPacket::MAX_DATA_LENGTH);
		UINT uiNewSize = clFile.Read(arBuffer.GetData(), CPacket::MAX_DATA_LENGTH);
		if(uiNewSize < CPacket::MAX_DATA_LENGTH)
			int i = 0;
		
		// end of file
		if(uiNewSize == 0)
			break;
		
		arBuffer.SetSize(uiNewSize);
			
		// send F_DATA
		clPacket = CPacket(F_DATA, arBuffer);

		// send packet
		if(!sendPacket(serialPort, clPacket))
		{
			clFile.Close();
			return ecsError;
		}
		
		// wait for E_OK
		if(!CPacket::readFromSerialPort(serialPort, clPacket))
		{
			//cout << endl << "ERROR: no response from CBX (timeout)" << endl;
			clFile.Close();
			return ecsError;
		}
		
		if(clPacket.m_eCommand != E_OK)
		{
			//cout << endl << "ERROR: unexpected response" << endl;
			break;
		}
		dwTotalSizeWrite += (UINT32)arBuffer.GetSize();
		
		if (pdCompletion)
		{
			*pdCompletion =  (double)dwTotalSizeWrite / (double)dwFileSize * 100.0;
			TASApp.PumpMessages();
		}
//		char szTemp[10];
//		sprintf_s(szTemp,sizeof(szTemp), "%3u%%", UINT((double)dwTotalSizeWrite / (double)dwFileSize * 100.0));
//		cout << "\b\b\b\b" << szTemp;
	}

	//cout << endl;
	clTimer.Stop();
	clFile.Close();
	if(m_bComInterruptRequested)
		return ecsUserInterrupted;

	// send F_DATA_COMPLETE
	clPacket = CPacket(F_DATA_COMPLETE, 0);
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// store serialport timeout
	DWORD dwRxTimeout, dwTxTimeout;
	if(!serialPort.GetTimeouts(dwRxTimeout, dwTxTimeout))
		return ecsError;

	// increase serialport timeout
	if(fSecure)
	{
		if(!serialPort.SetTimeouts(10000, dwTxTimeout))
			return ecsError;
	}

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
	{
		// restore serialport timeout
		serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

		// exit
		return ecsError;
	}

	// restore serialport timeout
	serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

	// show write info
	UINT32 dwDuration = (UINT32)floor(clTimer.Duration() + 0.5);
	double dSpeed = dFileSizeKiloBytes / (double)dwDuration;
	UINT16 uiSpeedRounded = (UINT16)floor(dSpeed + 0.5);

	//cout << "Success! " << iFileKiloRounded << "kB took ";
	//cout << dwDuration << " seconds, i.e. avarage transfer speed of ";
	//cout << uiSpeedRounded << " kB/s" << endl;

	// success
	return ecsOKFinish;
}

ServiceManager::eComStatus ServiceManager::service_F_READ(CSerialPort& serialPort, string filename,
														  CString *pTrgFn, double *pdCompletion)
{
	m_bComInterruptRequested = false;
	// send F_READ
	CPacket clPacket = CPacket(F_READ, filename);

	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// wait for valid response
	if(!receivePacket(serialPort, F_FILEINFO, clPacket))
		return ecsError;

	// get filesize
	UINT32 dwExpectedFileSize = clPacket.m_arData[3];
	dwExpectedFileSize += (clPacket.m_arData[2] << 8);
	dwExpectedFileSize += (clPacket.m_arData[1] << 16);
	dwExpectedFileSize += (clPacket.m_arData[0] << 24);

	double dExpectedKiloBytes = (double)dwExpectedFileSize / 1024.0;
	int dExpectedKiloBytesRounded = (int)floor(dExpectedKiloBytes + 0.5);
	//cout << endl << "Reading file " << filename.c_str() << " from CBX HandHeld. Reported filesize is " << dExpectedKiloBytesRounded << "kB" << endl;

	// send E_OK
	clPacket = CPacket(E_OK, 0);
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	CFile clFile;
	CString wszFileName;
	if (pTrgFn)
		wszFileName = *pTrgFn;
	else
		wszFileName	= CStringW(filename.c_str());
	if(clFile.Open(wszFileName, CFile::modeWrite | CFile::typeBinary | CFile::modeCreate) == 0)
		return ecsError;
	
	CHiPerfTimer clTimer;
	clTimer.Start();
	
	UINT32 dwTotalSizeRead = 0;
	//cout << "File reading progress:   0%%";
	while(!m_bComInterruptRequested)
	{
		// wait for F_DATA
		if(!CPacket::readFromSerialPort(serialPort, clPacket))
		{
			//cout << endl << "ERROR: no response from CBX (timeout)" << endl;
			clFile.Close();
			return ecsError;
		}
	
		if(clPacket.m_eCommand != F_DATA)
			break;
		
		dwTotalSizeRead += (UINT32)clPacket.m_arData.GetSize();
		if (pdCompletion)
			*pdCompletion =  (double)dwTotalSizeRead / (double)dwExpectedFileSize * 100.0;
		TASApp.PumpMessages();
		//char szTemp[10];
		//sprintf_s(szTemp,sizeof(szTemp), "%3u%%", UINT((double)dwTotalSizeRead / (double)dwExpectedFileSize * 100.0));
		//cout << "\b\b\b\b" << szTemp;

		// write m_arData to file
		clFile.Write(clPacket.m_arData.GetData(), (UINT)clPacket.m_arData.GetSize());

		// send E_OK
		clPacket = CPacket(E_OK, 0);
		if(!clPacket.writeToSerialPort(serialPort))
		{
			//cout << endl << "ERROR: cannot read from CBX (timeout)" << endl;
			clFile.Close();
			return ecsError;
		}
	}

	//cout << endl;
	clTimer.Stop();
	clFile.Close();
	
	if(m_bComInterruptRequested)
		return ecsUserInterrupted;

	// get actual filesize in bytes
	// PS: 'CFile::shareDenyNone' must be added because without that there can be sharing violation with task currently
	//     closing clFile above!!!
	if(!clFile.Open(wszFileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyNone ))
	{
		//cout << "ERROR: File was not created correctly!" << endl;
		return ecsError;
	}
	
	UINT32 dwFileSize = (UINT32)clFile.GetLength();
	double dFileSizeKiloBytes = ((double)dwFileSize) / 1024.0;
	
	if(dwFileSize != dwExpectedFileSize)
	{
		//cout << "ERROR: Reported filesize not correct!" << endl;
		//cout << "Actual filesize = " << dwFileSize << " bytes" << endl;
		//cout << "Filesize reported by CBX = " << dwExpectedFileSize << " bytes" << endl;
		return ecsError;
	}

	UINT16 uiFileSizeKiloBytes = (UINT16)floor(dFileSizeKiloBytes + 0.5);
	UINT32 dwDuration = (UINT32)floor(clTimer.Duration() + 0.5);
	double dSpeed = dFileSizeKiloBytes / (double)dwDuration;
	UINT16 uiSpeedRounded =(UINT16)floor(dSpeed + 0.5);

	//cout << "Success! " << uiFileSizeKiloBytes;
	//cout << "kB took " << dwDuration + " seconds, i.e. avarage transfer speed of ";
	//cout << uiSpeedRounded << " kB/s" << endl;

	return ecsOKFinish;
}

ServiceManager::eComStatus ServiceManager::service_F_ADD_CRC(CString filename)
{
	// get file info
	CFile clFile;
	CString wszFileName = filename;//CStringW(filename.c_str());
	if(!clFile.Open(wszFileName, CFile::modeReadWrite | CFile::typeBinary))
	{
		//cout << "ERROR: The file " << filename.c_str() << " cannot be found!" << endl;
		return ecsError;
	}

	// get file size in bytes
	UINT32 uiFileSize = (UINT32)clFile.GetLength();

	// sanity check
	if(uiFileSize < 3)
	{
		// print error message
		//cout << "ERROR: File too small, must be larger then 3 bytes!" << endl;
		// exit
		return ecsError;
	}
	// temp variable
	BYTE bTemp;//, bBuffer;
	// File Buffer
	BYTE *pFBuf = new (BYTE[uiFileSize+2]);
	BYTE *pOrgFBuf = pFBuf;
	if (!pFBuf) 
		return ecsError;
	clFile.Read(pFBuf, uiFileSize);
	clFile.Seek(CFile::begin,CFile::begin);
	// init CRC
	UINT16 uiCRCWord = 0xFFFF;
	UINT32 uiFileSizeWork = uiFileSize;
	// for all bytes except the 2 last bytes
	while(uiFileSizeWork-- > 2)
	{
		bTemp = (BYTE)(*pFBuf ^ ((BYTE)uiCRCWord));
		uiCRCWord >>= 8;
		uiCRCWord ^= wCRCTable[bTemp];
		pFBuf++;
	}
	// read the last two bytes
	BYTE lastBytes[2];
	lastBytes[0]=*pFBuf;
	lastBytes[1]=*(++pFBuf);

	// check if file already has a valid CRC
	if((UINT16)((UINT16)lastBytes[0] << 8 | (UINT16)lastBytes[1] << 0) == uiCRCWord)
	{
		// close file & read stream
		clFile.Close();

		// print message
		//cout << endl << "File already has a valid CRC!" << endl;

		// exit
		if (pOrgFBuf) 
			delete[] pOrgFBuf;
		return ecsError;
	}
	// add the last two bytes in CRC
	for(int i = 0; i < 2; i++)
	{
		bTemp = (BYTE)(lastBytes[i] ^ ((BYTE)uiCRCWord));
		uiCRCWord >>= 8;
		uiCRCWord ^= wCRCTable[bTemp];
	}

	// set writer to file end
	clFile.SeekToEnd();

	// write CRC, big endian
	bTemp = (BYTE)(uiCRCWord >> 8);
	clFile.Write(&bTemp, 1);
	bTemp = (BYTE)uiCRCWord;
	clFile.Write(&bTemp, 1);

	// close stream
	clFile.Close();

	// Free memory
	if (pOrgFBuf) 
		delete[] pOrgFBuf;

	return ecsOKFinish;
	//cout << "Success!" << endl;
}
ServiceManager::eComStatus ServiceManager::service_GET_TIME(CSerialPort& serialPort,CArray<BYTE> *parData)
{
	// create service request packet
	CPacket clPacket = CPacket(GET_TIME, 0);
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// wait for valid response
	if(!receivePacket(serialPort, GET_TIME, clPacket))
		return ecsError;

	if (parData)
	{
		if (!clPacket.m_arData.GetSize())
			return ecsError;			// Expected Data not received! Null Modem
		parData->RemoveAll();
		parData->Copy(clPacket.m_arData);
		return ecsOKFinish;
	}
	else
	{
		// get UTC time
		UINT32 uiUNIXTime = ((UINT32)clPacket.m_arData[0] | ((UINT32)clPacket.m_arData[1] << 8) | ((UINT32)clPacket.m_arData[2] << 16) | ((UINT32)clPacket.m_arData[3] << 24));
		
		struct tm ptm;
		_gmtime32_s(&ptm,(__time32_t *)&uiUNIXTime);
		SYSTEMTIME systemTime;
		systemTime.wYear = ptm.tm_year + 1900;
		systemTime.wMonth = ptm.tm_mon + 1;
		systemTime.wDay = ptm.tm_mday;
		systemTime.wHour = ptm.tm_hour;
		systemTime.wMinute = ptm.tm_min;
		systemTime.wSecond = ptm.tm_sec;
		systemTime.wDayOfWeek = ptm.tm_wday;
		systemTime.wMilliseconds = 0;
		
		// Get date
		wchar_t wszTemp[256];
		if(GetDateFormat(	NULL,					// locale
							DATE_SHORTDATE,			// options
							&systemTime,			// date
							NULL,					// date format
							wszTemp,				// formatted string buffer
							20						// size of buffer
						) == 0)
			return ecsOKFinish;

		// print date
		//_tprintf(_T("\n%s"), wszTemp);
		
		// Get time
		if(GetTimeFormat(	NULL,					// locale
							LOCALE_NOUSEROVERRIDE,	// options
							&systemTime,			// time
							NULL,					// time format string
							wszTemp,				// formatted string buffer
							20						// size of string buffer
						) == 0)
			return ecsOKFinish;
		return ecsOKFinish;		
		// print date
		//_tprintf(_T(" %s\n"), wszTemp);
	}
}
ServiceManager::eComStatus ServiceManager::service_GET_SOFTWARE_VERSIONS(CSerialPort& serialPort,CArray<BYTE> *parData)
{
    // create service request packet
	CPacket clPacket = CPacket(GET_SOFTWARE_VERSIONS, 0);

	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// wait for valid response
	if(!receivePacket(serialPort, GET_SOFTWARE_VERSIONS, clPacket))
		return ecsError;
	if (parData)
	{
		if (!clPacket.m_arData.GetSize())return ecsError;			// Expected Data not received! Null Modem
		parData->RemoveAll();
		parData->Copy(clPacket.m_arData);
		return ecsOKFinish;
	}
	else		// Console
	{
		UINT32 HHversion = (UINT32)clPacket.m_arData[0] << 0 | (UINT32)clPacket.m_arData[1] << 8 | (UINT32)clPacket.m_arData[2] << 16 | (UINT32)clPacket.m_arData[3] << 24;
		UINT32 HHZBversion = (UINT32)clPacket.m_arData[4] << 0 | (UINT32)clPacket.m_arData[5] << 8 | (UINT32)clPacket.m_arData[6] << 16 | (UINT32)clPacket.m_arData[7] << 24;
		UINT32 DPSversion = (UINT32)clPacket.m_arData[8] << 0 | (UINT32)clPacket.m_arData[9] << 8 | (UINT32)clPacket.m_arData[10] << 16 | (UINT32)clPacket.m_arData[11] << 24;

	 /*   // success
		Console.Write(System.Environment.NewLine);
		Console.WriteLine("HH    : " + HHversion.ToString("X") + System.Environment.NewLine);
		Console.WriteLine("HH-ZB : " + HHZBversion.ToString("X") + System.Environment.NewLine);
		Console.WriteLine("DPS   : " + DPSversion.ToString("X") + System.Environment.NewLine);
	*/
	}
	return ecsOKFinish;
}

ServiceManager::eComStatus ServiceManager::service_GET_COMSTATUS(CSerialPort& serialPort,CArray<BYTE> *parData)
{
    string sComStatus = "UNKNOWN";

    // create service request packet
	CPacket clPacket = CPacket(GET_COMSTATUS, 0);

    // send packet
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// wait for valid response
	if(!receivePacket(serialPort, GET_COMSTATUS, clPacket))
		return ecsError;

	if (parData)
	{
		if (!clPacket.m_arData.GetSize())return ecsError;			// Expected Data not received! Null Modem
		parData->RemoveAll();
		parData->Copy(clPacket.m_arData);
	}
	return ecsOKFinish;

/*  // Convert status
    if (p.data[0] == 0)
        sComStatus = "NOT CONNECTED";
    else if (p.data[0] == 1)
        sComStatus = "WIRELESS";
    else if (p.data[0] == 2)
        sComStatus = "CABLE";


    // print com status
    Console.WriteLine(System.Environment.NewLine + "DPS com status: " + sComStatus + System.Environment.NewLine);
*/
}
ServiceManager::eComStatus ServiceManager::service_RUN_GENERATE_METADATA(CSerialPort& serialPort)
{
    // create service request packet
	CPacket clPacket = CPacket(RUN_GENERATE_METADATA, 0);
    // send packet
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// store serialport timeout
	DWORD dwRxTimeout, dwTxTimeout;
	if(!serialPort.GetTimeouts(dwRxTimeout, dwTxTimeout))
		return ecsError;

	// increase serialport timeout
	if(!serialPort.SetTimeouts(10000, dwTxTimeout))
		return ecsError;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
	{
		// restore serialport timeout
		serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);
		
		return ecsError;
	}

	// restore serialport timeout
	serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

    // success
//    Console.Write(System.Environment.NewLine + "Success!" + System.Environment.NewLine);
	return ecsOKFinish;
}



ServiceManager::eComStatus ServiceManager::service_RUN_REBOOT(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(RUN_REBOOT, 0);
	if(!sendPacket(serialPort, clPacket))
		return ecsError;
	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return ecsError;
	// success
	//cout << endl << "Success!" << endl;
	return ecsOKFinish;

}

ServiceManager::eComStatus ServiceManager::service_RUN_REBOOT_ZBHH(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(RUN_REBOOT_ZBHH, 0);
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return ecsError;

	// success
	//cout << endl << "Success!" << endl;
	return ecsOKFinish;
}

ServiceManager::eComStatus ServiceManager::service_RUN_UPDATE_ZBHH(CSerialPort& serialPort, string filename, double *pdCompletion/*=NULL*/)
{
	// send the file to cbx
//	if(service_F_WRITE(serialPort, filename, false) == false)
//		return ecsError;

	// set calibration image
	//cout << endl << "Establishing connection with bootloader...";

	// create service request packet
	CPacket clPacket = CPacket(RUN_UPDATE_ZBHH, RemovePath(filename));
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// create valid responses
	ProtocolCommand pExpectedCommands[2];
	pExpectedCommands[0] = E_OK;
	pExpectedCommands[1] = E_FLASHING;

	// store serialport timeout
	DWORD dwRxTimeout, dwTxTimeout;
	if(!serialPort.GetTimeouts(dwRxTimeout, dwTxTimeout))
		return ecsError;

	// increase serialport timeout
	if(!serialPort.SetTimeouts(5000, dwTxTimeout))
		return ecsError;

	// established connection with bootloader flag
	bool establishedBootConnection = false;

	// until done
	while(true)
	{
		// wait for valid responses
		if(!receivePacket(serialPort, pExpectedCommands, 2, clPacket))
		{
			// delete the file
			//service_F_DELETE(serialPort, RemovePath(filename));

			// restore serialport timeout
			serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

			// exit
			return ecsError;
		}
		
		// check if done
		if(clPacket.m_eCommand == E_OK)
			break;

		// connection established
		if(establishedBootConnection == false)
		{
			// connection established
			establishedBootConnection = true;

			// success -> download started
			//cout << "Success!" << endl << endl << "Downloading FLASH image to target";
		}
		
		// show progress
		//cout << ".";
		if (pdCompletion) 
		{
			(*pdCompletion) ++;
			TASApp.PumpMessages();
		}
	}
	
	// restore serialport timeout
	serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

	// delete the file
	//service_F_DELETE(serialPort, RemovePath(filename));
	// success
	//cout << "Success!" << endl;
	return ecsOKFinish;

}
ServiceManager::eComStatus ServiceManager::service_RUN_SENSOR_REBOOT(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(RUN_SENSOR_REBOOT, 0);
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return ecsError;

	// success
	//cout << endl << "Success!" << endl;
	return ecsOKFinish;
}

ServiceManager::eComStatus ServiceManager::service_RUN_SENSOR_UPDATE(CSerialPort& serialPort, string filename,  double *pdCompletion/*=NULL*/)
{
	// send the file to cbx
	//if(service_F_WRITE(serialPort, filename, false) == false)
	//	return ecsError;

	// show message
	//cout << endl << "To enter bootloader mode, please hold down the ON/OFF button on the DPS" << endl;
	//cout << endl << "until the red LED is ON, then release it immediately." << endl;
	//cout << endl << endl << "Establishing connection with bootloader...";

	service_RUN_SENSOR_REBOOT(serialPort);
	// create service request packet
	CPacket clPacket = CPacket(RUN_SENSOR_UPDATE, RemovePath(filename));
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// create valid responses
	ProtocolCommand pExpectedCommands[2];
	pExpectedCommands[0] = E_OK;
	pExpectedCommands[1] = E_FLASHING;

	// store serialport timeout
	DWORD dwRxTimeout, dwTxTimeout;
	if(!serialPort.GetTimeouts(dwRxTimeout, dwTxTimeout))
		return ecsError;

	// increase serialport timeout
	if(!serialPort.SetTimeouts(20000, dwTxTimeout))
		return ecsError;

	// established connection with bootloader flag
	bool establishedBootConnection = false;

	// until done
	while(true)
	{
		// wait for valid responses
		if(!receivePacket(serialPort, pExpectedCommands, 2, clPacket))
		{
			// delete the file
			service_F_DELETE(serialPort, RemovePath(filename));

			// restore serialport timeout
			serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

			// exit
			return ecsError;
		}
		
		// check if done
		if(clPacket.m_eCommand == E_OK) 
			break;

		// connection established
		if(establishedBootConnection == false)
		{
			// connection established
			establishedBootConnection = true;

			// success -> download started
			//cout << "Success!" << endl << endl << "Downloading FLASH image to target";
		}
		
		// show progress
		//cout << ".";
		if (pdCompletion)
		{
			(*pdCompletion) ++;
			TASApp.PumpMessages();
		}
	}
	
	// restore serialport timeout
	serialPort.SetTimeouts(dwRxTimeout, dwTxTimeout);

	// delete the file
	// service_F_DELETE(serialPort, RemovePath(filename));
	// success
	//cout << "Success!" << endl;
	return ecsOKFinish;
}

ServiceManager::eComStatus ServiceManager::service_RUN_DISCHARGEPROJECT(CSerialPort& serialPort)
{
	// create service request packet
	CPacket clPacket = CPacket(RUN_DISCHARGE_PROJECT, 0);
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return ecsError;
    // success
    //Console.Write(System.Environment.NewLine + "Success!" + System.Environment.NewLine);
	return ecsOKFinish;
}

ServiceManager::eComStatus ServiceManager::service_RUN_GOTOMAINMENU(CSerialPort& serialPort)
{
    // create service request packet
	CPacket clPacket = CPacket(RUN_GOTO_MAINMENU_AND_WAITFORCOM, 0);
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return ecsError;

    // success
    //Console.Write(System.Environment.NewLine + "Success!" + System.Environment.NewLine);
	return ecsOKFinish;
}

ServiceManager::eComStatus ServiceManager::service_RUN_ERASEDATASTRUCT(CSerialPort& serialPort, wstring iD)
{
     // create service request packet
	CPacket clPacket = CPacket(RUN_ERASE_DATASTRUCTOBJECT, 0);
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return ecsError;

    // success
    //Console.Write(System.Environment.NewLine + "Success!" + System.Environment.NewLine);
	return ecsOKFinish;
}

ServiceManager::eComStatus ServiceManager::service_RUN_CLEANMAINDATASTRUCT(CSerialPort& serialPort)
{
     // create service request packet
	CPacket clPacket = CPacket(RUN_CLEAN_MAINDATASTRUCT, 0);
	if(!sendPacket(serialPort, clPacket))
		return ecsError;

	// wait for valid response
	if(!receivePacket(serialPort, E_OK, clPacket))
		return ecsError;
    // success
    //Console.Write(System.Environment.NewLine + "Success!" + System.Environment.NewLine);
	return ecsOKFinish;
}

