/*
 * DfuSe specific functions
 * 
 * This implements the ST Microsystems DFU extensions (DfuSe)
 * as per the DfuSe 1.1a specification (ST documents AN3156, AN2606)
 * The DfuSe file format is described in ST document UM0391.
 *
 * Copyright 2010-2018 Tormod Volden <debian.tormod@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <errno.h>
#include <string.h>

#include "portable.h"
#include "libusb.h"

#include "dfu.h"
#include "usb_dfu.h"
#include "dfu_file.h"
#include "dfuse.h"
#include "dfuse_mem.h"
#include "quirks.h"

#define DFU_TIMEOUT 5000

// static unsigned int last_erased_page = 1; /* non-aligned value, won't match */
static unsigned int dfuse_address = 0;
static unsigned int dfuse_address_present = 0;
static unsigned int dfuse_length = 0;
static int dfuse_force = 0;
static int dfuse_leave = 0;
static int dfuse_unprotect = 0;
static int dfuse_mass_erase = 0;
static int dfuse_will_reset = 0;

static unsigned int quad2uint(unsigned char *p)
{
	return (*p + (*(p + 1) << 8) + (*(p + 2) << 16) + (*(p + 3) << 24));
}

static int dfuse_parse_options(const TCHAR *options)
{
	TCHAR *end;
	const TCHAR *endword;
	unsigned int number;

	/* address, possibly empty, must be first */
	if (*options != _T(':')) {
		endword = _tcschr(options, _T(':'));
		if (!endword)
			endword = options + _tcslen(options); /* GNU _tcschrnul */

		number = _tcstoul(options, &end, 0);
		if (end == endword) {
			dfuse_address = number;
			dfuse_address_present = 1;
		} else {
			errx(EX_USAGE, _T("Invalid dfuse address: %s"), options);
		}
		options = endword;
	}

	while (*options) {
		if (*options == _T(':')) {
			options++;
			continue;
		}
		endword = _tcschr(options, _T(':'));
		if (!endword)
			endword = options + _tcslen(options);

		if (!_tcsnccmp(options, _T("force"), endword - options)) {
			dfuse_force++;
			options += 5;
			continue;
		}
		if (!_tcsnccmp(options, _T("leave"), endword - options)) {
			dfuse_leave = 1;
			options += 5;
			continue;
		}
		if (!_tcsnccmp(options, _T("unprotect"), endword - options)) {
			dfuse_unprotect = 1;
			options += 9;
			continue;
		}
		if (!_tcsnccmp(options, _T("mass-erase"), endword - options)) {
			dfuse_mass_erase = 1;
			options += 10;
			continue;
		}
		if (!_tcsnccmp(options, _T("will-reset"), endword - options)) {
			dfuse_will_reset = 1;
			options += 10;
			continue;
		}

		/* any valid number is interpreted as upload length */
		number = _tcstoul(options, &end, 0);

		if( end == endword )
		{
			dfuse_length = number;
		}
		else
		{
			errx(EX_USAGE, _T("Invalid dfuse modifier: %s"), options);
		}
		
		options = endword;
	}

	return 0;
}

/* DFU_UPLOAD request for DfuSe 1.1a */
static int dfuse_upload(struct dfu_if *dif, const unsigned short length,
		 unsigned char *data, unsigned short transaction)
{
	int status;

	status = libusb_control_transfer(dif->dev_handle,
		 /* bmRequestType */	 LIBUSB_ENDPOINT_IN |
					 LIBUSB_REQUEST_TYPE_CLASS |
					 LIBUSB_RECIPIENT_INTERFACE,
		 /* bRequest      */	 DFU_UPLOAD,
		 /* wValue        */	 transaction,
		 /* wIndex        */	 dif->interface,
		 /* Data          */	 data,
		 /* wLength       */	 length,
					 DFU_TIMEOUT);
	if (status < 0) {
		warnx(_T("dfuse_upload: libusb_control_transfer returned %d (%s)"),
		      status, libusb_error_name(status));
	}
	return status;
}

