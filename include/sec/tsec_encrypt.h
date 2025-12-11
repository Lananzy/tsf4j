/** @file $RCSfile: tsec_encrypt.h,v $
	general description of this module
	$Id: tsec_encrypt.h,v 1.1.1.1 2008/05/28 07:34:59 kent Exp $
@author $Author: kent $
@date $Date: 2008/05/28 07:34:59 $
@version $Revision: 1.1.1.1 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef _TSEC_ENCRYPT_H
#define _TSEC_ENCRYPT_H


/*@{ @name crypto function key

  description of defined macro
*/
#define CRYPTO_KEY  "key"
#define CRYPTO_TYPE "type"
/*@}*/


/*@{ @name crypto function default value

  description of defined macro
*/
#define CRYPTO_KEY_DEFAULT  "tsf4g_sec"
#define CRYPTO_DES_ECB      "DES"
#define CRYPTO_3DES_ECB     "3DES"
#define CRYPTO_AES          "AES"
#define CRYPTO_BLOWFISH_CBC "BLOWFISH"
#define CRYPTO_IDEA_CBC     "IDEA"

#define CRYPTO_AES_128      "AES128"
#define CRYPTO_AES_192      "AES192"
#define CRYPTO_AES_256      "AES256"
#define AES_128_BITS         128
#define AES_192_BITS         192
#define AES_256_BITS         256
/*@}*/


#define CRYPTO_TYPE_DEFAULT CRYPTO_DES_ECB


#endif /**< _TSEC_ENCRYPT_H */

