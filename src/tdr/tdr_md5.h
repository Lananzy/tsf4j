/* MD5.H - header file for MD5C.C
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.
These notices must be retained in any copies of any part of this
documentation and/or software.
 */

/* GLOBAL.H - RSAREF types and constants
 */

/* PROTOTYPES should be set to one if and only if the compiler supports
  function argument prototyping.
The following makes PROTOTYPES default to 0 if it has not already
  been defined with C compiler flags.
 */
#ifndef TDR_MD5_H
#define TDR_MD5_H

#include "tdr/tdr_define.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TDR_MD5_DIGEST_LENGTH	16	/*hash信息的长度，以字节为单位*/

void tdr_md5hash_buffer(OUT unsigned char szoutHashBuffer[TDR_MD5_DIGEST_LENGTH], const unsigned char *pszInBuffer, unsigned int uLength);

char  *tdr_md5hash2str(IN unsigned char szoutHashBuffer[TDR_MD5_DIGEST_LENGTH], OUT char *pszStr, IN int iStrSize);

int tdr_str2md5hash(OUT unsigned char szoutHashBuffer[TDR_MD5_DIGEST_LENGTH], IN const char *pszStr);

#ifdef __cplusplus
}
#endif

#endif



