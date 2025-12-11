/** @file $RCSfile: tsec_acrypto.h,v $
  general description of this module
  $Id: tsec_acrypto.h,v 1.1.1.1 2008/05/28 07:34:59 kent Exp $
@author $Author: kent $
@date $Date: 2008/05/28 07:34:59 $
@version $Revision: 1.1.1.1 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef _TSEC_ACRYPTO_H
#define _TSEC_ACRYPTO_H


/*@{ @name asymmetrical crypto function key

  description of defined macro
*/
#define ACRYPTO_FILE "file"
#define ACRYPTO_TYPE "type"
#define ACRYPTO_BITS "bits"
#define ACRYPTO_SUB  "pubcrypt" /**< 0 -- private encrypt, public descrypt; 1 -- public encrypt, private descrypt */
/*@}*/


/*@{ @name asymmetrical crypto function default value

  description of defined macro
*/
#define ACRYPTO_KEY_DEFAULT  "tsf4g_sec"
#define ACRYPTO_FILE_DEFAULT "tsf4g_rsa"
#define ACRYPTO_BITS_DEFAULT 1024

#define ACRYPTO_KEYGEN       "KEYGEN"
#define ACRYPTO_RSA          "RSA"
#define ACRYPTO_RSA_PUB      1
#define ACRYPTO_RSA_PRIV     0

#define ACRYPTO_TYPE_DEFAULT ACRYPTO_RSA
#define ACRYPTO_SUB_DEFAULT  ACRYPTO_RSA_PUB
/*@}*/


/*@{ @name RSA key file

  description of defined macro
*/
#define ACRYPTO_LOAD_ALL  0
#define ACRYPTO_LOAD_PUB  1
#define ACRYPTO_LOAD_PRIV 2

#define RSA_PUB_POSTFIX   "tsf4g.pub.pem"
#define RSA_PRIV_POSTFIX  "tsf4g.priv.pem"

#define RSA_CHUNK_BYTES   128
/*@}*/

/*@{ @name Return code defined

  description of defined macro
*/
#define RSA_PUBFILE_EXIST   -1 
#define RSA_PRIVFILE_EXIST  -2
#define RSA_PUBFILE_FAILED  -3
#define RSA_PRIVFILE_FAILED -4
#define RSA_KEYGEN_FAILED   -5
/*@}*/


#endif /**< _TSEC_ACRYPTO_H */

