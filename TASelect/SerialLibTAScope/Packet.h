#pragma once
#include<string>
using namespace std;
class CSerialPort;
namespace TASCOPE
{
	enum ProtocolCommand
	{
		/* Client Commands */
		F_LIST = 0x10,
		F_READ = 0x11,
		F_WRITE = 0x12,
		F_DELETE = 0x13,
		F_FORMAT = 0x14,
		F_DELETE_BOOTLOADER = 0x15,
		F_WRITE_SECURE = 0x16,
        F_SAVE_IN_DATAFLASH = 0x17,
	    F_RECALL_FROM_DATAFLASH = 0x18,
		F_ERASE_NANDFLASH = 0x20,
        F_DELETE_MULTIPLE = 0x21,
        F_SNAPSHOT = 0x22,
		/* Server Commands */
		F_FILESYSTEMINFO = 0x50,
		F_FILEINFO = 0x51,
		F_DATA = 0x52,
		F_DATA_COMPLETE = 0x53,
		/* Aftermarket & Production Specific Commands */
		SET_TIME = 0x70,
		GET_TIME = 0x71,
		SET_PANID = 0x72,
		GET_PANID = 0x73,
		SET_SENSOR_MAC = 0x74,
		GET_SENSOR_MAC = 0x75,
		SET_INTERNAL_I2C_ON = 0x76,
		SET_INTERNAL_I2C_OFF = 0x77,
		SET_SENSOR_INTERNAL_I2C_ON = 0x78,
		SET_SENSOR_INTERNAL_I2C_OFF = 0x79,
		SET_EXTERNAL_PWR_ON = 0x7A,
		SET_EXTERNAL_PWR_OFF = 0x7B,
		GET_RAW_DATA = 0x7C,
		SET_CALIB_DP = 0x7D,
		SET_CALIB_DTS = 0x7E,
		GET_CALIB_DP = 0x7F,
		GET_CALIB_DTS = 0x80,
		GET_SENSOR_MAGNET = 0x81,
		SET_KEYBOARD = 0x82,
		GET_KEYBOARD = 0x83,
		SET_SENSOR_LED = 0x84,
		RUN_SLEEP = 0x85,
		RUN_BUZZER = 0x86,
		RUN_REBOOT = 0x87,
		RUN_REBOOT_ZBHH = 0x88,
		RUN_UPDATE_ZBHH = 0x89,
		RUN_DIAGNOSTICS = 0x8A,
		RUN_SENSOR_REBOOT = 0x8B,
		RUN_SENSOR_UPDATE = 0x8C,
		RUN_SENSOR_DIAGNOSTICS = 0x8D,
		SET_SENSOR_BATTERY_INSTAL_DATE = 0x8E,
		GET_SENSOR_BATTERY_INSTAL_DATE = 0x8F,
		SET_BATTERY_INSTAL_DATE = 0x90,
		GET_BATTERY_INSTAL_DATE = 0x91,
		RUN_BATTERY_IMPEDANCE_TRACK = 0x92,
		RUN_SENSOR_BATTERY_IMPEDANCE_TRACK = 0x93,
		GET_T3_DATA = 0x94,
		GET_PHYSICAL_DATA = 0x95,
        SET_LANGUAGE = 0x96,
        RUN_GENERATE_METADATA = 0x97,
        GET_SOFTWARE_VERSIONS = 0x98,
        GET_COMSTATUS = 0x99,
	    RUN_DISCHARGE_PROJECT = 0x9A,
	    RUN_GOTO_MAINMENU_AND_WAITFORCOM = 0x9B,
	    RUN_ERASE_DATASTRUCTOBJECT = 0x9C,
	    RUN_CLEAN_MAINDATASTRUCT = 0x9D,
        /* Acknowledge Commands */
	    E_OK = 0xA0,
	    E_ERROR = 0xA1,
        E_FLASHING = 0xA2
    };


	typedef enum
	{
		NONE = 0,
		/* Generic */
		INPUT = 0x10,
		RESPONSE = 0x11,
		UNSUPPORTED = 0x13,
		CHECKSUM = 0x14,
		FILESYSTEM = 0x15,
		INTERNAL = 0x16,
		UNSUPPORTED_BOOT = 0x17,
		INVALID_CRC = 0x18,
		/* Sensor Related */
		OFFLINE = 0x50,
		OFFLINE_RS485 = 0x51
	}ErrorCode;

	class CPacket
	{
	public:
		CPacket(){};
		CPacket(int size);
		CPacket(ProtocolCommand eCommand, int iSize);
		CPacket(ProtocolCommand eCommand, CArray<BYTE>& arData);
		CPacket(ProtocolCommand eCommand, string text);
		CPacket(const CPacket &clPacket);
		~CPacket();
		
		CPacket &operator=(CPacket &clPacket);

		bool writeToSerialPort(CSerialPort& serialPort);
		UINT16 calculateChecksum();
        static bool readFromSerialPort(CSerialPort& serialPort, CPacket& clPacket);
        static void StrToByteArray(string str, CArray<BYTE>& arData);
		static bool createPacket_F_WRITE(UINT32 uiFileSize, string filename, bool fSecure, CPacket& clPacket);
   
	public:
		ProtocolCommand m_eCommand;
        CArray<BYTE> m_arData;
        UINT16 m_uiCheckSum;
        BYTE m_bNrPadBytes;

		// Max payload length
		static const int MAX_DATA_LENGTH = 4096;

		/// <summary>
		/// Packet structure:
		/// 
		/// Command:        1 byte
		/// Data Size:      2 bytes
		/// Data:           0 - 65 535 bytes
		/// Padding Size:   1 byte
		/// Padding:        0 - 255 bytes
		/// Checksum:       2 bytes
		/// </summary>
	};
}
