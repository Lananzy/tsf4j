/** @file $RCSfile: tsec_acrypto.c,v $
  Various asymmetrical crypto functions for tsf4g-security module
  $Id: tsec_acrypto.c,v 1.3 2008/08/13 04:03:49 jackyai Exp $
@author $Author: jackyai $
@date $Date: 2008/08/13 04:03:49 $
@version $Revision: 1.3 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/




#include <openssl/evp.h>
#include <openssl/pem.h>
#ifdef _WIN32
#include "applink.c"
#endif

#include "sec/tsec.h"
#include "sec/tsec_i.h"

#define PASSPHRASE "tsf4g-sec rsa key file pass phrase"


TSECSET s_stTsec ;
TSECGLOBAL mstTsecIni ;

static int s_iInited ;
static TSECARGS s_stValues ;

LPTLOGCATEGORYINST g_ACryptoLog ;


/**
  various internal function defined
*/
int tsec_so_rsa_keygen (
  IN const char *a_pszFile,
  IN const char *a_pszBits
) ;

int tsec_so_rsa_encrypt (
  IN const char *a_pszFile,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  IN const int aiSub
) ;

int tsec_so_rsa_descrypt (
  IN const char *a_pszFile,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  IN const int aiSub
) ;

int tsec_load_both_key_i ( IN const char *a_pszFile, INOUT RSA **a_pstPub, INOUT RSA **a_pstPriv ) ;
int tsec_load_pub_key_i ( IN const char *a_pszFile, INOUT RSA **a_pstPub ) ;
int tsec_load_priv_key_i ( IN const char *a_pszFile, INOUT RSA **a_pstPriv ) ;

int tsec_get_filename_i (
  IN const char *a_pszFile,
  INOUT char *a_pszPub,
  IN const ssize_t aiPubLen,
  INOUT char *a_pszPriv,
  IN const ssize_t aiPrivLen
) ;



