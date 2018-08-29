/*************************************************************************
    > File Name: test.c
    > Author: hzq
    > Mail: 1593409937@qq.com 
    > Created Time: Sat 25 Aug 2018 11:06:28 AM CST
    > function:
 ************************************************************************/
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include "format.h"
#include "rsa.h"
#include "my_io.h"

int main()
{
    int  i,len,bytes;
    char buffer[100] = "this is a test string";
    bignum *encoded;
    int *decoded;
    bignum *e = bignum_init(), *n = bignum_init(), *d = bignum_init();
    bignum *bbytes = bignum_init(), *shift = bignum_init();

    if( file_init() == -1){
        return -1;
    }
    read_key(public_key,e,n);
    read_key(private_key,d,n);
    //bignum_print(n);
    /* 计算在一个加密过程中能够加密的最大数字 */

    bytes = -1;
	bignum_fromint(shift, 1 << 7); /* 7 bits per char */
	bignum_fromint(bbytes, 1);
	while(bignum_less(bbytes, n)) {
        /* Shift by one byte, NB: we use bitmask representative so this can actually be a shift... */
		bignum_imultiply(bbytes, shift);
		bytes++;
	}

    len = strlen(buffer);
    do {
		buffer[len] = '\0';
		len++;
	}while(len % bytes != 0);

    //printf("len = %d,bytes = %d\n",strlen(buffer),bytes);
    encoded = encodeMessage(len, bytes, buffer, e, n);
    decoded = decodeMessage(len/bytes, bytes, encoded, d, n);

    for(i = 0; i < len/bytes; i++) free(encoded[i].data);
    free(encoded);
	free(decoded);
    return 0;
}


