This script allows to parse the "resource.h" and "TASelect.rc" files and to 
verify for all the IDs defined if there are used in the source code of HySelect.

The script will generate the "defines_found.txt" and "defines_notfound.txt" files.
And also will generate the "resource.h-cleaned" and "TASelect.rc-cleaned" files.

To finish:

 1) "_APS_xxx" must be excluded from the search.
 2) The "TASelect.rc-cleaned" file seems to be not good !
