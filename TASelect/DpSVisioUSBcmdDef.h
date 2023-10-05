/**
* \file 		USBCmdDef.h
* \brief		List all USB Messages
* \author		Ech
* \version		1.0
* \date			24/02/16
*
*/

#ifndef USB_CMDDEF_H_
#define USB_CMDDEF_H_

#define USBD_CUSTOMHID_OUTREPORT_SENDSIZE	65
#define USBD_CUSTOMHID_OUTREPORT_BUF_SIZE	64
#define DPSVISIOERROR_SIZE					2

///////////////////////////////////////////////////////////////////////////////////////////////
// MESSAGE STRUCTURE
///////////////////////////////////////////////////////////////////////////////////////////////

typedef UINT16							DPSVISIOCMD_FULLCMDTYPE;        // It's DPSVISIOCMD_CMDCATEGTYPE + DPSVISIOCMD_CMDTYPE
typedef UINT8							DPSVISIOCMD_CMDCATEGTYPE;
typedef UINT8							DPSVISIOCMD_CMDTYPE;
typedef UINT8							DPSVISIOCMD_DATALENTYPE;

#define DPSVISIOCMD_CMDCATEGSIZE        ( sizeof( DPSVISIOCMD_CMDCATEGTYPE ) )
#define DPSVISIOCMD_CMDSIZE				( sizeof( DPSVISIOCMD_CMDTYPE ) )
#define DPSVISIOCMD_FULLCMDSIZE			( sizeof( DPSVISIOCMD_FULLCMDTYPE ) )
#define DPSVISIOCMD_DATALENSIZE			( sizeof( DPSVISIOCMD_DATALENTYPE ) )
#define DPSVISIOCMD_DATAMAXSIZE			( USBD_CUSTOMHID_OUTREPORT_BUF_SIZE - DPSVISIOCMD_CMDCATEGSIZE - DPSVISIOCMD_CMDSIZE - DPSVISIOCMD_DATALENSIZE )

#define DPSVISIOCMD_BASESIZE			( DPSVISIOCMD_CMDCATEGSIZE + DPSVISIOCMD_CMDSIZE  + DPSVISIOCMD_DATALENSIZE)
#define DPSVISIOCMD_TOTALSIZE			( DPSVISIOCMD_CMDCATEGSIZE + DPSVISIOCMD_CMDSIZE + DPSVISIOCMD_DATALENSIZE + DPSVISIOCMD_DATAMAXSIZE )

#define DPSVISIOCMD_CMDCATEGINDEX		0x00
#define DPSVISIOCMD_CMDINDEX			( DPSVISIOCMD_CMDCATEGINDEX + DPSVISIOCMD_CMDCATEGSIZE )
#define DPSVISIOCMD_DATALENINDEX		( DPSVISIOCMD_CMDINDEX + DPSVISIOCMD_CMDSIZE )
#define DPSVISIOCMD_DATAINDEX			( DPSVISIOCMD_DATALENINDEX + DPSVISIOCMD_DATALENSIZE )

#define CMD_CMDSIZE						( sizeof( UINT16 ) )
#define CMD_DATALENSIZE					( sizeof( DPSVISIOCMD_DATALENTYPE ) )
#define CMD_DATAMAXSIZE					( USBD_CUSTOMHID_OUTREPORT_BUF_SIZE - CMD_CMDSIZE - CMD_DATALENSIZE )

///////////////////////////////////////////////////////////////////////////////////////////////
// TO PC
// 0x80xx
///////////////////////////////////////////////////////////////////////////////////////////////
#define USBCMD_NACK								0x8000
#define USBCMD_ACK								0x8001
#define USBCMD_TEXTMSG							0x8002
#define USBCMD_ACKDATA							0x8003

///////////////////////////////////////////////////////////////////////////////////////////////
// TO DGL
// 0x03xx define in file DongleCmdDef.f
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
// System
// 0x00
///////////////////////////////////////////////////////////////////////////////////////////////
#define USBCMD_JUMPTODFU				0xFFFF
#define USBCMD_READ_TXT_MSG 			0x0000
#define USBCMD_GETFULLSN				0x0002
#define USBCMD_GETSHORTSN				0x0003
#define USBCMD_GETFIRMWARE				0x0007

#define UPGFW_STARTSEND					0x0361
#define UPGFW_SENDVERSION				0x0362
#define UPGFW_SENDONEPACKET				0x0363
#define UPGFW_SENDCRC					0x0364
#define UPGFM_PROGRAM					0x0365

