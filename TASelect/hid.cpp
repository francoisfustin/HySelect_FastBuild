/*******************************************************
 HIDAPI - Multi-Platform library for
 communication with HID devices.

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009, All Rights Reserved.

 At the discretion of the user of this library,
 this software may be licensed under the terms of the
 GNU Public License v3, a BSD-Style license, or the
 original HIDAPI license as outlined in the LICENSE.txt,
 LICENSE-gpl3.txt, LICENSE-bsd.txt, and LICENSE-orig.txt
 files located at the root of the source distribution.
 These files may also be found in the public source
 code repository located at:
        http://github.com/signal11/hidapi .
********************************************************/

#include "stdafx.h"
#include <windows.h>

#ifndef _NTDEF_
typedef LONG NTSTATUS;
#endif

#include <setupapi.h>
#include <winioctl.h>

// Copied from inc/ddk/hidclass.h, part of the Windows DDK.
#define HID_OUT_CTL_CODE(id)  \
		CTL_CODE(FILE_DEVICE_KEYBOARD, (id), METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_HID_GET_FEATURE                   HID_OUT_CTL_CODE(100)

#include <stdio.h>
#include <stdlib.h>
#include "hidapi.h"


#ifdef _MSC_VER
// Thanks Microsoft, but I know how to use strncpy().
#pragma warning(disable:4996)
#endif

// Since we're not building with the DDK, and the HID header
// files aren't part of the SDK, we have to define all this
// stuff here. In lookup_functions(), the function pointers
// defined below are set.
typedef struct _HIDD_ATTRIBUTES
{
	ULONG Size;
	USHORT VendorID;
	USHORT ProductID;
	USHORT VersionNumber;
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

typedef USHORT USAGE;
typedef struct _HIDP_CAPS
{
	USAGE Usage;
	USAGE UsagePage;
	USHORT InputReportByteLength;
	USHORT OutputReportByteLength;
	USHORT FeatureReportByteLength;
	USHORT Reserved[17];
	USHORT fields_not_used_by_hidapi[10];
} HIDP_CAPS, *PHIDP_CAPS;
typedef char *HIDP_PREPARSED_DATA;
#define HIDP_STATUS_SUCCESS 0x0

typedef BOOLEAN( __stdcall *HidD_GetAttributes_ )( HANDLE device, PHIDD_ATTRIBUTES attrib );
typedef BOOLEAN( __stdcall *HidD_GetSerialNumberString_ )( HANDLE device, PVOID buffer, ULONG buffer_len );
typedef BOOLEAN( __stdcall *HidD_GetManufacturerString_ )( HANDLE handle, PVOID buffer, ULONG buffer_len );
typedef BOOLEAN( __stdcall *HidD_GetProductString_ )( HANDLE handle, PVOID buffer, ULONG buffer_len );
typedef BOOLEAN( __stdcall *HidD_SetFeature_ )( HANDLE handle, PVOID data, ULONG length );
typedef BOOLEAN( __stdcall *HidD_GetFeature_ )( HANDLE handle, PVOID data, ULONG length );
typedef BOOLEAN( __stdcall *HidD_GetIndexedString_ )( HANDLE handle, ULONG string_index, PVOID buffer, ULONG buffer_len );
typedef BOOLEAN( __stdcall *HidD_GetPreparsedData_ )( HANDLE handle, HIDP_PREPARSED_DATA **preparsed_data );
typedef BOOLEAN( __stdcall *HidD_FreePreparsedData_ )( HIDP_PREPARSED_DATA *preparsed_data );
typedef BOOLEAN( __stdcall *HidP_GetCaps_ )( HIDP_PREPARSED_DATA *preparsed_data, HIDP_CAPS *caps );

static HidD_GetAttributes_ HidD_GetAttributes;
static HidD_GetSerialNumberString_ HidD_GetSerialNumberString;
static HidD_GetManufacturerString_ HidD_GetManufacturerString;
static HidD_GetProductString_ HidD_GetProductString;
static HidD_SetFeature_ HidD_SetFeature;
static HidD_GetFeature_ HidD_GetFeature;
static HidD_GetIndexedString_ HidD_GetIndexedString;
static HidD_GetPreparsedData_ HidD_GetPreparsedData;
static HidD_FreePreparsedData_ HidD_FreePreparsedData;
static HidP_GetCaps_ HidP_GetCaps;

static HMODULE lib_handle = NULL;
static BOOLEAN initialized = FALSE;

static hid_device *new_hid_device()
{
	hid_device *dev = ( hid_device * ) calloc( 1, sizeof( hid_device ) );
	dev->device_writehandle = INVALID_HANDLE_VALUE;
	dev->device_readhandle = INVALID_HANDLE_VALUE;
	dev->blocking = TRUE;
	dev->input_report_length = 0;
	dev->last_error_str = NULL;
	dev->last_error_num = 0;
	dev->read_pending = FALSE;
	dev->read_buf = NULL;
	memset( &dev->ol, 0, sizeof( dev->ol ) );
	dev->ol.hEvent = CreateEvent( NULL, FALSE, FALSE /*inital state f=nonsignaled*/, NULL );

	return dev;
}

static void register_error( hid_device *device, const char *op )
{
	WCHAR *ptr, *msg;

	FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					GetLastError(),
					MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
					( LPWSTR )&msg, 0/*sz*/,
					NULL );

	// Get rid of the CR and LF that FormatMessage() sticks at the
	// end of the message. Thanks Microsoft!
	ptr = msg;

	while( *ptr )
	{
		if( *ptr == '\r' )
		{
			*ptr = 0x0000;
			break;
		}

		ptr++;
	}

	// Store the message off in the Device entry so that
	// the hid_error() function can pick it up.
	LocalFree( device->last_error_str );
	device->last_error_str = msg;
}

static int lookup_functions()
{
	lib_handle = LoadLibraryA( "hid.dll" );

	if( lib_handle )
	{
#define RESOLVE(x) x = (x##_)GetProcAddress(lib_handle, #x); if (!x) return -1;
		RESOLVE( HidD_GetAttributes );
		RESOLVE( HidD_GetSerialNumberString );
		RESOLVE( HidD_GetManufacturerString );
		RESOLVE( HidD_GetProductString );
		RESOLVE( HidD_SetFeature );
		RESOLVE( HidD_GetFeature );
		RESOLVE( HidD_GetIndexedString );
		RESOLVE( HidD_GetPreparsedData );
		RESOLVE( HidD_FreePreparsedData );
		RESOLVE( HidP_GetCaps );
#undef RESOLVE
	}
	else
	{
		return -1;
	}

	return 0;
}

static int open_device( const char *path, hid_device *pDevice )
{
	if( NULL == path || NULL == pDevice )
	{
		return -1;
	}

	// Some devices must be opened with sharing enabled (even though they are only opened once),
	// so try it here.
	pDevice->device_readhandle = CreateFileA( path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
								 NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0 );

	if( INVALID_HANDLE_VALUE == pDevice->device_readhandle )
	{
		OutputDebugString( (LPCWSTR)"Unable to open a read handle to the device\n!" );
		return -1;
	}

	pDevice->device_writehandle = CreateFileA( path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
								  NULL, OPEN_EXISTING, 0, 0 );

	if( INVALID_HANDLE_VALUE == pDevice->device_writehandle )
	{
		OutputDebugString( (LPCWSTR)"Unable to open a write handle to the device\n!" );
		return -1;
	}

	return 0;
}

int hid_init( void )
{
	if( FALSE == initialized )
	{
		if( lookup_functions() < 0 )
		{
			hid_exit();
			return -1;
		}

		initialized = TRUE;
	}

	return 0;
}

int hid_exit( void )
{
	if( lib_handle )
	{
		FreeLibrary( lib_handle );
	}

	lib_handle = NULL;
	initialized = FALSE;

	return 0;
}

struct hid_device_info *hid_enumerate( unsigned short vendor_id, unsigned short product_id )
{
	BOOL res;
	struct hid_device_info *pRootDevice = NULL; // return object
	struct hid_device_info *pCurrentDevice = NULL;

	// Windows objects for interacting with the driver.
	GUID InterfaceClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };
	SP_DEVINFO_DATA devinfo_data;
	SP_DEVICE_INTERFACE_DATA device_interface_data;
	SP_DEVICE_INTERFACE_DETAIL_DATA_A *pDevice_interface_detail_data = NULL;
	HDEVINFO device_info_set = INVALID_HANDLE_VALUE;
	int iDeviceIndex = 0;

	if( hid_init() < 0 )
	{
		return NULL;
	}

	// Initialize the Windows objects.
	devinfo_data.cbSize = sizeof( SP_DEVINFO_DATA );
	device_interface_data.cbSize = sizeof( SP_DEVICE_INTERFACE_DATA );

	// Get information for all the devices belonging to the HID class.
	device_info_set = SetupDiGetClassDevs( &InterfaceClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE );

	// Iterate over each device in the HID class, looking for the right one.

	for( ;; )
	{
		HANDLE write_handle = INVALID_HANDLE_VALUE;
		DWORD required_size = 0;
		HIDD_ATTRIBUTES attrib;

		res = SetupDiEnumDeviceInterfaces( device_info_set,
										   NULL,
										   &InterfaceClassGuid,
										   iDeviceIndex,
										   &device_interface_data );

		if( FALSE == res )
		{
			// A return of FALSE from this function means that
			// there are no more devices.
			break;
		}

		// Call with 0-sized detail size, and let the function
		// tell us how long the detail struct needs to be. The
		// size is put in &required_size.
		res = SetupDiGetDeviceInterfaceDetailA( device_info_set,
												&device_interface_data,
												NULL,
												0,
												&required_size,
												NULL );

		// Allocate a long enough structure for device_interface_detail_data.
		pDevice_interface_detail_data = ( SP_DEVICE_INTERFACE_DETAIL_DATA_A * ) malloc( required_size );
		pDevice_interface_detail_data->cbSize = sizeof( SP_DEVICE_INTERFACE_DETAIL_DATA_A );

		// Get the detailed data for this device. The detail data gives us
		// the device path for this device, which is then passed into
		// CreateFile() to get a handle to the device.
		res = SetupDiGetDeviceInterfaceDetailA( device_info_set,
												&device_interface_data,
												pDevice_interface_detail_data,
												required_size,
												NULL,
												NULL );

		if( FALSE == res )
		{
			//register_error(dev, "Unable to call SetupDiGetDeviceInterfaceDetail");
			// Continue to the next device.
			goto cont;
		}

		//wprintf(L"HandleName: %s\n", device_interface_detail_data->DevicePath);

		// Open a handle to the device
		write_handle = CreateFileA( pDevice_interface_detail_data->DevicePath, 0, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);

		// Check validity of write_handle.
		if( write_handle == INVALID_HANDLE_VALUE )
		{
			// Unable to open the device.
			goto cont_close;
		}


		// Get the Vendor ID and Product ID for this device.
		attrib.Size = sizeof( HIDD_ATTRIBUTES );
		HidD_GetAttributes( write_handle, &attrib );

		// Check the VID/PID to see if we should add this device to the enumeration list.
		if( ( vendor_id == 0x0 && product_id == 0x0 ) ||
			( attrib.VendorID == vendor_id && attrib.ProductID == product_id ) )
		{

#define WSTR_LEN 512
			const char *str;
			struct hid_device_info *tmp;
			HIDP_PREPARSED_DATA *pp_data = NULL;
			HIDP_CAPS caps;
			BOOLEAN res;
			NTSTATUS nt_res;
			wchar_t wstr[WSTR_LEN]; // TODO: Determine Size
			size_t len;

			/* VID/PID match. Create the record. */
			tmp = ( struct hid_device_info * ) calloc( 1, sizeof( struct hid_device_info ) );

			if( pCurrentDevice )
			{
				pCurrentDevice->next = tmp;
			}
			else
			{
				pRootDevice = tmp;
			}

			pCurrentDevice = tmp;

			// Get the Usage Page and Usage for this device.
			res = HidD_GetPreparsedData( write_handle, &pp_data );

			if( res )
			{
				nt_res = HidP_GetCaps( pp_data, &caps );

				if( nt_res == HIDP_STATUS_SUCCESS )
				{
					pCurrentDevice->usage_page = caps.UsagePage;
					pCurrentDevice->usage = caps.Usage;
				}

				HidD_FreePreparsedData( pp_data );
			}

			/* Fill out the record */
			pCurrentDevice->next = NULL;
			str = pDevice_interface_detail_data->DevicePath;

			if( str )
			{
				len = strlen( str );
				pCurrentDevice->path = ( char * ) calloc( len + 1, sizeof( char ) );
				strncpy( pCurrentDevice->path, str, len + 1 );
				pCurrentDevice->path[len] = '\0';
			}
			else
			{
				pCurrentDevice->path = NULL;
			}

			/* Serial Number */
			res = HidD_GetSerialNumberString( write_handle, wstr, sizeof( wstr ) );
			wstr[WSTR_LEN - 1] = 0x0000;

			if( res )
			{
				pCurrentDevice->serial_number = _wcsdup( wstr );
			}

			/* Manufacturer String */
			res = HidD_GetManufacturerString( write_handle, wstr, sizeof( wstr ) );
			wstr[WSTR_LEN - 1] = 0x0000;

			if( res )
			{
				pCurrentDevice->manufacturer_string = _wcsdup( wstr );
			}

			/* Product String */
			res = HidD_GetProductString( write_handle, wstr, sizeof( wstr ) );
			wstr[WSTR_LEN - 1] = 0x0000;

			if( res )
			{
				pCurrentDevice->product_string = _wcsdup( wstr );
			}

			/* VID/PID */
			pCurrentDevice->vendor_id = attrib.VendorID;
			pCurrentDevice->product_id = attrib.ProductID;

			/* Release Number */
			pCurrentDevice->release_number = attrib.VersionNumber;

			/* Interface Number. It can sometimes be parsed out of the path
			   on Windows if a device has multiple interfaces. See
			   http://msdn.microsoft.com/en-us/windows/hardware/gg487473 or
			   search for "Hardware IDs for HID Devices" at MSDN. If it's not
			   in the path, it's set to -1. */
			pCurrentDevice->interface_number = -1;

			if( pCurrentDevice->path )
			{
				char *interface_component = strstr( pCurrentDevice->path, "&mi_" );

				if( interface_component )
				{
					char *hex_str = interface_component + 4;
					char *endptr = NULL;
					pCurrentDevice->interface_number = strtol( hex_str, &endptr, 16 );

					if( endptr == hex_str )
					{
						/* The parsing failed. Set interface_number to -1. */
						pCurrentDevice->interface_number = -1;
					}
				}
			}
		}

cont_close:
		CloseHandle( write_handle );
cont:
		// We no longer need the detail data. It can be freed
		free( pDevice_interface_detail_data );

		iDeviceIndex++;

	}

	// Close the device information handle.
	SetupDiDestroyDeviceInfoList( device_info_set );

	return pRootDevice;

}

