#ifndef __MY_IO_H
#define __MY_IO_H

#include "rsa.h"

#define	FILE_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
					/* default file access permissions for new files */
#define	DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)
					/* default permissions for new directories */


int file_init(void);
int save_key(const char *filename,bignum e,bignum n);

#endif