/* DFU_DNLOAD request for DfuSe 1.1a */
int dfuse_download( struct dfu_if *dif, const unsigned short length, unsigned char *data, unsigned short transaction )
{
	int status;

	status = libusb_control_transfer(dif->dev_handle,
		 /* bmRequestType */	 LIBUSB_ENDPOINT_OUT |
					 LIBUSB_REQUEST_TYPE_CLASS |
					 LIBUSB_RECIPIENT_INTERFACE,
		 /* bRequest      */	 DFU_DNLOAD,
		 /* wValue        */	 transaction,
		 /* wIndex        */	 dif->interface,
		 /* Data          */	 data,
		 /* wLength       */	 length,
					 DFU_TIMEOUT);
	if (status < 0) {
		/* Silently fail on leave request on some unpredictable devices */
		if ((dif->quirks & QUIRK_DFUSE_LEAVE) && !length && !data && transaction == 2)
			return status;
		warnx(_T("dfuse_download: libusb_control_transfer returned %d (%s)"),
		      status, libusb_error_name(status));
	}
	return status;
}

/* DfuSe only commands */
/* Leaves the device in dfuDNLOAD-IDLE state */
int dfuse_special_command( struct dfu_if *dif, unsigned int address, enum dfuse_command command, int verbose, unsigned int *puiLastErasedPage )
{
	const TCHAR *dfuse_command_name[] = { _T("SET_ADDRESS") , _T("ERASE_PAGE"), _T("MASS_ERASE"), _T("READ_UNPROTECT") };
	unsigned char buf[5];
	int length;
	int ret;
	struct dfu_status dst;
	int firstpoll = 1;
	int zerotimeouts = 0;
	int polltimeout = 0;
	int stalls = 0;
	CString str;

	if( command == ERASE_PAGE )
	{
		struct memsegment *segment;
		int page_size;

		if( NULL == puiLastErasedPage )
		{
			errx( -1, _T("'puiLastErasedPage' variable can't be null.") );
		}

		segment = find_segment( dif->mem_layout, address );

		if( !segment || !( segment->memtype & DFUSE_ERASABLE ) )
		{
			errx( -1, _T("Page at 0x%08x can not be erased"), address );
		}
		
		page_size = segment->pagesize;
		
		if( verbose )
		{
			_ftprintf(stderr, _T("Erasing page size %i at address 0x%08x, page starting at 0x%08x\n"), page_size, address, address & ~( page_size - 1 ) );
		}

		buf[0] = 0x41;	// Erase command.
		length = 5;
		*puiLastErasedPage = address & ~( page_size - 1 );
	}
	else if( SET_ADDRESS == command )
	{
		if( verbose > 1 )
		{
			_ftprintf( stderr, _T("  Setting address pointer to 0x%08x\n"), address );
		}

		buf[0] = 0x21;	// Set Address Pointer command.
		length = 5;
	}
	else if( MASS_ERASE == command )
	{
		buf[0] = 0x41;	// Mass erase command when length = 1.
		length = 1;
	}
	else if( READ_UNPROTECT == command )
	{
		buf[0] = 0x92;
		length = 1;
	}
	else
	{
		errx( -1, _T("Non-supported special command %d"), command );
	}
	
	buf[1] = address & 0xff;
	buf[2] = ( address >> 8 ) & 0xff;
	buf[3] = ( address >> 16 ) & 0xff;
	buf[4] = ( address >> 24 ) & 0xff;

	ret = dfuse_download( dif, length, buf, 0 );

	if( ret < 0 )
	{
		errx( -1, _T("Error during special command \"%s\" download"), dfuse_command_name[command] );
	}
	
	do
	{
		ret = dfu_get_status( dif, &dst );

		// Workaround for some STM32L4 bootloaders that report a too
		// short poll timeout and may stall the pipe when we poll.
		if( ret == LIBUSB_ERROR_PIPE && polltimeout != 0 && stalls < 3 )
		{
			dst.bState = DFU_STATE_dfuDNBUSY;
			stalls++;

			if( verbose )
			{
				_ftprintf( stderr, _T("* Device stalled USB pipe, reusing last poll timeout\n") );
			}
		}
		else if( ret < 0 )
		{
			errx( -1, _T("Error during special command \"%s\" get_status"), dfuse_command_name[command] );
		} 
		else
		{
			polltimeout = dst.bwPollTimeout;
		}
		
		if( firstpoll )
		{
			firstpoll = 0;

			if( dst.bState != DFU_STATE_dfuDNBUSY )
			{
				_ftprintf( stderr, _T("DFU state(%u) = %s, status(%u) = %s\n"), dst.bState, dfu_state_to_string( dst.bState ), dst.bStatus, dfu_status_to_string( dst.bStatus ) );
				errx( -1, _T("Wrong state after command \"%s\" download"), dfuse_command_name[command] );
			}
			
			// STM32F405 lies about mass erase timeout.
			if( command == MASS_ERASE && dst.bwPollTimeout == 100 )
			{
				polltimeout = 35000; // Datasheet says up to 32 seconds.
				_tprintf( _T("Setting timeout to 35 seconds\n") );
			}
		}
		
		// Wait while command is executed.
		if( verbose > 1 )
		{
			_ftprintf( stderr, _T("   Poll timeout %i ms\n"), polltimeout );
		}

		milli_sleep( polltimeout );
		
		if( command == READ_UNPROTECT )
		{
			return ret;
		}

		// Workaround for e.g. Black Magic Probe getting stuck.
		if( dst.bwPollTimeout == 0 )
		{
			if( ++zerotimeouts == 100 )
			{
				errx( -1, _T("Device stuck after special command request") );
			}
		}
		else
		{
			zerotimeouts = 0;
		}
	}while( dst.bState == DFU_STATE_dfuDNBUSY );

	if( dst.bStatus != DFU_STATUS_OK )
	{
		errx( -1, _T("%s not correctly executed"), dfuse_command_name[command] );
	}

	return ret;
}

