#ifndef __MY_IO_H
#define __MY_IO_H

#include "rsa.h"

//#define	FILE_MODE	(S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)
					/* default file access permissions for new files */
                    /* user has read and write permission ... */

//#define	DIR_MODE	(FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)
					/* default permissions for new directories */
#define	DIR_MODE    00777
#define	FILE_MODE   00777

extern const char const *public_key;
extern const char const *private_key;

int file_init(void);
int save_key(const char *filename,bignum *e,bignum *n);
void Write(int fd, void *ptr, size_t nbytes);
ssize_t Read(int fd, void *ptr, size_t nbytes);
int read_key(const char *filename,bignum *e,bignum *n);

#endif