void hid_free_enumeration( struct hid_device_info *devs )
{
	// TODO: Merge this with the Linux version. This function is platform-independent.
	struct hid_device_info *d = devs;

	while( d )
	{
		struct hid_device_info *next = d->next;
		free( d->path );
		free( d->serial_number );
		free( d->manufacturer_string );
		free( d->product_string );
		free( d );
		d = next;
	}
}


hid_device *hid_open( unsigned short vendor_id, unsigned short product_id, wchar_t *serial_number )
{
	// TODO: Merge this functions with the Linux version. This function should be platform independent.
	struct hid_device_info *devs, *cur_dev;
	const char *path_to_open = NULL;
	hid_device *handle = NULL;

	devs = hid_enumerate( vendor_id, product_id );
	cur_dev = devs;

	while( NULL != cur_dev )
	{
		if( vendor_id == cur_dev->vendor_id && product_id == cur_dev->product_id )
		{
			if( NULL != serial_number )
			{
				if( 0 == wcscmp( serial_number, cur_dev->serial_number ) )
				{
					path_to_open = cur_dev->path;
					break;
				}
			}
			else
			{
				path_to_open = cur_dev->path;
				break;
			}
		}

		cur_dev = cur_dev->next;
	}

	if( NULL != path_to_open )
	{
		/* Open the device */
		handle = hid_open_path( path_to_open );
	}

	hid_free_enumeration( devs );

	return handle;
}