/* returns number of bytes sent */
int dfuse_dnload_chunk( struct dfu_if *dif, unsigned char *data, int size, int transaction )
{
	int bytes_sent;
	struct dfu_status dst;
	int ret;

	ret = dfuse_download(dif, size, size ? data : NULL, transaction);
	if (ret < 0) {
		err(EX_IOERR, _T("Error during download"));
		return ret;
	}
	bytes_sent = ret;

	do {
		ret = dfu_get_status(dif, &dst);

		if (ret < 0)
		{
			err(EX_IOERR, _T("Error during download get_status"));
			return ret;
		}
		milli_sleep(dst.bwPollTimeout);
	} while (dst.bState != DFU_STATE_dfuDNLOAD_IDLE &&
		 dst.bState != DFU_STATE_dfuERROR &&
		 dst.bState != DFU_STATE_dfuMANIFEST &&
		 !(dfuse_will_reset && (dst.bState == DFU_STATE_dfuDNBUSY)));

	if (dst.bState == DFU_STATE_dfuMANIFEST)
			_tprintf(_T("Transitioning to dfuMANIFEST state\n"));

	if (dst.bStatus != DFU_STATUS_OK) {
		_tprintf(_T(" failed!\n"));
		_ftprintf(stderr, _T("DFU state(%u) = %s, status(%u) = %s\n"), dst.bState,
		       dfu_state_to_string(dst.bState), dst.bStatus,
		       dfu_status_to_string(dst.bStatus));
		return -1;
	}
	return bytes_sent;
}

static int dfuse_do_leave( struct dfu_if *dif, int verbose )
{
	if( dfuse_address_present )
	{
		int ret = dfuse_special_command( dif, dfuse_address, SET_ADDRESS, verbose );

		if( ret < 0 )
		{
			errx( EX_SOFTWARE, _T("Error when executing 'dfuse_special_command' (%i)."), ret );
		}
	}
	
	_tprintf( _T("Submitting leave request...\n") );

	if( dif->quirks & QUIRK_DFUSE_LEAVE )
	{
		struct dfu_status dst;
		
		/* The device might leave after this request, with or without a response */
		dfuse_download( dif, 0, NULL, 2 );
		
		/* Or it might leave after this request, with or without a response */
		dfu_get_status( dif, &dst );
	}
	else
	{
		dfuse_dnload_chunk( dif, NULL, 0, 2 );
	}

	return 0;
}

