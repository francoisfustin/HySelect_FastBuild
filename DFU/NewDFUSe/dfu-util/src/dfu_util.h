#pragma once


/* USB string descriptor should contain max 126 UTF-16 characters
 * but 254 would even accommodate a UTF-8 encoding + NUL terminator */
#define MAX_DESC_STR_LEN 254

enum mode {
	MODE_NONE,
	MODE_VERSION,
	MODE_LIST,
	MODE_DETACH,
	MODE_UPLOAD,
	MODE_DOWNLOAD
};

typedef struct _DFUUtilWorkVariables
{
	struct dfu_if *dfu_root;
	char *match_path;
	int match_vendor;
	int match_product;
	int match_vendor_dfu;
	int match_product_dfu;
	int match_config_index;
	int match_iface_index;
	int match_iface_alt_index;
	int match_devnum;
	const char *match_iface_alt_name;
	const char *match_serial;
	const char *match_serial_dfu;
}_DFUUtilWorkVariables;

int get_utf8_string_descriptor( libusb_device_handle *devh, uint8_t desc_index, unsigned char *data, int length );
void probe_devices( libusb_context *, _DFUUtilWorkVariables *pDFUUtilWorkVariables );
void disconnect_devices( _DFUUtilWorkVariables *pDFUUtilWorkVariables );
char *get_path( libusb_device *dev );
void print_dfu_if( struct dfu_if * );
void list_dfu_interfaces( _DFUUtilWorkVariables *pDFUUtilWorkVariables );