hid_device *hid_open_path( const char *path )
{
	hid_device *dev;
	HIDP_CAPS caps;
	HIDP_PREPARSED_DATA *pp_data = NULL;
	BOOLEAN res;
	NTSTATUS nt_res;

	if( hid_init() < 0 )
	{
		return NULL;
	}

	dev = new_hid_device();

	// Open a handle to the device.
	int iResult = open_device( path, dev );

	// Check validity of write_handle.
	if( -1 == iResult )
	{
		// Unable to open the device.
		register_error( dev, "CreateFile" );
		goto err;
	}

	// Get the Input Report length for the device.
	res = HidD_GetPreparsedData( dev->device_readhandle, &pp_data );

	if( 0 == res )
	{
		register_error( dev, "HidD_GetPreparsedData" );
		goto err;
	}

	nt_res = HidP_GetCaps( pp_data, &caps );

	if( HIDP_STATUS_SUCCESS != nt_res )
	{
		register_error( dev, "HidP_GetCaps" );
		goto err_pp_data;
	}

	dev->input_report_length = caps.InputReportByteLength;
	HidD_FreePreparsedData( pp_data );

	dev->read_buf = ( char * )malloc( dev->input_report_length );

	return dev;

err_pp_data:
	HidD_FreePreparsedData( pp_data );

err:
	CloseHandle( dev->device_readhandle );
	CloseHandle( dev->device_writehandle );
	CloseHandle( dev->ol.hEvent );
	free( dev );

	return NULL;
}