int dfuse_do_upload(struct dfu_if *dif, int xfer_size, int fd, const TCHAR *dfuse_options, int verbose )
{
	int total_bytes = 0;
	int upload_limit = 0;
	unsigned char *buf;
	int transaction;
	int ret;

	ret = dfu_malloc( xfer_size, (void **)&buf );

	if( ret != 0 )
	{
		warnx(_T("Could not allocate memory, returned %d"), ret);
		return ret;
	}

	if (dfuse_options)
	{
		ret = dfuse_parse_options(dfuse_options);

		if( ret != 0 )
		{
			errx( EX_SOFTWARE, _T("Problem when executing 'dfuse_parse_options'.") );
		}
	}

	if (dfuse_length)
		upload_limit = dfuse_length;
	if (dfuse_address_present) {
		struct memsegment *mem_layout, *segment;

		mem_layout = parse_memory_layout((char *)dif->alt_name, verbose );
		if (!mem_layout)
			errx(EX_IOERR, _T("Failed to parse memory layout"));
		if (dif->quirks & QUIRK_DFUSE_LAYOUT)
			fixup_dfuse_layout(dif, &mem_layout);

		segment = find_segment(mem_layout, dfuse_address);
		if (!dfuse_force &&
		    (!segment || !(segment->memtype & DFUSE_READABLE)))
			errx(EX_USAGE, _T("Page at 0x%08x is not readable"),
				dfuse_address);

		if (!upload_limit) {
			if (segment) {
				upload_limit = segment->end - dfuse_address + 1;
				_tprintf(_T("Limiting upload to end of memory segment, ")
						_T("%i bytes\n"), upload_limit);
			} else {
				/* unknown segment - i.e. "force" has been used */
				upload_limit = 0x4000;
				_tprintf(_T("Limiting upload to %i bytes\n"), upload_limit);
			}
		}
		
		ret = dfuse_special_command( dif, dfuse_address, SET_ADDRESS, verbose );
		
		if( ret < 0 )
		{
			errx( EX_SOFTWARE, _T("Error when executing 'dfuse_special_command' (%i)."), ret );
		}
		
		ret = dfu_abort_to_idle( dif );

		if( ret != 0 )
		{
			return ret;
		}
	} else {
		/* Boot loader decides the start address, unknown to us */
		/* Use a short length to lower risk of running out of bounds */
		if (!upload_limit) {
			warnx(_T("Unbound upload not supported on DfuSe devices"));
			upload_limit = 0x4000;
		}
		_tprintf(_T("Limiting default upload to %i bytes\n"), upload_limit);
	}

	dfu_progress_bar(_T("Upload"), 0, 1);

	transaction = 2;
	while (1) {
		int rc;

		/* last chunk can be smaller than original xfer_size */
		if (upload_limit - total_bytes < xfer_size)
			xfer_size = upload_limit - total_bytes;
		rc = dfuse_upload(dif, xfer_size, buf, transaction++);
		if (rc < 0) {
			ret = rc;
			goto out_free;
		}

		dfu_file_write_crc(fd, 0, buf, rc);
		total_bytes += rc;

		if (total_bytes < 0)
			errx(EX_SOFTWARE, _T("Received too many bytes"));

		if (rc < xfer_size || total_bytes >= upload_limit) {
			/* last block, return successfully */
			ret = 0;
			break;
		}
		dfu_progress_bar(_T("Upload"), total_bytes, upload_limit);
	}

	dfu_progress_bar(_T("Upload"), total_bytes, total_bytes);

	ret = dfu_abort_to_idle( dif );

	if( ret != 0 )
	{
		return ret;
	}

	if (dfuse_leave)
	{
		ret = dfuse_do_leave( dif, verbose );
	}

 out_free:
	free(buf);

	return ret;
}

