#pragma once

#include "SerialPort.h"
#include "Packet.h"

using namespace std;
namespace TASCOPE 
{
	class ServiceManager
	{
	public:	
		typedef enum
		{
			ecsError = 0,
			ecsUserInterrupted,
			ecsOKFinish,
			ecsOKContinue,			// used for downloading file for instance
			ecsUndef,
			ecsLast
		}eComStatus;
		typedef enum
		{
			eDPcsNotServiceNotSupported = -1,
			eDPcsNotConnected = 0,
			eDPcsWireless = 1,
			eDPcsWired = 2,
			eDPcsLast
		}eDPSComStatus;
	public: 
		ServiceManager(){m_LastErrorCode = NONE;};
		static string RemovePath(string filename);
		static void service_F_ERASE_NANDFLASH(CSerialPort& serialPort);
		static void service_F_FORMAT(CSerialPort& serialPort);
		static void service_F_DELETE_BOOTLOADER(CSerialPort& serialPort);
		static void service_SET_TIME(CSerialPort& serialPort);
		static void service_SET_PANID(CSerialPort& serialPort, UINT64 panId);
		static void service_GET_PANID(CSerialPort& serialPort);
		static void service_SET_SENSOR_MAC(CSerialPort& serialPort, UINT64 mac);
		static void service_GET_SENSOR_MAC(CSerialPort& serialPort);
		static void service_SET_INTERNAL_I2C_ON(CSerialPort& serialPort);
		static void service_SET_INTERNAL_I2C_OFF(CSerialPort& serialPort);
		static void service_SET_SENSOR_INTERNAL_I2C_ON(CSerialPort& serialPort);
		static void service_SET_SENSOR_INTERNAL_I2C_OFF(CSerialPort& serialPort);
		static void service_SET_EXTERNAL_PWR_ON(CSerialPort& serialPort);
		static void service_SET_EXTERNAL_PWR_OFF(CSerialPort& serialPort);
		static void service_GET_RAW_DATA(CSerialPort& serialPort);
		static void service_GET_PHYSICAL_DATA(CSerialPort& serialPort);
		static void service_GET_T3_DATA(CSerialPort& serialPort);
		static void service_SET_CALIB_DP(CSerialPort& serialPort, string filename);
		static void service_SET_CALIB_DTS(CSerialPort& serialPort, string filename);
		static void service_GET_CALIB_DP(CSerialPort& serialPort, string filename);
		static void service_GET_CALIB_DTS(CSerialPort& serialPort, string filename);
		static void service_GET_SENSOR_MAGNET(CSerialPort& serialPort);
		static void service_SET_KEYBOARD(CSerialPort& serialPort, BYTE value);
		static void service_GET_KEYBOARD(CSerialPort& serialPort);
		static void service_SET_SENSOR_LED(CSerialPort& serialPort, BYTE value);
		static void service_RUN_SLEEP(CSerialPort& serialPort);
		static void service_RUN_BUZZER(CSerialPort& serialPort);
		static void service_RUN_DIAGNOSTICS(CSerialPort& serialPort);
		static void service_RUN_SENSOR_DIAGNOSTICS(CSerialPort& serialPort);
		static void service_SET_BATTERY_INSTAL_DATE(CSerialPort& serialPort, UINT32 valueUTC);
		static void service_GET_BATTERY_INSTALLATION_DATE(CSerialPort& serialPort);
		static void service_SET_SENSOR_BATTERY_INSTAL_DATE(CSerialPort& serialPort, UINT32 valueUTC);
		static void service_GET_SENSOR_BATTERY_INSTALLATION_DATE(CSerialPort& serialPort);
		static void service_RUN_BATTERY_IMPEDANCE_TRACK(CSerialPort& serialPort);
		static void service_RUN_SENSOR_BATTERY_IMPEDANCE_TRACK(CSerialPort& serialPort);
		static bool sendPacket(CSerialPort& serialPort, CPacket& clPacket);
		
		static ServiceManager::eComStatus service_F_ADD_CRC(CString filename);
		static ServiceManager::eComStatus service_F_DELETE(CSerialPort& serialPort, string filename);
		static ServiceManager::eComStatus service_F_LIST(CSerialPort& serialPort, CArray<CString> *parList, CArray<UINT32> *parSize);
		static ServiceManager::eComStatus service_F_WRITE(CSerialPort& serialPort, string filename, bool secure, double *pdCompletion=NULL);
		static ServiceManager::eComStatus service_F_READ(CSerialPort& serialPort, string filename,CString *pTrgFn=NULL, double *pdCompletion=NULL);
		static ServiceManager::eComStatus service_F_SNAPSHOT(CSerialPort& serialPort);

		static ServiceManager::eComStatus service_RUN_DISCHARGEPROJECT(CSerialPort& serialPort);
		static ServiceManager::eComStatus service_RUN_GOTOMAINMENU(CSerialPort& serialPort);
		static ServiceManager::eComStatus service_RUN_ERASEDATASTRUCT(CSerialPort& serialPort, wstring iD);
		static ServiceManager::eComStatus service_RUN_CLEANMAINDATASTRUCT(CSerialPort& serialPort);
		static ServiceManager::eComStatus service_RUN_REBOOT(CSerialPort& serialPort);
		static ServiceManager::eComStatus service_RUN_SENSOR_REBOOT(CSerialPort& serialPort);
		static ServiceManager::eComStatus service_RUN_REBOOT_ZBHH(CSerialPort& serialPort);
		static ServiceManager::eComStatus service_RUN_UPDATE_ZBHH(CSerialPort& serialPort, string filename, double *pdCompletion=NULL);
		static ServiceManager::eComStatus service_RUN_SENSOR_UPDATE(CSerialPort& serialPort, string filename, double *pdCompletion=NULL);
		static ServiceManager::eComStatus service_RUN_GENERATE_METADATA(CSerialPort& serialPort);
		static ServiceManager::eComStatus service_GET_TIME(CSerialPort& serialPort,CArray<BYTE> *parData);
		static ServiceManager::eComStatus service_GET_SOFTWARE_VERSIONS(CSerialPort& serialPort,CArray<BYTE> *parData);
		static ServiceManager::eComStatus service_GET_COMSTATUS(CSerialPort& serialPort,CArray<BYTE> *parData);
		static ErrorCode service_GET_LASTERRORCODE(){return m_LastErrorCode;};
		//Stop current service used for 
		// stop Writing and reading file operation
		static void StopCurrentService(){m_bComInterruptRequested = true;};
	private:
		static ErrorCode m_LastErrorCode;
		static bool receivePacket(CSerialPort& serialPort, ProtocolCommand eExpectedCommand, CPacket& clPacket);
		static bool receivePacket(CSerialPort& serialPort, ProtocolCommand *peExpectedCommands, int iCommandSize, CPacket& clPacket);
		static bool validCommand(ProtocolCommand receivedCommand, ProtocolCommand *peExpectedCommands, int iCommandSize);
		static void printErrorCodeInfo(ErrorCode errorCode);
		
	private:
		static UINT16 wCRCTable[256];
		// Used to interrupt long communication process (Read, Write files)
		static bool m_bComInterruptRequested;
    };
}