int hid_write( hid_device *dev, const unsigned char *data, size_t length )
{
	DWORD bytes_written;
	BOOL res;

	res = WriteFile( dev->device_writehandle, (LPCVOID)data, (DWORD)length, (LPDWORD)&bytes_written, 0 );

	if( 0 == res )
	{
		DWORD dwError = GetLastError();

		if( ERROR_IO_PENDING != dwError )
		{
			// WriteFile() failed. Return error.
			register_error( dev, "WriteFile" );
			return -1;
		}
	}

	return bytes_written;
}


int hid_read_timeout( hid_device *dev, unsigned char *data, size_t length, int milliseconds )
{
	DWORD bytes_read = 0;
	BOOL res;

	// Copy the handle for convenience.
	HANDLE ev = dev->ol.hEvent;

	if( !dev->read_pending )
	{
		// Start an Overlapped I/O read.
		dev->read_pending = TRUE;
		ResetEvent( ev );
		res = ReadFile( dev->device_readhandle, dev->read_buf, dev->input_report_length, &bytes_read, &dev->ol );

		if( !res )
		{
			if( GetLastError() != ERROR_IO_PENDING )
			{
				// ReadFile() has failed.
				// Clean up and return error.
				CancelIo( dev->device_readhandle );
				dev->read_pending = FALSE;
				goto end_of_function;
			}
		}
	}

	if( milliseconds >= 0 )
	{
		// See if there is any data yet.
		res = WaitForSingleObject( ev, milliseconds );

		if( res != WAIT_OBJECT_0 )
		{
			// There was no data this time. Return zero bytes available,
			// but leave the Overlapped I/O running.
			return 0;
		}
	}

	// Either WaitForSingleObject() told us that ReadFile has completed, or
	// we are in non-blocking mode. Get the number of bytes read. The actual
	// data has been copied to the data[] array which was passed to ReadFile().
	res = GetOverlappedResult( dev->device_readhandle, &dev->ol, &bytes_read, TRUE/*wait*/ );

	// Set pending back to false, even if GetOverlappedResult() returned error.
	dev->read_pending = FALSE;

	if( res && bytes_read > 0 )
	{
		if( dev->read_buf[0] == 0x0 )
		{
			/* If report numbers aren't being used, but Windows sticks a report
			   number (0x0) on the beginning of the report anyway. To make this
			   work like the other platforms, and to make it work more like the
			   HID spec, we'll skip over this byte. */
			bytes_read--;
			memcpy( data, dev->read_buf + 1, length );
		}
		else
		{
			/* Copy the whole buffer, report number and all. */
			memcpy( data, dev->read_buf, length );
		}
	}

end_of_function:

	if( !res )
	{
		register_error( dev, "GetOverlappedResult" );
		return -1;
	}

	return bytes_read;
}