/* Writes an element of any size to the device, taking care of page erases */
/* returns 0 on success, otherwise -EINVAL */
static int dfuse_dnload_element(struct dfu_if *dif, unsigned int dwElementAddress, unsigned int dwElementSize, unsigned char *data, int xfer_size, int verbose)
{
	int p;
	int ret;
	struct memsegment *segment;

	// Check at least that we can write to the last address.
	segment = find_segment( dif->mem_layout, dwElementAddress + dwElementSize - 1 );
	
	if( !dfuse_force && ( !segment || !( segment->memtype & DFUSE_WRITEABLE ) ) )
	{
		errx( EX_USAGE, _T("Last page at 0x%08x is not writeable"), dwElementAddress + dwElementSize - 1 );
	}

	if( !verbose )
	{
		dfu_progress_bar( _T("Erase   "), 0, 1 );
	}

	// First pass: Erase involved pages if needed.
	for( p = 0; p < (int)dwElementSize; p += xfer_size )
	{
		int page_size;
		unsigned int erase_address;
		unsigned int address = dwElementAddress + p;
		int chunk_size = xfer_size;

		segment = find_segment( dif->mem_layout, address );

		if( !dfuse_force && ( !segment || !( segment->memtype & DFUSE_WRITEABLE ) ) )
		{
			errx( EX_USAGE, _T("Page at 0x%08x is not writeable"), address );
		}
		
		/* If the location is not in the memory map we skip erasing */
		/* since we wouldn't know the correct page size for flash erase */
		if( !segment )
		{
			continue;
		}

		page_size = segment->pagesize;

		/* check if this is the last chunk */
		if( p + chunk_size > (int)dwElementSize )
		{
			chunk_size = dwElementSize - p;
		}

		/* Erase only for flash memory downloads */
		if( ( segment->memtype & DFUSE_ERASABLE ) && !dfuse_mass_erase )
		{
			unsigned int uiLastErasedPage = 1;

			/* erase all involved pages */
			for( erase_address = address; erase_address < address + chunk_size; erase_address += page_size )
			{
				if( ( erase_address & ~(page_size - 1) ) != uiLastErasedPage )
				{
					ret = dfuse_special_command( dif, erase_address, ERASE_PAGE, verbose, &uiLastErasedPage );

					if( ret < 0 )
					{
						errx( EX_SOFTWARE, _T("Error when executing 'dfuse_special_command' (%i)."), ret );
					}
				}
			}

			if( ( ( address + chunk_size - 1 ) & ~(page_size - 1)) != uiLastErasedPage )
			{
				if( verbose > 1 )
				{
					_ftprintf(stderr, _T(" Chunk extends into next page, erase it as well\n"));
				}
				
				ret = dfuse_special_command( dif, address + chunk_size - 1, ERASE_PAGE, verbose );

				if( ret < 0 )
				{
					errx( EX_SOFTWARE, _T("Error when executing 'dfuse_special_command' (%i)."), ret );
				}
			}
			
			if( !verbose )
			{
				dfu_progress_bar( _T("Erase   "), p, dwElementSize );
			}
		}
	}

	if( !verbose )
	{
		dfu_progress_bar(_T("Erase   "), dwElementSize, dwElementSize);
	}

	if( !verbose )
	{
		dfu_progress_bar(_T("Download"), 0, 1);
	}

	/* Second pass: Write data to (erased) pages */
	for (p = 0; p < (int)dwElementSize; p += xfer_size)
	{
		unsigned int address = dwElementAddress + p;
		int chunk_size = xfer_size;

		/* check if this is the last chunk */
		if (p + chunk_size > (int)dwElementSize)
		{
			chunk_size = dwElementSize - p;
		}

		if (verbose)
		{
			_ftprintf(stderr, _T(" Download from image offset %08x to memory %08x-%08x, size %i\n"), p, address, address + chunk_size - 1, chunk_size);
		}
		else
		{
			dfu_progress_bar(_T("Download"), p, dwElementSize);
		}
		
		ret = dfuse_special_command(dif, address, SET_ADDRESS, verbose );

		if( ret < 0 )
		{
			errx( EX_SOFTWARE, _T("Error when executing 'dfuse_special_command' (%i)."), ret );
		}

		/* transaction = 2 for no address offset */
		ret = dfuse_dnload_chunk(dif, data + p, chunk_size, 2);

		if (ret != chunk_size)
		{
			errx(EX_IOERR, _T("Failed to write whole chunk: %i of %i bytes"), ret, chunk_size);
			return -EINVAL;
		}
	}
	
	if (!verbose)
		dfu_progress_bar(_T("Download"), dwElementSize, dwElementSize);
	return 0;
}

int dfuse_memcpy( unsigned char *dst, unsigned char **src, int *rem, int size )
{
	if( size > *rem )
	{
		errx( EX_NOINPUT, _T("Corrupt DfuSe file: Cannot read %d bytes from %d bytes"), size, *rem );
	}
	
	if( dst != NULL )
	{
		memcpy( dst, *src, size );
	}

	(*src) += size;
	(*rem) -= size;

	return 0;
}

/* Download raw binary file to DfuSe device */
static int dfuse_do_bin_dnload(struct dfu_if *dif, int xfer_size, struct dfu_file *file, unsigned int start_address, int verbose )
{
	unsigned int dwElementAddress;
	unsigned int dwElementSize;
	unsigned char *data;
	int ret;

	dwElementAddress = start_address;
	dwElementSize = file->size.total -
	    file->size.suffix - file->size.prefix;

	_tprintf(_T("Downloading element to address = 0x%08x, size = %i\n"),
	       dwElementAddress, dwElementSize);

	data = file->firmware + file->size.prefix;

	ret = dfuse_dnload_element( dif, dwElementAddress, dwElementSize, data, xfer_size, verbose );
	if (ret == 0)
		_tprintf(_T("File downloaded successfully\n"));

	return ret;
}

