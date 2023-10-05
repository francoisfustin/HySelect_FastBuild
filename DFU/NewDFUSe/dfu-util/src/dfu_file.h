#pragma once


#include <stdint.h>

struct dfu_file {
    /* File name */
    CString name;
    /* Pointer to file loaded into memory */
    uint8_t *firmware;
    /* Different sizes */
    struct {
	off_t total;
	int prefix;
	int suffix;
    } size;
    /* From prefix fields */
    uint32_t lmdfu_address;
    /* From prefix fields */
    uint32_t prefix_type;

    /* From DFU suffix fields */
    uint32_t dwCRC;
    uint16_t bcdDFU;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;

    struct dfu_file()
    {
        clear();
    }

    void clear()
    {
        name = _T("");
        firmware = NULL;
        size.total = 0;
        size.prefix = 0;
        size.suffix = 0;
        uint32_t lmdfu_address = 0;
        uint32_t prefix_type = 0;
        uint32_t dwCRC = 0;
        uint16_t bcdDFU = 0;
        uint16_t idVendor = 0;
        uint16_t idProduct = 0;
        uint16_t bcdDevice = 0;
    }
};

enum suffix_req {
	NO_SUFFIX,
	NEEDS_SUFFIX,
	MAYBE_SUFFIX
};

enum prefix_req {
	NO_PREFIX,
	NEEDS_PREFIX,
	MAYBE_PREFIX
};

enum prefix_type {
	ZERO_PREFIX,
	LMDFU_PREFIX,
	LPCDFU_UNENCRYPTED_PREFIX
};

int dfu_load_file( struct dfu_file *file, enum suffix_req check_suffix, enum prefix_req check_prefix, int verbose );
int dfu_store_file( struct dfu_file *file, int write_suffix, int write_prefix );

void dfu_progress_bar( const TCHAR *desc, unsigned long long curr, unsigned long long max );
int dfu_malloc( size_t size, void **ppMemory );
uint32_t dfu_file_write_crc( int f, uint32_t crc, const void *buf, int size );
void show_suffix_and_prefix( struct dfu_file *file );