int hid_read( hid_device *dev, unsigned char *data, size_t length )
{
	return hid_read_timeout( dev, data, length, ( dev->blocking ) ? -1 : 0 );
}

int hid_set_nonblocking( hid_device *dev, int nonblock )
{
	dev->blocking = !nonblock;
	return 0; /* Success */
}

int hid_send_feature_report( hid_device *dev, const unsigned char *data, size_t length )
{
	BOOL res = HidD_SetFeature( dev->device_writehandle, ( PVOID )data, length );

	if( !res )
	{
		register_error( dev, "HidD_SetFeature" );
		return -1;
	}

	return length;
}


int hid_get_feature_report( hid_device *dev, unsigned char *data, size_t length )
{
	BOOL res;
	DWORD bytes_returned;

	OVERLAPPED ol;
	memset( &ol, 0, sizeof( ol ) );

	res = DeviceIoControl( dev->device_readhandle,
						   IOCTL_HID_GET_FEATURE,
						   data, length,
						   data, length,
						   &bytes_returned, &ol );

	if( !res )
	{
		if( GetLastError() != ERROR_IO_PENDING )
		{
			// DeviceIoControl() failed. Return error.
			register_error( dev, "Send Feature Report DeviceIoControl" );
			return -1;
		}
	}

	// Wait here until the write is done. This makes
	// hid_get_feature_report() synchronous.
	res = GetOverlappedResult( dev->device_readhandle, &ol, &bytes_returned, TRUE/*wait*/ );

	if( !res )
	{
		// The operation failed.
		register_error( dev, "Send Feature Report GetOverLappedResult" );
		return -1;
	}

	return bytes_returned;
}