/* Parse a DfuSe file and download contents to device */
static int dfuse_do_dfuse_dnload( struct dfu_if *dif, int xfer_size, struct dfu_file *file, int verbose )
{
	uint8_t dfuprefix[11];
	uint8_t targetprefix[274];
	uint8_t elementheader[8];
	int image;
	int element;
	int bTargets;
	int bAlternateSetting;
	struct dfu_if *adif;
	int dwNbElements;
	unsigned int dwElementAddress;
	unsigned int dwElementSize;
	uint8_t *data;
	int ret;
	int rem;
	int bFirstAddressSaved = 0;

	rem = file->size.total - file->size.prefix - file->size.suffix;
	data = file->firmware + file->size.prefix;

        /* Must be larger than a minimal DfuSe header and suffix */
	if( rem < (int)(sizeof( dfuprefix ) + sizeof( targetprefix ) + sizeof( elementheader ) ) )
	{
		errx( EX_DATAERR, _T("File too small for a DfuSe file") );
	}

	if( dfuse_memcpy( dfuprefix, &data, &rem, sizeof( dfuprefix ) ) != 0 )
	{
		errx( EX_SOFTWARE, _T("Problem when executing 'dfuse_memcpy.'") );
	}

	if( strncmp( (char *)dfuprefix, "DfuSe", 5 ) )
	{
		errx( EX_DATAERR, _T("No valid DfuSe signature") );
	}
	
	if( dfuprefix[5] != 0x01 )
	{
		errx( EX_DATAERR, _T("DFU format revision %i not supported"), dfuprefix[5] );
	}
	
	bTargets = dfuprefix[10];
	_tprintf( _T("File contains %i DFU images\n"), bTargets );

	for( image = 1; image <= bTargets; image++ )
	{
		_tprintf( _T("Parsing DFU image %i\n"), image );
		
		ret = dfuse_memcpy( targetprefix, &data, &rem, sizeof( targetprefix ) );

		if( ret != 0 )
		{
			errx( EX_SOFTWARE, _T("Problem when executing 'dfuse_memcpy.'") );
		}
		
		if( strncmp( (char *)targetprefix, "Target", 6 ) )
		{
			errx( EX_DATAERR, _T("No valid target signature") );
		}

		bAlternateSetting = targetprefix[6];

		if( targetprefix[7] )
		{
			printf( "Target name: %s\n", &targetprefix[11] );
		}
		else
		{
			_tprintf( _T("No target name\n") );
		}

		dwNbElements = quad2uint( (unsigned char *)targetprefix + 270 );
		_tprintf( _T("Image for alternate setting %i, "), bAlternateSetting );
		_tprintf( _T("(%i elements, "), dwNbElements );
		_tprintf( _T("total size = %i)\n"), quad2uint( (unsigned char *)targetprefix + 266 ) );

		adif = dif;
		while( adif )
		{
			if( bAlternateSetting == adif->altsetting )
			{
				adif->dev_handle = dif->dev_handle;
				_tprintf( _T("Setting Alternate Interface #%d ...\n"), adif->altsetting );
				ret = libusb_set_interface_alt_setting( adif->dev_handle, adif->interface, adif->altsetting );
				
				if( ret < 0 )
				{
					errx( EX_IOERR, _T("Cannot set alternate interface: %s"), libusb_error_name( ret ) );
				}
				
				break;
			}
			adif = adif->next;
		}
		
		if( !adif )
		{
			warnx( _T("No alternate setting %d (skipping elements)"), bAlternateSetting );
		}

		for( element = 1; element <= dwNbElements; element++ )
		{
			_tprintf( _T("Parsing element %i, "), element );

			ret = dfuse_memcpy( elementheader, &data, &rem, sizeof( elementheader ) );
			
			if( ret != 0 )
			{
				errx(EX_SOFTWARE, _T("Problem when executing 'dfuse_memcpy.'"));
			}
			
			dwElementAddress = quad2uint( (unsigned char *)elementheader );
			dwElementSize = quad2uint( (unsigned char *)elementheader + 4 );
			_tprintf( _T("address = 0x%08x, "), dwElementAddress );
			_tprintf( _T("size = %i\n"), dwElementSize );

			if( !bFirstAddressSaved)
			{
				bFirstAddressSaved = 1;
				dfuse_address = dwElementAddress;
			}
			
			/* sanity check */
			if( (int)dwElementSize > rem )
			{
				errx( EX_DATAERR, _T("File too small for element size") );
			}

			if( adif )
			{
				ret = dfuse_dnload_element( adif, dwElementAddress, dwElementSize, data, xfer_size, verbose );
			}
			else
			{
				ret = 0;
			}

			/* advance read pointer */
			ret = dfuse_memcpy( NULL, &data, &rem, dwElementSize );

			if( ret != 0 )
			{
				return ret;
			}
		}
	}

	if( rem != 0 )
	{
		warnx( _T("%d bytes leftover"), rem );
	}

	_tprintf( _T("Done parsing DfuSe file\n") );

	return 0;
}