///////////////////////////////////////////////////////////////////////////////////////////////
// DPS2 Test commands 0xFCxx
// if parameter or numeric answer is present: 4 bytes Big Indian
///////////////////////////////////////////////////////////////////////////////////////////////
#define USBCMD_TEST_DPS2				0xFC00		// Used to categorize message in main interpreter

#define T_SET_PRODUCTIONMODE			0xFC00		// B[3]: production off (0)/on (1) - Answer: ACK/NACK
#define T_SET_MONITOR					0xFC01		// B[3]: monitor off (0)/on (1) - Answer: ACK/NACK

#define T_DISPLAY_ON					0xFC02		// Answer: ACK/NACK
#define T_DISPLAY_OFF					0xFC03		// Answer: ACK/NACK
#define T_DISPLAY_LUMI					0xFC04		// B[3]: 0...7 - Answer: ACK/NACK

#define T_SET_LED_ON					0xFC05		// B[3]: 0(R), 1(G), 2(B) - Answer: ACK/NACK
#define T_SET_LED_OFF					0xFC06		// B[3]: 0(R), 1(G), 2(B) - Answer: ACK/NACK

#define T_GET_BUTTON					0xFC07		// Answer: Bits 0,1,2
#define T_GET_VUSB						0xFC08		//	V Usb (mV)
#define T_GET_VDISP						0xFC09		//	V Display (mV)
#define T_GET_VMOTOR					0xFC0A		//	V Motor (mV)

#define T_SLEEP							0xFC0B		// Answer: ACK/NACK

#define T_CHECK_BLE						0xFC0C		// Answer: ACK/NACK
#define T_CHECK_ZB						0xFC0D		// Answer: ACK/NACK
#define T_CHECK_CHARGER					0xFC0E		// Answer: ACK/NACK
#define T_CHECK_FUELGAUGE				0xFC0F		// Answer: ACK/NACK
#define T_CHECK_EEPROM_DP				0xFC10		// Answer: ACK/NACK
#define T_CHECK_EEPROM_LOG				0xFC11		// Answer: ACK/NACK

#define T_GET_DP						0xFC12		// Dp Raw
#define T_GET_DTS1						0xFC13		// T1 Raw
#define T_GET_DTS2						0xFC14		// T2 Raw

#define T_BUTTON_SIM					0xFC15		// B[3]: Button pressure code - Answer: ACK/NACK

#define T_GET_DP_PHYS					0xFC16		// Dp Physical, offset corrected
#define T_GET_DTS1_PHYS					0xFC17		// T1 Physical
#define T_GET_DTS2_PHYS					0xFC18		// T2 Physical

#define T_CLEAR_EEPROM_DP				0xFC19		// Answer: ACK/NACK
#define T_CLEAR_EEPROM_MAIN				0xFC1A		// Answer: ACK/NACK

#define T_SET_MOTOR_CONTROL				0xFC20		// B[3]: 0 - Open, 1 - Close, 2 - Loop fast, 3 - Loop slow - Answer: ACK/NACK
#define T_SET_MOTOR_CURRENT 			0xFC21		// B[3]: 0...15 - Answer: ACK/NACK
#define T_SET_MOTOR_SPEED 				0xFC22		// B[3]: 0...7 - Answer: ACK/NACK
#define T_SET_MOTOR_ACCEL				0xFC23		// B[3]: 0...7 - Answer: ACK/NACK
#define T_SET_MOTOR_END					0xFC24		// B[3]: 0...7 - Answer: ACK/NACK

#define T_SET_CHARGER_CURRENT			0xFC30		// B[2,3]:
#define T_CHECK_EEPROM_DTS1				0xFC31		// Answer: ACK/NACK
#define T_CHECK_EEPROM_DTS2				0xFC32		// Answer: ACK/NACK
#define T_GET_BATTERY_STATUS			0xFC33		// B[3]: 0 - 100 mA; 1 - 500 mA

#define T_GET_STATISTICS_1				0xFC34		// Statistics part 1
#define T_GET_STATISTICS_2				0xFC35		// Statistics part 2
#define T_CLEAR_STATISTICS				0xFC36		// Clear Statistics in non volatile memory
#define T_GET_CALIBARRAY				0xFC37		// Calibration array
#define T_GET_SERIAL_NBR				0xFC38		// Serial number

///////////////////////////////////////////////////////////////////////////////////////////////
// UnitTests 0xFExx
///////////////////////////////////////////////////////////////////////////////////////////////
#define USBCMD_UNITTEST					0xFE00		// Used to categorize message in main interpreter

#endif /* CMDDEF_H_ */