void hid_close( hid_device *dev )
{
	if( !dev )
	{
		return;
	}

	CancelIo( dev->device_readhandle );
	CancelIo( dev->device_writehandle );
	CloseHandle( dev->ol.hEvent );
	CloseHandle( dev->device_readhandle );
	CloseHandle( dev->device_writehandle );
	LocalFree( dev->last_error_str );
	free( dev->read_buf );
	free( dev );
}

int hid_get_manufacturer_string( hid_device *dev, wchar_t *string, size_t maxlen )
{
	BOOL res;

	res = HidD_GetManufacturerString( dev->device_readhandle, string, 2 * maxlen );

	if( !res )
	{
		register_error( dev, "HidD_GetManufacturerString" );
		return -1;
	}

	return 0;
}

int hid_get_product_string( hid_device *dev, wchar_t *string, size_t maxlen )
{
	BOOL res;

	res = HidD_GetProductString( dev->device_readhandle, string, 2 * maxlen );

	if( !res )
	{
		register_error( dev, "HidD_GetProductString" );
		return -1;
	}

	return 0;
}

int hid_get_serial_number_string( hid_device *dev, wchar_t *string, size_t maxlen )
{
	BOOL res;

	res = HidD_GetSerialNumberString( dev->device_readhandle, string, 2 * maxlen );

	if( !res )
	{
		register_error( dev, "HidD_GetSerialNumberString" );
		return -1;
	}

	return 0;
}

int hid_get_indexed_string( hid_device *dev, int string_index, wchar_t *string, size_t maxlen )
{
	BOOL res;

	res = HidD_GetIndexedString( dev->device_readhandle, string_index, string, 2 * maxlen );

	if( !res )
	{
		register_error( dev, "HidD_GetIndexedString" );
		return -1;
	}

	return 0;
}


const wchar_t *  hid_error( hid_device *dev )
{
	return ( wchar_t * )dev->last_error_str;
}