int dfuse_do_dnload( struct dfu_if *dif, int xfer_size, struct dfu_file *file, const TCHAR *dfuse_options, int verbose )
{
	int ret;
	struct dfu_if *adif;

	if( dfuse_options )
	{
		ret = dfuse_parse_options( dfuse_options );

		if( ret != 0 )
		{
			errx( EX_SOFTWARE, _T("Problem when executing 'dfuse_parse_options'.") );
		}
	}

	adif = dif;

	while( adif )
	{
		adif->mem_layout = parse_memory_layout( (char *)adif->alt_name, verbose );

		if( !adif->mem_layout )
		{
			errx( EX_IOERR, _T("Failed to parse memory layout for alternate interface %i"), adif->altsetting );
		}

		if( adif->quirks & QUIRK_DFUSE_LAYOUT )
		{
			fixup_dfuse_layout( adif, &(adif->mem_layout) );
		}

		adif = adif->next;
	}

	if( dfuse_unprotect )
	{
		if( !dfuse_force )
		{
			errx(EX_USAGE, _T("The read unprotect command will erase the flash memory and can only be used with force\n") );
		}
		
		ret = dfuse_special_command( dif, 0, READ_UNPROTECT, verbose );
		_tprintf(_T("Device disconnects, erases flash and resets now\n"));
		return ret;
	}
	
	if( dfuse_mass_erase )
	{
		if( !dfuse_force )
		{
			errx( EX_USAGE, _T("The mass erase command can only be used with force") );
		}
		
		_tprintf( _T("Performing mass erase, this can take a moment\n") );
		ret = dfuse_special_command( dif, 0, MASS_ERASE, verbose );
	}
	
	if( !file->name )
	{
		_tprintf( _T("DfuSe command mode\n") );
		ret = 0;
	}
	else if( dfuse_address_present )
	{
		if( file->bcdDFU == 0x11a )
		{
			errx( EX_USAGE, _T("This is a DfuSe file, not  meant for raw download") );
		}
		ret = dfuse_do_bin_dnload( dif, xfer_size, file, dfuse_address, verbose );
	}
	else
	{
		if( file->bcdDFU != 0x11a )
		{
			warnx( _T("Only DfuSe file version 1.1a is supported") );
			errx( EX_USAGE, _T("(for raw binary download, use the --dfuse-address option)") );
		}
		
		ret = dfuse_do_dfuse_dnload( dif, xfer_size, file, verbose );

		if( ret != 0 )
		{
			errx( EX_USAGE, _T("Problem when executing 'dfuse_do_dfuse_dnload'.") );
		}
	}

	adif = dif;
	
	while( adif )
	{
		free_segment_list( adif->mem_layout );
		adif = adif->next;
	}

	if( !dfuse_will_reset )
	{
		ret = dfu_abort_to_idle( dif );

		if( ret != 0 )
		{
			return ret;
		}
	}

	if( dfuse_leave )
	{
		ret = dfuse_do_leave( dif, verbose );
	}

	return ret;
}

/* Check if we have one interface, possibly multiple alternate interfaces */
int dfuse_multiple_alt(struct dfu_if *dfu_root)
{
	libusb_device *dev = dfu_root->dev;
	uint8_t configuration = dfu_root->configuration;
	uint8_t interface = dfu_root->interface;
	struct dfu_if *dif = dfu_root->next;

	while (dif) {
		if (dev != dif->dev ||
		    configuration != dif->configuration ||
		    interface != dif->interface)
			return 0;
		dif = dif->next;
	}
	return 1;
}