/**
  @brief Global inited function
  @retval 0 -- service inited ok
  @retval !0 -- service inited failed
  @note This function should be invoked before using any tsec functions
*/
TSEC_API int tsec_so_init ( IN const char *a_pszBuffer , IN LPTLOGCATEGORYINST a_pstLogCat   )
{
  int iRet = TSEC_OK ;

  /**
    init debug log
  */
  if (NULL != a_pstLogCat)
  {
	  g_ACryptoLog = a_pstLogCat;
  }
  


  tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_so_init ( '%s' )", a_pszBuffer ) ;

  memset ( &s_stValues, 0, sizeof(s_stValues) ) ;

  s_iInited = SVC_INITED ;

  OpenSSL_add_all_algorithms() ;

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_so_init ( const char * )=%i", iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_init_ex ( IN int argc, IN char** argv, INOUT void* extbuf, INOUT ssize_t *extbufLen )
{
  int iRet = TSEC_OK ;
  tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_init_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (0 >= argc) || (NULL == argv) || (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_init_ex ( )=%i", iRet ) ;
  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_process (
  IN const char *argsbuf,
  IN const ssize_t argsLen,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
)
{
  int iRet = TSEC_OK ;
  char sType[SIZE_16B] = { 0 },
       sFile[SIZE_128B] = { 0 },
       sBits[SIZE_16B] = { 0 },
       sBuffer[SIZE_16B] = { 0 } ;
  int iSub = 0 ;

  tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_process ( \"%s\", %i, data, %i, apvBuffer, %i )",
      argsbuf, argsLen, aiDataLen, *apiLen ) ;

  /**
    parse buffer first
  */
  memset ( &s_stValues, 0, sizeof(s_stValues) ) ;
  s_stValues.iArgsCnt = tsec_parse_args ( argsbuf, s_stValues.stArgs, TSEC_MAX_ARGS ) ;
  tlog_debug(g_ACryptoLog,0,0, "DEBUG: tsec_process buffer parse item count %i", s_stValues.iArgsCnt ) ;


  /**
    get type
  */
  tsec_get_value ( ACRYPTO_TYPE, sType, sizeof(sType), s_stValues ) ;
  tsec_get_value ( ACRYPTO_FILE, sFile, sizeof(sFile), s_stValues ) ;

  if ( 0 == strcasecmp ( ACRYPTO_KEYGEN, sType ) ) {
    tsec_get_value ( ACRYPTO_BITS, sBits, sizeof(sBits), s_stValues ) ;

    iRet = tsec_so_rsa_keygen ( sFile, sBits ) ;

  } else if ( 0 == strcasecmp ( ACRYPTO_RSA, sType ) ) {
    tsec_get_value ( ACRYPTO_SUB, sBuffer, sizeof(sBuffer), s_stValues ) ;
    iSub = atoi ( sBuffer ) ;

    iRet = tsec_so_rsa_encrypt ( sFile, apvData, aiDataLen, apvBuffer, apiLen, iSub ) ;

  } else {
    /**< default is RSA encrypt */
    tsec_get_value ( ACRYPTO_SUB, sBuffer, sizeof(sBuffer), s_stValues ) ;
    iSub = atoi ( sBuffer ) ;

    iRet = tsec_so_rsa_encrypt ( sFile, apvData, aiDataLen, apvBuffer, apiLen, iSub ) ;
  }

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_process( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_process_ex (
  IN const char *argsbuf,
  IN const ssize_t argsLen,
  IN const void *data,
  IN const ssize_t dataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  INOUT void *extbuf,
  INOUT ssize_t *extbufLen
)
{
  int iRet = TSEC_OK ;
  tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_process_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == argsbuf) || (0 >= argsLen) || (NULL == data) || (0 >= dataLen)
      || (NULL == apvBuffer) || (0 >= apiLen) || (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_process_ex ( )=%i", iRet ) ;
  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_de_process (
  IN const char *argsbuf,
  IN const ssize_t argsLen,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen
)
{
  int iRet = TSEC_OK ;
  char sType[SIZE_16B] = { 0 },
       sFile[SIZE_128B] = { 0 },
       sBits[SIZE_16B] = { 0 },
       sBuffer[SIZE_16B] = { 0 } ;
  int iSub = 0 ;

  tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_de_process( \"%s\", %i, data, %i, apvBuffer, %i )", argsbuf, argsLen, aiDataLen, *apiLen ) ;

  /**
    parse buffer first
  */
  memset ( &s_stValues, 0, sizeof(s_stValues) ) ;
  s_stValues.iArgsCnt = tsec_parse_args ( argsbuf, s_stValues.stArgs, TSEC_MAX_ARGS ) ;
  tlog_debug(g_ACryptoLog,0,0, "DEBUG: tsec_de_process buffer parse item count %i", s_stValues.iArgsCnt ) ;


  /**
    get type
  */
  tsec_get_value ( ACRYPTO_TYPE, sType, sizeof(sType), s_stValues ) ;
  tsec_get_value ( ACRYPTO_FILE, sFile, sizeof(sFile), s_stValues ) ;

  if ( 0 == strcasecmp ( ACRYPTO_KEYGEN, sType ) ) {
    tsec_get_value ( ACRYPTO_BITS, sBits, sizeof(sBits), s_stValues ) ;

    iRet = tsec_so_rsa_keygen ( sFile, sBits ) ;

  } else if ( 0 == strcasecmp ( ACRYPTO_RSA, sType ) ) {
    tsec_get_value ( ACRYPTO_SUB, sBuffer, sizeof(sBuffer), s_stValues ) ;
    iSub = atoi ( sBuffer ) ;

    iRet = tsec_so_rsa_descrypt ( sFile, apvData, aiDataLen, apvBuffer, apiLen, iSub ) ;

  } else {
    /**< default is RSA encrypt */
    tsec_get_value ( ACRYPTO_SUB, sBuffer, sizeof(sBuffer), s_stValues ) ;
    iSub = atoi ( sBuffer ) ;

    iRet = tsec_so_rsa_descrypt ( sFile, apvData, aiDataLen, apvBuffer, apiLen, iSub ) ;
  }

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_de_process( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_de_process_ex (
  IN const char *argsbuf,
  IN const ssize_t argsLen,
  IN const void *data,
  IN const ssize_t dataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  INOUT void *extbuf,
  INOUT ssize_t *extbufLen
)
{
  int iRet = TSEC_OK ;
  tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_de_process_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == argsbuf) || (0 >= argsLen) || (NULL == data) || (0 >= dataLen)
      || (NULL == apvBuffer) || (0 >= apiLen) || (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_de_process_ex ( )=%i", iRet ) ;
  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_fini ( )
{
  int iRet = TSEC_OK ;

  tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_fini ( )" ) ;


  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_fini ( )=%i", iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
TSEC_API int tsec_so_fini_ex ( INOUT void *extbuf, INOUT ssize_t *extbufLen )
{
  int iRet = TSEC_OK ;
  tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_fini_ex ( )" ) ;

  /* just for annoying warning messages from complier */
  if ( (NULL == extbuf) || (0 >= extbufLen) )
  {
    ;
  }

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_fini_ex ( )=%i", iRet ) ;
  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
#if 0
int tsec_so_rsa_keygen (
  IN const char *a_pszFile,
  IN const char *a_pszBits
)
{
  int iRet = TSEC_OK ;

  RSA *pair = NULL ;
  FILE *pfFile = NULL ;
  TFSTAT stFile ;
  char sPubFile[SIZE_512B] = { 0 },
       sPrivFile[SIZE_512B] = { 0 } ;
  int iBits = 0 ;

  tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_so_rsa_keygen( '%s', %s )", a_pszFile, a_pszBits ) ;

  memset ( &stFile, 0, sizeof(stFile) ) ;

  iBits = atoi ( a_pszBits ) ;

  /**
    make up a key file
  */
  tsec_get_filename_i ( a_pszFile, sPubFile, sizeof(sPubFile), sPrivFile, sizeof(sPrivFile) ) ;

  /**
    check file first, if exist, just return failed
  */
  if ( -1 != tflstat(sPubFile, &stFile) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, file '%s' exist, not overwrite", sPubFile ) ;
    iRet = RSA_PUBFILE_EXIST ;
    return iRet ;
  }
  else if ( -1 != tflstat(sPrivFile, &stFile) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, file '%s' exist, not overwrite", sPrivFile ) ;
    iRet = RSA_PRIVFILE_EXIST ;
    return iRet ;
  }
  else
  {
    tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_so_rsa_keygen, pub and priv file checked pass" ) ;
  }

  /**
    generated rsa key
  */
  pair = RSA_generate_key ( iBits, 0x10001, NULL, NULL ) ;
  if ( NULL == pair )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, RSA_generate_key() failed" ) ;
    iRet = RSA_KEYGEN_FAILED ;
    return iRet ;
  }

  /**
    write public key file
  */
#if 0
  iFd = tfopen ( sPubFile, TF_MODE_CREATE|TF_MODE_WRITE|TF_MODE_TRUNCATE ) ;
  if ( 0 > iFd )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, open '%s' failed", sPubFile ) ;
    iRet = RSA_PUBFILE_FAILED ;
    RSA_free ( pair ) ;
    return iRet ;
  }
  if ( NULL == ( pfFile = fdopen(iFd, "w") ) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, open '%s' failed", sPubFile ) ;
    iRet = RSA_PUBFILE_FAILED ;
    RSA_free ( pair ) ;
    tfclose ( iFd ) ;
    return iRet ;
  }
#endif
  if ( NULL == ( pfFile = fopen(sPubFile, "w") ) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, open '%s' failed", sPubFile ) ;
    iRet = RSA_PUBFILE_FAILED ;
    RSA_free ( pair ) ;
    return iRet ;
  }

  if ( !PEM_write_RSAPublicKey ( pfFile, pair ) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, PEM_write_RSAPublicKey '%s' failed", sPubFile ) ;
    iRet = RSA_PUBFILE_FAILED ;
    RSA_free ( pair ) ;
    fclose ( pfFile ) ;
    return iRet ;
  }

  fclose ( pfFile ) ;
  pfFile = NULL ;

  /**
    write private key file
  */
#if 0
  iFd = tfopen ( sPrivFile, TF_MODE_CREATE|TF_MODE_WRITE|TF_MODE_TRUNCATE ) ;
  if ( 0 > iFd )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, open '%s' failed", sPrivFile ) ;
    iRet = RSA_PRIVFILE_FAILED ;
    RSA_free ( pair ) ;
    return iRet ;
  }
  if ( NULL == ( pfFile = fdopen(iFd, "w") ) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, open '%s' failed", sPrivFile ) ;
    iRet = RSA_PRIVFILE_FAILED ;
    RSA_free ( pair ) ;
    tfclose ( iFd ) ;
    return iRet ;
  }
#endif
  if ( NULL == ( pfFile = fopen(sPrivFile, "w") ) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, open '%s' failed", sPrivFile ) ;
    iRet = RSA_PRIVFILE_FAILED ;
    RSA_free ( pair ) ;
    return iRet ;
  }

  /**< use 3des cbc method to encrypt rsa key file */
  if ( !PEM_write_RSAPrivateKey ( pfFile, pair, EVP_des_ede3_cbc(), (unsigned char *)PASSPHRASE, strlen(PASSPHRASE), NULL, NULL ) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, PEM_write_RSAPrivateKey '%s' failed", sPrivFile ) ;
    iRet = RSA_PRIVFILE_FAILED ;
    RSA_free ( pair ) ;
    fclose ( pfFile ) ;
    return iRet ;
  }

  fclose ( pfFile ) ;
  pfFile = NULL ;

  RSA_free ( pair ) ;

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_so_rsa_keygen( '%s', %s )=%i", a_pszFile, a_pszBits, iRet ) ;

  return iRet ;
}
#endif
/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_rsa_keygen (
  IN const char *a_pszFile,
  IN const char *a_pszBits
)
{
  int iRet = TSEC_OK ;
  BIO *PubOut=NULL;
  BIO *PrivOut=NULL;
  RSA *pair = NULL ;
  TFSTAT stFile ;
  char sPubFile[SIZE_512B] = { 0 },
       sPrivFile[SIZE_512B] = { 0 } ;
  int iBits = 0 ;

  tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_so_rsa_keygen( '%s', %s )", a_pszFile, a_pszBits ) ;

  memset ( &stFile, 0, sizeof(stFile) ) ;

  iBits = atoi ( a_pszBits ) ;

  /**
    make up a key file
  */
  tsec_get_filename_i ( a_pszFile, sPubFile, sizeof(sPubFile), sPrivFile, sizeof(sPrivFile) ) ;

  /**
    check file first, if exist, just return failed
  */
  if ( -1 != tflstat(sPubFile, &stFile) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, file '%s' exist, not overwrite", sPubFile ) ;
    iRet = RSA_PUBFILE_EXIST ;
    return iRet ;
  }
  else if ( -1 != tflstat(sPrivFile, &stFile) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, file '%s' exist, not overwrite", sPrivFile ) ;
    iRet = RSA_PRIVFILE_EXIST ;
    return iRet ;
  }
  else
  {
    tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_so_rsa_keygen, pub and priv file checked pass" ) ;
  }

  /**
    generated rsa key
  */
  pair = RSA_generate_key ( iBits, 0x10001, NULL, NULL ) ;
  if ( NULL == pair )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, RSA_generate_key() failed" ) ;
    iRet = RSA_KEYGEN_FAILED ;
    return iRet ;
  }

  PubOut=BIO_new(BIO_s_file());

  /**
    write public key file
  */
  if ( 0 >= BIO_write_filename(PubOut,sPubFile) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, open '%s' failed", sPubFile ) ;
    iRet = RSA_PUBFILE_FAILED ;
    if ( NULL != PubOut ) BIO_free_all(PubOut) ;
    RSA_free ( pair ) ;
    return iRet ;
  }

  if ( !PEM_write_bio_RSA_PUBKEY ( PubOut, pair ) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, PEM_write_RSAPublicKey '%s' failed", sPubFile ) ;
    iRet = RSA_PUBFILE_FAILED ;
    if ( NULL != PubOut ) BIO_free_all(PubOut) ;
    RSA_free ( pair ) ;
    return iRet ;
  }

  if ( NULL != PubOut ) BIO_free_all(PubOut) ;
  PubOut = NULL ;


  PrivOut=BIO_new(BIO_s_file());

  /**
    write private key file
  */
  if ( 0 >= BIO_write_filename(PrivOut,sPrivFile) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, open '%s' failed", sPrivFile ) ;
    iRet = RSA_PRIVFILE_FAILED ;
    if ( NULL != PrivOut ) BIO_free_all(PrivOut) ;
    RSA_free ( pair ) ;
    return iRet ;
  }

  /**< use 3des cbc method to encrypt rsa key file */
  if ( !PEM_write_bio_RSAPrivateKey ( PrivOut, pair, EVP_des_ede3_cbc(), NULL, 0, NULL, PASSPHRASE ) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_keygen, PEM_write_RSAPrivateKey '%s' failed", sPrivFile ) ;
    iRet = RSA_PRIVFILE_FAILED ;
    if ( NULL != PrivOut ) BIO_free_all(PrivOut) ;
    RSA_free ( pair ) ;
    return iRet ;
  }

  if ( NULL != PrivOut ) BIO_free_all(PrivOut) ;
  PrivOut = NULL ;

  RSA_free ( pair ) ;

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_so_rsa_keygen( '%s', %s )=%i", a_pszFile, a_pszBits, iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_rsa_encrypt (
  IN const char *a_pszFile,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  IN const int aiSub
)
{
  int iRet = TSEC_OK ;
  int iOutLen = 0,
      iSrcLen = 0,
      iCLen = 0,
      iStaticLen = 0;
  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;

  RSA *pstPub = NULL,
      *pstPriv = NULL ;

  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  *apiLen = 0 ;
  iSrcLen = aiDataLen ;

  tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_so_rsa_encrypt( '%s', data, %i, apvBuffer, %i, %i )", a_pszFile, aiDataLen, *apiLen, aiSub ) ;

  /**
    encrypt data
  */
  if ( ACRYPTO_RSA_PRIV == aiSub )
  {
    /**
      read key from file
    */
    if ( 0 != ( iRet = tsec_load_priv_key_i(a_pszFile, &pstPriv) ) )
    {
      tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_encrypt, tsec_load_priv_key_i() failed" ) ;
      return iRet ;
    }

    iStaticLen = RSA_size ( pstPriv ) ;
    tlog_debug(g_ACryptoLog,0,0, "DEBUG: tsec_so_rsa_encrypt, RSA_size()=%i", iStaticLen  ) ;

    /**< private encrypt, public descrypt */
    while ( 0 != iSrcLen )
    {
      /**
        process chunks RSA_size(rsa)-11 bytes at a time, iCLen must less than RSA_size(rsa)-11
      */
      iCLen = iSrcLen ;
      if ( iCLen > iStaticLen - 11 )
      {
        iCLen = iStaticLen - 11 ;
      }

      iOutLen = RSA_private_encrypt ( iCLen, pchInBuf, pchOutBuf, pstPriv, RSA_PKCS1_PADDING ) ;

      if ( iStaticLen != iOutLen )
      {
        tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_encrypt, RSA_private_encrypt()=%i failed", iOutLen ) ;
        *apiLen = 0 ;
        RSA_free ( pstPriv ) ;
        return TSEC_SYS_ERROR ;
      }

      *apiLen += iOutLen ;
      iSrcLen -= iCLen ;

      pchInBuf += iCLen ;
      pchOutBuf += iOutLen ;

    } /**< end while ( 0 != iSrcLen ) */

    RSA_free ( pstPriv ) ;

  }
  else
  {
    /**
      read key from file
    */
    if ( 0 != ( iRet = tsec_load_pub_key_i(a_pszFile, &pstPub) ) )
    {
      tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_encrypt, tsec_load_pub_key_i() failed" ) ;
      return iRet ;
    }

    iStaticLen = RSA_size ( pstPub ) ;
    tlog_debug(g_ACryptoLog,0,0, "DEBUG: tsec_so_rsa_encrypt, RSA_size()=%i", iStaticLen  ) ;

    /**< public encrypt, private descrypt */
    while ( 0 != iSrcLen )
    {
      /**
        process chunks RSA_size(rsa)-42 bytes at a time, iCLen must less than RSA_size(rsa)-41
      */
      iCLen = iSrcLen ;
      if ( iCLen > iStaticLen - 42 )
      {
        iCLen = iStaticLen - 42 ;
      }

      iOutLen = RSA_public_encrypt ( iCLen, pchInBuf, pchOutBuf, pstPub, RSA_PKCS1_OAEP_PADDING ) ;

      if ( iStaticLen != iOutLen )
      {
        tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_encrypt, RSA_public_encrypt()=%i failed %i", iOutLen, iStaticLen ) ;
        *apiLen = 0 ;
        RSA_free ( pstPub ) ;
        return TSEC_SYS_ERROR ;
      }

      *apiLen += iOutLen ;
      iSrcLen -= iCLen ;

      pchInBuf += iCLen ;
      pchOutBuf += iOutLen ;

    } /**< end while ( 0 != iSrcLen ) */

    RSA_free ( pstPub ) ;

  }

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_so_rsa_encrypt( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}


/**
  @brief
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_so_rsa_descrypt (
  IN const char *a_pszFile,
  IN const void *apvData,
  IN const ssize_t aiDataLen,
  INOUT void *apvBuffer,
  INOUT ssize_t *apiLen,
  IN const int aiSub
)
{
  int iRet = TSEC_OK ;
  int iOutLen = 0,
      iSrcLen = 0,
      iStaticLen = 0 ;
  unsigned const char *pchInBuf = NULL ;
  unsigned char *pchOutBuf = NULL ;

  RSA *pstPub = NULL,
      *pstPriv = NULL ;

  *apiLen = 0 ;
  iSrcLen = aiDataLen ;

  pchInBuf = apvData ;
  pchOutBuf = apvBuffer ;

  tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_so_rsa_descrypt( '%s', data, %i, apvBuffer, %i, %i )",
      a_pszFile, aiDataLen, *apiLen, aiSub ) ;

  /**
    descrypt data
  */
  if ( ACRYPTO_RSA_PRIV == aiSub )
  {
    /**
      read key from file
    */
    if ( 0 != ( iRet = tsec_load_pub_key_i(a_pszFile, &pstPub) ) )
    {
      tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_descrypt, tsec_load_pub_key_i() failed" ) ;
      return iRet ;
    }

    iStaticLen = RSA_size ( pstPub ) ;
    tlog_debug(g_ACryptoLog,0,0, "DEBUG: tsec_so_rsa_descrypt, RSA_size()=%i", iStaticLen  ) ;

    if ( 0 != iSrcLen % iStaticLen )
    {
      tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_descrypt, %i not a multiple of %i bytes", aiDataLen, iStaticLen ) ;
      RSA_free ( pstPub ) ;
      return TSEC_ARG_ERROR ;
    }

    /**< private encrypt, public descrypt */
    while ( 0 != iSrcLen )
    {
      /**
        process chunks RSA_size(rsa) bytes at a time
      */
      iOutLen = RSA_public_decrypt ( iStaticLen, pchInBuf, pchOutBuf, pstPub, RSA_PKCS1_PADDING ) ;
      if ( 0 > iOutLen )
      {
        *apiLen = 0 ;
        RSA_free ( pstPub ) ;
        return TSEC_SYS_ERROR ;
      }

      *apiLen += iOutLen ;
      pchInBuf += iStaticLen ;
      iSrcLen -= iStaticLen ;
      pchOutBuf += iOutLen ;

    } /**< end while ( 0 != iSrcLen ) */

    RSA_free ( pstPub ) ;

  }
  else
  {
    /**
      read key from file
    */
    if ( 0 != ( iRet = tsec_load_priv_key_i(a_pszFile, &pstPriv) ) )
    {
      tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_descrypt, tsec_load_priv_key_i() failed" ) ;
      return iRet ;
    }

    iStaticLen = RSA_size ( pstPriv ) ;
    tlog_debug(g_ACryptoLog,0,0, "DEBUG: tsec_so_rsa_descrypt, RSA_size()=%i", iStaticLen  ) ;

    if ( 0 != iSrcLen % iStaticLen )
    {
      tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_so_rsa_descrypt, %i not a multiple of %i bytes", aiDataLen, iStaticLen ) ;
      RSA_free ( pstPriv ) ;
      return TSEC_ARG_ERROR ;
    }

    /**< public encrypt, private descrypt */
    while ( 0 != iSrcLen )
    {
      /**
        process chunks RSA_size(rsa) bytes at a time
      */
      iOutLen = RSA_private_decrypt ( iStaticLen, pchInBuf, pchOutBuf, pstPriv, RSA_PKCS1_OAEP_PADDING ) ;
      if ( 0 > iOutLen )
      {
        *apiLen = 0 ;
        RSA_free ( pstPriv ) ;
        return TSEC_SYS_ERROR ;
      }

      *apiLen += iOutLen ;
      pchInBuf += iStaticLen ;
      iSrcLen -= iStaticLen ;
      pchOutBuf += iOutLen ;

    } /**< end while ( 0 != iSrcLen ) */

    RSA_free ( pstPriv ) ;
  }

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_so_rsa_descrypt( ..., OutBuf, OutLen=%i )=%i", *apiLen, iRet ) ;

  return iRet ;
}


/**
  @brief
  @param
  @retval
  @retval
  @note
*/
int tsec_get_filename_i (
  IN const char *a_pszFile,
  INOUT char *a_pszPub,
  IN const ssize_t aiPubLen,
  INOUT char *a_pszPriv,
  IN const ssize_t aiPrivLen
)
{
  int iRet = TSEC_OK ;

  /* tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_get_filename_i( '%s', char *, char * )", a_pszFile ) ; */

  snprintf ( a_pszPub, aiPubLen, "%s.%s", a_pszFile, RSA_PUB_POSTFIX ) ;
  snprintf ( a_pszPriv, aiPrivLen, "%s.%s", a_pszFile, RSA_PRIV_POSTFIX ) ;

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_get_filename_i( '%s', char *, char * )=%i", a_pszFile, iRet ) ;

  return iRet ;
}


/**
  @brief
  @param
  @retval
  @retval
  @note
*/
#if 0
int tsec_load_pub_key_i ( IN const char *a_pszFile, INOUT RSA **a_pstPub )
{
  int iRet = TSEC_OK ;

  FILE *pfFile = NULL ;

  char sPubFile[SIZE_512B] = { 0 },
       sPrivFile[SIZE_512B] = { 0 } ;

  /* tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_load_pub_key_i( '%s', RSA * )", a_pszFile ) ; */

  tsec_get_filename_i ( a_pszFile, sPubFile, sizeof(sPubFile), sPrivFile, sizeof(sPrivFile) ) ;

  /**
    read public key from file
  */
  pfFile = fopen ( sPubFile, "r" ) ;
  if ( NULL == pfFile )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_load_pub_key_i, open '%s' failed", sPubFile ) ;
    iRet = RSA_PUBFILE_FAILED ;
    return iRet ;
  }

  /* *a_pstPub = PEM_read_RSAPublicKey ( pfFile, NULL, NULL, PASSPHRASE ) ; */
  *a_pstPub = PEM_read_RSAPublicKey ( pfFile, NULL, NULL, NULL ) ;
  if ( NULL == *a_pstPub )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_load_pub_key_i, PEM_read_RSAPublicKey '%s' failed", sPubFile ) ;
    iRet = RSA_PUBFILE_FAILED ;
    fclose ( pfFile ) ;
    return iRet ;
  }

  fclose ( pfFile ) ;
  pfFile = NULL ;

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_load_pub_key_i( '%s', RSA * )=%i", a_pszFile, iRet ) ;

  return iRet ;
}
#endif
int tsec_load_pub_key_i ( IN const char *a_pszFile, INOUT RSA **a_pstPub )
{
  int iRet = TSEC_OK ;
  BIO *PubOut=NULL;
  EVP_PKEY *pkey = NULL;

  char sPubFile[SIZE_512B] = { 0 },
       sPrivFile[SIZE_512B] = { 0 } ;

  /* tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_load_pub_key_i( '%s', RSA * )", a_pszFile ) ; */

  tsec_get_filename_i ( a_pszFile, sPubFile, sizeof(sPubFile), sPrivFile, sizeof(sPrivFile) ) ;

  PubOut=BIO_new(BIO_s_file());

  /**
    read public key from file
  */
  if ( 0 >= BIO_read_filename(PubOut,sPubFile) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_load_pub_key_i, open '%s' failed", sPubFile ) ;
    if ( NULL != PubOut ) BIO_free_all(PubOut) ;
    iRet = RSA_PUBFILE_FAILED ;
    return iRet ;
  }

  pkey = PEM_read_bio_PUBKEY ( PubOut, 0, 0, NULL ) ;
  if ( NULL == pkey )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_load_pub_key_i, PEM_read_RSAPublicKey '%s' failed", sPubFile ) ;
    if ( NULL != PubOut ) BIO_free_all(PubOut) ;
    iRet = RSA_PUBFILE_FAILED ;
    return iRet ;
  }

  *a_pstPub = EVP_PKEY_get1_RSA ( pkey ) ;
  if ( NULL == *a_pstPub )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_load_pub_key_i, EVP_PKEY_get1_RSA '%s' failed", sPubFile ) ;
    iRet = RSA_PUBFILE_FAILED ;
  }

  EVP_PKEY_free(pkey);
  pkey = NULL ;

  if ( NULL != PubOut ) BIO_free_all(PubOut) ;
  PubOut = NULL ;

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_load_pub_key_i( '%s', RSA * )=%i", a_pszFile, iRet ) ;

  return iRet ;
}


/**
  @brief
  @param
  @retval
  @retval
  @note
*/
#if 0
int tsec_load_priv_key_i ( IN const char *a_pszFile, INOUT RSA **a_pstPriv )
{
  int iRet = TSEC_OK ;

  FILE *pfFile = NULL ;

  char sPubFile[SIZE_512B] = { 0 },
       sPrivFile[SIZE_512B] = { 0 } ;

  /* tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_load_priv_key_i( '%s', RSA * )", a_pszFile ) ; */

  tsec_get_filename_i ( a_pszFile, sPubFile, sizeof(sPubFile), sPrivFile, sizeof(sPrivFile) ) ;

  /**
    read private key from file
  */
  pfFile = fopen ( sPrivFile, "r" ) ;
  if ( NULL == pfFile )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_load_priv_key_i, open '%s' failed", sPrivFile ) ;
    iRet = RSA_PRIVFILE_FAILED ;
    return iRet ;
  }

  *a_pstPriv = PEM_read_RSAPrivateKey ( pfFile, (RSA **)NULL, NULL, PASSPHRASE ) ;
  if ( NULL == *a_pstPriv )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_load_priv_key_i, PEM_read_RSAPrivateKey '%s' failed", sPrivFile ) ;
    iRet = RSA_PRIVFILE_FAILED ;
    fclose ( pfFile ) ;
    return iRet ;
  }

  fclose ( pfFile ) ;
  pfFile = NULL ;

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_load_priv_key_i( '%s', RSA * )=%i", a_pszFile, iRet ) ;

  return iRet ;
}
#endif
int tsec_load_priv_key_i ( IN const char *a_pszFile, INOUT RSA **a_pstPriv )
{
  int iRet = TSEC_OK ;
  BIO *PrivOut=NULL;
  EVP_PKEY *pkey = NULL;

  char sPubFile[SIZE_512B] = { 0 },
       sPrivFile[SIZE_512B] = { 0 } ;

  /* tlog_debug(g_ACryptoLog,0,0, "CALL: tsec_load_priv_key_i( '%s', RSA * )", a_pszFile ) ; */

  tsec_get_filename_i ( a_pszFile, sPubFile, sizeof(sPubFile), sPrivFile, sizeof(sPrivFile) ) ;

  PrivOut=BIO_new(BIO_s_file());

  /**
    read private key from file
  */
  if ( 0 >= BIO_read_filename(PrivOut,sPrivFile) )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_load_priv_key_i, open '%s' failed", sPrivFile ) ;
    if ( NULL != PrivOut ) BIO_free_all(PrivOut) ;
    iRet = RSA_PRIVFILE_FAILED ;
    return iRet ;
  }

  pkey = PEM_read_bio_PrivateKey ( PrivOut, NULL, NULL, PASSPHRASE ) ;
  if ( NULL == pkey )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_load_priv_key_i, PEM_read_RSAPrivateKey '%s' failed", sPrivFile ) ;
    if ( NULL != PrivOut ) BIO_free_all(PrivOut) ;
    iRet = RSA_PRIVFILE_FAILED ;
    return iRet ;
  }

  *a_pstPriv = EVP_PKEY_get1_RSA ( pkey ) ;
  if ( NULL == *a_pstPriv )
  {
    tlog_error(g_ACryptoLog,0,0, "ERROR: tsec_load_priv_key_i, EVP_PKEY_get1_RSA '%s' failed", sPrivFile ) ;
    iRet = RSA_PRIVFILE_FAILED ;
  }

  EVP_PKEY_free(pkey);
  pkey = NULL ;

  if ( NULL != PrivOut ) BIO_free_all(PrivOut) ;
  PrivOut = NULL ;

  tlog_debug(g_ACryptoLog,0,0, "RETN: tsec_load_priv_key_i( '%s', RSA * )=%i", a_pszFile, iRet ) ;

  return iRet ;
}


/**
  @brief
  @param
  @retval
  @retval
  @note
*/
int tsec_load_both_key_i ( IN const char *a_pszFile, INOUT RSA **a_pstPub, INOUT RSA **a_pstPriv )
{
  int iRet = TSEC_OK ;

  iRet = tsec_load_pub_key_i ( a_pszFile, a_pstPub ) ;
  if ( 0 != iRet )
  {
    return iRet ;
  }

  iRet = tsec_load_priv_key_i ( a_pszFile, a_pstPriv ) ;
  if ( 0 != iRet )
  {
    RSA_free ( *a_pstPub ) ;
    *a_pstPub = NULL ;
    return iRet ;
  }

  return iRet ;
}

