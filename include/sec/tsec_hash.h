/** @file $RCSfile: tsec_hash.h,v $
  general description of this module
  $Id: tsec_hash.h,v 1.1.1.1 2008/05/28 07:34:59 kent Exp $
@author $Author: kent $
@date $Date: 2008/05/28 07:34:59 $
@version $Revision: 1.1.1.1 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef _TSEC_HASH_H
#define _TSEC_HASH_H


/*@{ @name hash function key

  description of defined macro
*/
#define HASH_TYPE "type"
/*@}*/

/*@{ @name hash function default type

  description of defined macro
*/
#define HASH_MD2    "MD2"
#define HASH_MD5    "MD5"
#define HASH_SHA    "SHA"
#define HASH_SHA1   "SHA1"
#define HASH_SHA224 "SHA224"
#define HASH_SHA256 "SHA256"
#define HASH_SHA384 "SHA384"
#define HASH_SHA512 "SHA512"

#define HASH_DEFAULT HASH_MD5
/*@}*/


#endif /**< _TSEC_HASH_H */

