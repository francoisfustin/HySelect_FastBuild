#pragma once


int dfuload_do_upload( struct dfu_if *dif, int xfer_size, int expected_size, int fd );
int dfuload_do_dnload( struct dfu_if *dif, int xfer_size, struct dfu_file *file, int verbose );
