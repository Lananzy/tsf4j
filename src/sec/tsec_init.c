/** @file $RCSfile: tsec_init.c,v $
  inited process for tsf4g-security module
  $Id: tsec_init.c,v 1.3 2008/08/13 04:03:49 jackyai Exp $
@author $Author: jackyai $
@date $Date: 2008/08/13 04:03:49 $
@version 1.0
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#include "sec/tsec.h"
#include "sec/tsec_i.h"

#include "tdr/tdr.h"
#include "tdr/tdr_metalib_manage.h"
#include "tdr/tdr_data_io.h"


extern unsigned char g_szMetalib_tsec[] ;

/**
  global parameters
*/
TSECSET s_stTsec ;
TSECGLOBAL mstTsecIni ;
LPTLOGCATEGORYINST g_pstCat = NULL;



/**
  internal function defined
*/
int tsec_load_fun_i ( INOUT TSECSOFUN *a_ptSoFun, IN const char *sMoulde ) ;
int tsec_load_info_i ( IN const char *a_pszFile, INOUT TSECSOITEM **a_pstItems, INOUT int *a_piItemCnts ) ;
int tsec_dup_info_i ( IN const TSECSOITEM a_stSrcItem, INOUT TSECSOITEM *a_pstDstItem ) ;


/**
  @brief Global inited function
  @retval 0 -- service inited ok
  @retval !0 -- service inited failed
  @note This function should be invoked before using any tsec functions
*/
int tsec_init ( IN const char *aszInitFile , IN LPTLOGCATEGORYINST a_pstLogCat  )
{
  int iRet = TSEC_OK ;


  int i = 0,
      j = 0 ;

  int iItems = 0 ;


  if ( (NULL == aszInitFile) || ('\0' == *aszInitFile ) ) {
    fprintf ( stderr, "FATAL: init configured file error, process aborted\n" ) ;
    tsec_fini() ;
    return TSEC_FAILED ;
  }

  /**
    init debug log
  */
  if (NULL != a_pstLogCat)
  {
	  g_pstCat = a_pstLogCat;
  }
	 
  
 
 
 
  tlog_debug(g_pstCat,0,0, "CALL: tsec_init ( %s )", aszInitFile ) ;

  /**
    read global settings from xml file
  */
  {
    LPTDRMETALIB ptLib = NULL ;
    LPTDRMETA ptMeta = NULL ;
    TDRDATA stConfTmp ;

    ptLib = (LPTDRMETALIB) g_szMetalib_tsec ;
    ptMeta = tdr_get_meta_by_name ( ptLib, TSEC_CONF_INFO ) ;
    if ( NULL == ptMeta )
    {
      fprintf ( stderr, "FATAL: tdr_get_meta_by_name() failed\n" ) ;
      tsec_fini() ;
      return TSEC_FAILED ;
    }

    tlog_debug(g_pstCat,0,0, "INIT: read global settings from xml file %s", aszInitFile ) ;

    stConfTmp.iBuff = sizeof(mstTsecIni.stTsecConf) ;
    stConfTmp.pszBuff = (char *)&(mstTsecIni.stTsecConf) ;

    iRet = tdr_input_file ( ptMeta, &stConfTmp, aszInitFile, 1, 2 ) ;
    if ( 0 > iRet )
    {
      fprintf ( stderr, "FATAL: tdr_input_file()=%i failed %s\n", iRet, aszInitFile ) ;
      tsec_fini() ;
      return TSEC_FAILED ;
    }

    STRNCPY ( mstTsecIni.sInitFile, aszInitFile, sizeof(mstTsecIni.sInitFile) ) ;

    if ( 0 == stat ( mstTsecIni.stTsecConf.szFuncFile, &mstTsecIni.stFileStat ) ) {
      tlog_debug(g_pstCat,0,0, "INIT: Functions file %s, size %i, mtime %i",
          mstTsecIni.stTsecConf.szFuncFile, mstTsecIni.stFileStat.st_size, mstTsecIni.stFileStat.st_mtime ) ;
    } else {
      /**< create function file stat */
      tlog_warn ( g_pstCat, 0,0, "WARN: Create function file %s stat failed", mstTsecIni.stTsecConf.szFuncFile ) ;
    }

  } /**< end read global settings from xml file */


  /**
    init so function set
  */
  tlog_debug(g_pstCat,0,0, "INIT: read various functions from xml file %s", mstTsecIni.stTsecConf.szFuncFile ) ;

  /**
    read tsec functional so file
  */
  tsec_load_info_i ( mstTsecIni.stTsecConf.szFuncFile, &(mstTsecIni.pstItems), &mstTsecIni.iTotalCnt ) ;

  iItems = mstTsecIni.iTotalCnt ;

  memset ( &s_stTsec, 0, sizeof(s_stTsec) ) ;
  s_stTsec.ptTsecAPI = (TSECSVC *) malloc ( sizeof(TSECSVC) * iItems ) ;
  memset ( s_stTsec.ptTsecAPI, 0, sizeof(TSECSVC) * iItems ) ;
  for ( i=0; i<iItems; i++ ) {
    /**
      crypto function set
    */
    s_stTsec.ptTsecAPI[i].pstCrypto = (TSECSOFUN *) malloc ( sizeof(TSECSOFUN) * iItems ) ;
    memset ( s_stTsec.ptTsecAPI[i].pstCrypto, 0, (sizeof(TSECSOFUN) * iItems) ) ;
    for ( j = 0; j<iItems; j++ ) {
      s_stTsec.ptTsecAPI[i].pstCrypto[j].iRun = FUNC_NOT_INITED ;
    }

    /**
      hash function set
    */
    s_stTsec.ptTsecAPI[i].pstHash = (TSECSOFUN *) malloc ( sizeof(TSECSOFUN) * iItems ) ;
    memset ( s_stTsec.ptTsecAPI[i].pstHash, 0, (sizeof(TSECSOFUN) * iItems) ) ;
    for ( j = 0; j<iItems; j++ ) {
      s_stTsec.ptTsecAPI[i].pstHash[j].iRun = FUNC_NOT_INITED ;
    }

    /**
      asymmetrical crypto function set
    */
    s_stTsec.ptTsecAPI[i].pstACrypto = (TSECSOFUN *) malloc ( sizeof(TSECSOFUN) * iItems ) ;
    memset ( s_stTsec.ptTsecAPI[i].pstACrypto, 0, (sizeof(TSECSOFUN) * iItems) ) ;
    for ( j = 0; j<iItems; j++ ) {
      s_stTsec.ptTsecAPI[i].pstACrypto[j].iRun = FUNC_NOT_INITED ;
    }

    /**
      envelope function set
    */
    s_stTsec.ptTsecAPI[i].pstEnvp = (TSECSOFUN *) malloc ( sizeof(TSECSOFUN) * iItems ) ;
    memset ( s_stTsec.ptTsecAPI[i].pstEnvp, 0, (sizeof(TSECSOFUN) * iItems) ) ;
    for ( j = 0; j<iItems; j++ ) {
      s_stTsec.ptTsecAPI[i].pstEnvp[j].iRun = FUNC_NOT_INITED ;
    }

    /**
      ip verify function set
    */
    s_stTsec.ptTsecAPI[i].pstIP = (TSECSOFUN *) malloc ( sizeof(TSECSOFUN) * iItems ) ;
    memset ( s_stTsec.ptTsecAPI[i].pstIP, 0, (sizeof(TSECSOFUN) * iItems) ) ;
    for ( j = 0; j<iItems; j++ ) {
      s_stTsec.ptTsecAPI[i].pstIP[j].iRun = FUNC_NOT_INITED ;
    }

  } /**< end for ( ... ) */


  /**
    load so line by line
  */
  for ( i=0; i<iItems; i++ ) {
    int iFunCnt = 0, /**< function index at the same service */
        iCurSvc = 0, /**< current service index, identified by 'service name' */
        iDifferent = 1;

    if ( '\0' == *(mstTsecIni.pstItems[i].sMPath) ) {
      continue ;
    }

    tlog_debug(g_pstCat,0,0, "DEBUG: begin to read function from %s", mstTsecIni.pstItems[i].sMPath ) ;

    /**
      get service location first
    */
    for ( iCurSvc=0; iCurSvc<s_stTsec.iSvcCnt; iCurSvc++ ) {
      if ( 0 == strcasecmp ( mstTsecIni.pstItems[i].sSvcName, s_stTsec.ptTsecAPI[iCurSvc].sSvc ) ) {
        /**< found */
        tlog_debug(g_pstCat,0,0, "DEBUG: found exist service %s at %i", mstTsecIni.pstItems[i].sSvcName, iCurSvc ) ;
        iDifferent = 0 ;
        break ;
      }
    }

    if ( SVC_NOT_INITED == s_stTsec.ptTsecAPI[iCurSvc].iEnable ) {
      /**< current service first inited */
      s_stTsec.ptTsecAPI[iCurSvc].iEnable = SVC_INITED ;
      STRNCPY ( s_stTsec.ptTsecAPI[iCurSvc].sSvc, mstTsecIni.pstItems[i].sSvcName, sizeof(s_stTsec.ptTsecAPI[iCurSvc].sSvc) ) ;
    }


    /**
      various function
    */
    if ( 0 == strcasecmp ( mstTsecIni.pstItems[i].sMType, TSEC_SVC_CRYPTO ) ) {

      /**
        crypto function
      */
      iFunCnt = s_stTsec.ptTsecAPI[iCurSvc].iCryptoCnts ;
      iRet = tsec_load_fun_i ( &(s_stTsec.ptTsecAPI[iCurSvc].pstCrypto[iFunCnt]), mstTsecIni.pstItems[i].sMPath ) ;
      if ( TSEC_OK != iRet ) {
        tlog_error(g_pstCat,0,0, "ERROR: crypto type, read funtion from %s failed", mstTsecIni.pstItems[i].sMPath ) ;
        break ;
      } else {
        s_stTsec.ptTsecAPI[iCurSvc].pstCrypto[iFunCnt].iRun = FUNC_INITED ;
      }

      /**<
        copy so info into function set struct
      */
      tsec_dup_info_i ( mstTsecIni.pstItems[i], &(s_stTsec.ptTsecAPI[iCurSvc].pstCrypto[iFunCnt].stItem) ) ;

      /**
        call so function's inited function
      */
      iRet = s_stTsec.ptTsecAPI[iCurSvc].pstCrypto[iFunCnt].so_init ( mstTsecIni.pstItems[i].stArgFields.sArgs ) ;
      if ( TSEC_OK != iRet ) {
        tlog_error(g_pstCat,0,0, "ERROR: crypto type, invoked init funtion from %s failed", mstTsecIni.pstItems[i].sMPath ) ;
        break ;
      }

      s_stTsec.ptTsecAPI[iCurSvc].iCryptoCnts ++ ;

    } else if ( 0 == strcasecmp ( mstTsecIni.pstItems[i].sMType, TSEC_SVC_HASH ) ) {
      /**
        hash function
      */
      iFunCnt = s_stTsec.ptTsecAPI[iCurSvc].iHashCnts ;
      iRet = tsec_load_fun_i ( &(s_stTsec.ptTsecAPI[iCurSvc].pstHash[iFunCnt]), mstTsecIni.pstItems[i].sMPath ) ;
      if ( TSEC_OK != iRet ) {
        tlog_error(g_pstCat,0,0, "ERROR: hash type, read funtion from %s failed", mstTsecIni.pstItems[i].sMPath ) ;
        break ;
      } else {
        s_stTsec.ptTsecAPI[iCurSvc].pstHash[iFunCnt].iRun = FUNC_INITED ;
      }

      /**<
        copy so info into function set struct
      */
      tsec_dup_info_i ( mstTsecIni.pstItems[i], &(s_stTsec.ptTsecAPI[iCurSvc].pstHash[iFunCnt].stItem) ) ;

      /**
        call so function's inited function
      */
      iRet = s_stTsec.ptTsecAPI[iCurSvc].pstHash[iFunCnt].so_init ( mstTsecIni.pstItems[i].stArgFields.sArgs ) ;
      if ( TSEC_OK != iRet ) {
        tlog_error(g_pstCat,0,0, "ERROR: crypto type, invoked init funtion from %s failed", mstTsecIni.pstItems[i].sMPath ) ;
        break ;
      }

      s_stTsec.ptTsecAPI[iCurSvc].iHashCnts ++ ;

    } else if ( 0 == strcasecmp ( mstTsecIni.pstItems[i].sMType, TSEC_SVC_ACRYPTO ) ) {

      /**
        asymmetrical crypto function
      */
      iFunCnt = s_stTsec.ptTsecAPI[iCurSvc].iAsyCnts ;
      iRet = tsec_load_fun_i ( &(s_stTsec.ptTsecAPI[iCurSvc].pstACrypto[iFunCnt]), mstTsecIni.pstItems[i].sMPath ) ;
      if ( TSEC_OK != iRet ) {
        tlog_error(g_pstCat,0,0, "ERROR: asymmetrical crypto type, read funtion from %s failed", mstTsecIni.pstItems[i].sMPath ) ;
        break ;
      } else {
        s_stTsec.ptTsecAPI[iCurSvc].pstACrypto[iFunCnt].iRun = FUNC_INITED ;
      }

      /**<
        copy so info into function set struct
      */
      tsec_dup_info_i ( mstTsecIni.pstItems[i], &(s_stTsec.ptTsecAPI[iCurSvc].pstACrypto[iFunCnt].stItem) ) ;

      /**
        call so function's inited function
      */
      iRet = s_stTsec.ptTsecAPI[iCurSvc].pstACrypto[iFunCnt].so_init ( mstTsecIni.pstItems[i].stArgFields.sArgs ) ;
      if ( TSEC_OK != iRet ) {
        tlog_error(g_pstCat,0,0, "ERROR: asymmetrical crypto type, invoked init funtion from %s failed", mstTsecIni.pstItems[i].sMPath ) ;
        break ;
      }

      s_stTsec.ptTsecAPI[iCurSvc].iAsyCnts ++ ;

      } else if ( 0 == strcasecmp ( mstTsecIni.pstItems[i].sMType, TSEC_SVC_CRYPTO ) ) {

      /**
        envelope function
      */
      iFunCnt = s_stTsec.ptTsecAPI[iCurSvc].iEnvpCnts ;
      iRet = tsec_load_fun_i ( &(s_stTsec.ptTsecAPI[iCurSvc].pstEnvp[iFunCnt]), mstTsecIni.pstItems[i].sMPath ) ;
      if ( TSEC_OK != iRet ) {
        tlog_error(g_pstCat,0,0, "ERROR: envelope type, read funtion from %s failed", mstTsecIni.pstItems[i].sMPath ) ;
        break ;
      } else {
        s_stTsec.ptTsecAPI[iCurSvc].pstEnvp[iFunCnt].iRun = FUNC_INITED ;
      }

      /**<
        copy so info into function set struct
      */
      tsec_dup_info_i ( mstTsecIni.pstItems[i], &(s_stTsec.ptTsecAPI[iCurSvc].pstEnvp[iFunCnt].stItem) ) ;

      /**
        call so function's inited function
      */
      iRet = s_stTsec.ptTsecAPI[iCurSvc].pstEnvp[iFunCnt].so_init ( mstTsecIni.pstItems[i].stArgFields.sArgs ) ;
      if ( TSEC_OK != iRet ) {
        tlog_error(g_pstCat,0,0, "ERROR: envelope type, invoked init funtion from %s failed", mstTsecIni.pstItems[i].sMPath ) ;
        break ;
      }

      s_stTsec.ptTsecAPI[iCurSvc].iEnvpCnts ++ ;

    } else if ( 0 == strcasecmp ( mstTsecIni.pstItems[i].sMType, TSEC_SVC_IP ) ) {

      /**
        ip verify function
      */
      iFunCnt = s_stTsec.ptTsecAPI[iCurSvc].iIPCnts ;
      iRet = tsec_load_fun_i ( &(s_stTsec.ptTsecAPI[iCurSvc].pstIP[iFunCnt]), mstTsecIni.pstItems[i].sMPath ) ;
      if ( TSEC_OK != iRet ) {
        tlog_error(g_pstCat,0,0, "ERROR: ip verify type, read funtion from %s failed", mstTsecIni.pstItems[i].sMPath ) ;
        break ;
      } else {
        s_stTsec.ptTsecAPI[iCurSvc].pstIP[iFunCnt].iRun = FUNC_INITED ;
      }

      /**<
        copy so info into function set struct
      */
      tsec_dup_info_i ( mstTsecIni.pstItems[i], &(s_stTsec.ptTsecAPI[iCurSvc].pstIP[iFunCnt].stItem) ) ;

      /**
        call so function's inited function
      */
      iRet = s_stTsec.ptTsecAPI[iCurSvc].pstIP[iFunCnt].so_init ( mstTsecIni.pstItems[i].stArgFields.sArgs ) ;
      if ( TSEC_OK != iRet ) {
        tlog_error(g_pstCat,0,0, "ERROR: ip verify type, invoked init funtion from %s failed", mstTsecIni.pstItems[i].sMPath ) ;
        break ;
      }

      s_stTsec.ptTsecAPI[iCurSvc].iIPCnts ++ ;

    } else {
      ;
    }

    /**
      set service counter
    */
    if ( iDifferent ) s_stTsec.iSvcCnt ++ ;

    if ( TSEC_OK == iRet ) {
      tlog_debug(g_pstCat,0,0, "DEBUG: load funtion from %s ok, service %s, CurSvc %i",
          mstTsecIni.pstItems[i].sMPath, mstTsecIni.pstItems[i].sSvcName, iCurSvc ) ;
    }

  } /**< end for ( i=0; i<iItems; i++ ) */

  if ( TSEC_OK != iRet ) tsec_fini ( ) ;
  else s_stTsec.iInited = SVC_INITED ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_init( %s )=%i, svcCnt %i", aszInitFile, iRet, s_stTsec.iSvcCnt ) ;

  return iRet ;
}


/**
  @brief Global destructed function
  @retval 0 -- service destructed ok
  @retval !0 -- service destructed failed
  @note This function should be invoked before application process exits
*/
int tsec_fini ( )
{
  int iRet = 0 ;




  return iRet ;
}


/**
  @brief Get operation handler via specified module name
  @retval 0 -- get handler ok
  @retval !0 -- get handler failed
  @note
*/
int tsec_open_handler ( IN const char *aszModule, INOUT int *apiHandler )
{
  int iRet = TSEC_FAILED,
      i = 0 ;
  size_t iLen = 0 ;

  iLen = strlen ( aszModule ) ;
  *apiHandler = HANDLER_NOT_FOUND ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_open_handler ( %s, int * )", aszModule ) ;

  for ( i=0; i<s_stTsec.iSvcCnt; i++ ) {
    if ( 0 == strncmp ( aszModule, s_stTsec.ptTsecAPI[i].sSvc, iLen ) ) {
      tlog_debug(g_pstCat,0,0, "DEBUG: service %s handle found -- %i", s_stTsec.ptTsecAPI[i].sSvc, i ) ;
      iRet = TSEC_OK ;
      *apiHandler = i ;
      break ;
    }

  }

  tlog_debug(g_pstCat,0,0, "RETN: tsec_open_handler ( %s, %i )=%i", aszModule, *apiHandler, iRet ) ;

  return iRet ;
}


/**
  @brief Disable operation handler
  @param apiHandler -- return handler
  @retval 0 -- close handler ok
  @retval !0 -- close handler failed
  @note
*/
int tsec_close_handler ( INOUT int *apiHandler )
{
  int iRet = TSEC_OK ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_close_handler ( %i )", *apiHandler ) ;

  *apiHandler = HANDLER_NOT_FOUND ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_close_handler()=%i", iRet ) ;

  return iRet ;
}


/**
  @brief
  @param
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_svc_reload ( IN const char *a_pszSvcName )
{
  int iRet = TSEC_OK ;
  int iFound = 0 ;
  int iIndex = 0 ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_svc_reload( '%s' )", a_pszSvcName ) ;

  /**
    search in-use service name first
  */
  for ( iIndex=0; iIndex<s_stTsec.iSvcCnt; iIndex++ ) {
    if ( SVC_NOT_INITED == s_stTsec.ptTsecAPI[iIndex].iEnable ) continue ;

    if ( 0 == strcasecmp ( a_pszSvcName, s_stTsec.ptTsecAPI[iIndex].sSvc ) ) {
      /**< found */
      tlog_debug(g_pstCat,0,0, "DEBUG: found exist service %s at %i", a_pszSvcName, iIndex ) ;
      iFound = 1 ;
      break ;
    }
  }

  if ( 0 == iFound ) {
    tlog_debug(g_pstCat,0,0, "RETN: service %s not exist, no reload action", a_pszSvcName ) ;
    iRet = TSEC_SVC_NOT_FOUNT ;
    return iRet ;
  }

  /**
    disable service function first
  */
  {
    int i = 0 ;
    for ( i=0; i<s_stTsec.ptTsecAPI[iIndex].iCryptoCnts; i++ ) {
      s_stTsec.ptTsecAPI[iIndex].pstCrypto[i].iRun = FUNC_NOT_INITED ;
    }

    for ( i=0; i<s_stTsec.ptTsecAPI[iIndex].iHashCnts; i++ ) {
      s_stTsec.ptTsecAPI[iIndex].pstHash[i].iRun = FUNC_NOT_INITED ;
    }

    for ( i=0; i<s_stTsec.ptTsecAPI[iIndex].iAsyCnts; i++ ) {
      s_stTsec.ptTsecAPI[iIndex].pstACrypto[i].iRun = FUNC_NOT_INITED ;
    }

    for ( i=0; i<s_stTsec.ptTsecAPI[iIndex].iEnvpCnts; i++ ) {
      s_stTsec.ptTsecAPI[iIndex].pstEnvp[i].iRun = FUNC_NOT_INITED ;
    }
  }

  /**
    reload all function info from xml file
  */
  tsec_load_info_i ( mstTsecIni.stTsecConf.szFuncFile, &(mstTsecIni.pstItems), &mstTsecIni.iTotalCnt ) ;

  /**
    reload function
  */


  tlog_debug(g_pstCat,0,0, "RETN: tsec_svc_reload( '%s' )=%i", a_pszSvcName, iRet ) ;

  return iRet ;
}


/**
  @brief
  @param
  @retval 0 --
  @retval !0 --
  @note
*/
int tsec_is_config_changed ( )
{
  int iRet = TSEC_FILE_NO_CHANGED ;

  struct stat stFileStat ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_is_config_changed( )" ) ;

  memset ( &stFileStat, 0, sizeof(stFileStat) ) ;

  if ( 0 == stat ( mstTsecIni.stTsecConf.szFuncFile, &stFileStat ) ) {
    tlog_debug(g_pstCat,0,0, "DEBUG: Functions file %s, size %i, mtime %i",
        mstTsecIni.stTsecConf.szFuncFile, stFileStat.st_size, stFileStat.st_mtime ) ;

    if ( mstTsecIni.stFileStat.st_mtime != stFileStat.st_mtime ) {
      mstTsecIni.stFileStat.st_mtime = stFileStat.st_mtime ;
      iRet = TSEC_FILE_TIME_CHANGED ;
    }

    if ( mstTsecIni.stFileStat.st_size != stFileStat.st_size ) {
      mstTsecIni.stFileStat.st_size = stFileStat.st_size ;
      iRet = TSEC_FILE_SIZE_CHANGED ;
    }

    if ( (mstTsecIni.stFileStat.st_mtime != stFileStat.st_mtime) && (mstTsecIni.stFileStat.st_size != stFileStat.st_size) ) {
      mstTsecIni.stFileStat.st_mtime = stFileStat.st_mtime ;
      mstTsecIni.stFileStat.st_size = stFileStat.st_size ;
      iRet = TSEC_FILE_BOTH_CHANGED ;
    }

  } else {
    /**< function file stat failed */
    tlog_warn ( g_pstCat, 0,0, "WARN: Create function file %s stat failed", mstTsecIni.stTsecConf.szFuncFile ) ;
    iRet = TSEC_FILE_FAILED ;
  }

  tlog_debug(g_pstCat,0,0, "RETN: tsec_is_config_changed( )=%i", iRet ) ;

  return iRet ;
}


/**
  @brief Load so function
  @param
  @retval 0 -- load ok
  @retval !0 -- load failed
  @note Internal function
*/
int tsec_load_fun_i ( INOUT TSECSOFUN *a_ptSoFun, IN const char *sMoulde )
{
  int iRet = TSEC_OK ;
  void *pvHandler = NULL ;
  char *pcError = NULL ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_load_fun_i(TSECSOFUN *, '%s')", sMoulde ) ;

  pvHandler = dlopen ( sMoulde, RTLD_NOW ) ;
  if ( NULL != ( pcError = dlerror() ) ) {
    /**< load so failed */
    tlog_error(g_pstCat,0,0, "ERROR: open %s function failed", sMoulde ) ;
    iRet = TSEC_FAILED ;
    return iRet ;
  }

  a_ptSoFun->so_init = (TSEC_SO_INIT) dlsym ( pvHandler, TSEC_SO_INIT_SYM ) ;
  if ( NULL != ( pcError = dlerror() ) ) {
    /**
    load init function failed
    */
    tlog_error(g_pstCat,0,0, "ERROR: load %s failed: %s", TSEC_SO_INIT_SYM, pcError ) ;
    iRet = TSEC_FAILED ;
  }

  a_ptSoFun->so_init_ex = (TSEC_SO_INIT_EX) dlsym ( pvHandler, TSEC_SO_INIT_EX_SYM ) ;
  if ( NULL != ( pcError = dlerror() ) ) {
    /**
      load init extension function failed
    */
    tlog_error(g_pstCat,0,0, "ERROR: load %s failed: %s", TSEC_SO_INIT_EX_SYM, pcError ) ;
    iRet = TSEC_FAILED ;
  }

  /**
    load process function
  */
  a_ptSoFun->so_proc = (TSEC_SO_PROC) dlsym ( pvHandler, TSEC_SO_PORC_SYM ) ;
  if ( NULL != ( pcError = dlerror() ) ) {
    /**
      load process function failed
    */
    tlog_error(g_pstCat,0,0, "ERROR: load %s failed: %s", TSEC_SO_PORC_SYM, pcError ) ;
    iRet = TSEC_FAILED ;
  }

  a_ptSoFun->so_proc_ex = (TSEC_SO_PROC_EX) dlsym ( pvHandler, TSEC_SO_PORC_EX_SYM ) ;
  if ( NULL != ( pcError = dlerror() ) ) {
    /**
      load process extension function failed
    */
    tlog_error(g_pstCat,0,0, "ERROR: load %s failed: %s", TSEC_SO_PORC_EX_SYM, pcError ) ;
    iRet = TSEC_FAILED ;
  }

  /**
    load reverse function
  */
  a_ptSoFun->so_deproc = (TSEC_SO_DEPROC) dlsym ( pvHandler, TSEC_SO_DEPROC_SYM ) ;
  if ( NULL != ( pcError = dlerror() ) ) {
    /**
      load reverse process function failed
    */
    tlog_error(g_pstCat,0,0, "ERROR: load %s failed: %s", TSEC_SO_DEPROC_SYM, pcError ) ;
    iRet = TSEC_FAILED ;
  }

  a_ptSoFun->so_deproc_ex = (TSEC_SO_DEPROC_EX) dlsym ( pvHandler, TSEC_SO_DEPROC_EX_SYM ) ;
  if ( NULL != ( pcError = dlerror() ) ) {
    /**
      load reverse process extension function failed
    */
    tlog_error(g_pstCat,0,0, "ERROR: load %s failed: %s", TSEC_SO_DEPROC_EX_SYM, pcError ) ;
    iRet = TSEC_FAILED ;
  }


  /**
    load fini function
  */
  a_ptSoFun->so_fini = (TSEC_SO_FINI) dlsym ( pvHandler, TSEC_SO_FINI_SYM ) ;
  if ( NULL != ( pcError = dlerror() ) ) {
    /**
      load fini function failed
    */
    tlog_error(g_pstCat,0,0, "ERROR: load %s failed: %s", TSEC_SO_FINI_SYM, pcError ) ;
    iRet = TSEC_FAILED ;
  }

  a_ptSoFun->so_fini_ex = (TSEC_SO_FINI_EX) dlsym ( pvHandler, TSEC_SO_FINI_EX_SYM ) ;
  if ( NULL != ( pcError = dlerror() ) ) {
    /**
      load fini extension function failed
    */
    tlog_error(g_pstCat,0,0, "ERROR: load %s failed: %s", TSEC_SO_FINI_EX_SYM, pcError ) ;
    iRet = TSEC_FAILED ;
  }

  /* should *NOT* invoke dlclose() here */
#if 0
  if ( NULL != pvHandler )
  {
    dlclose ( pvHandler ) ;
  }
  pvHandler = NULL ;
#endif

  tlog_debug(g_pstCat,0,0, "RETN: tsec_load_fun_i( TSECSOFUN *, '%s' )=%i", sMoulde, iRet ) ;

  return iRet ;
}


/**
  @brief get total lines of a file
  @param
  @retval
  @retval
  @note
*/
int tsec_get_filelines_i ( IN const char *a_pszFile )
{
  int iLines = 0 ;

  FILE *pfFile = NULL ;
  char sBuffer[SIZE_1K] = { 0 } ;

  pfFile = fopen ( a_pszFile, "r" ) ;
  if ( NULL == pfFile )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec_get_filelines_i ( '%s' ) open file failed", a_pszFile ) ;
    return TSEC_FAILED ;
  }

  while ( NULL != ( fgets (sBuffer, SIZE_1K, pfFile) ) )
  {
    iLines ++ ;
  }

  fclose ( pfFile ) ;
  pfFile = NULL ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_get_filelines_i ( '%s' )=%i", a_pszFile, iLines ) ;

  return iLines ;
}


/**
  @brief
  @param
  @retval
  @retval
  @note
*/
int tsec_load_info_i ( IN const char *a_pszFile, INOUT TSECSOITEM **a_pstItems, INOUT int *a_piItemCnts )
{
  int iRet = TSEC_OK ;
  int i = 0 ;
  FILE *pfFile = NULL ;
  char sBuffer[SIZE_1K] = { 0 } ;
  int iItems = 0 ;

  TSECSOITEM *pstItem = NULL ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_load_info_i ( '%s', TSECSOITEM **, int * )", a_pszFile ) ;

  iItems = tsec_get_filelines_i ( a_pszFile ) ;
  if ( 0 >= iItems )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec_load_info_i, file '%s' empty, no info load", a_pszFile ) ;
    return TSEC_FAILED ;
  }

  pfFile = fopen ( a_pszFile, "r" ) ;
  if ( NULL == pfFile )
  {
    tlog_error(g_pstCat,0,0, "ERROR: tsec_load_info_i ( '%s' ) open file failed", a_pszFile ) ;
    return TSEC_FAILED ;
  }

  /**
    free old buffer first
  */
  pstItem = *a_pstItems ;
  if ( NULL != pstItem )
  {
    free ( pstItem ) ;
    pstItem = NULL ;
  }

  /**
    malloc new space for item info
  */
  pstItem = (TSECSOITEM *) malloc ( sizeof(TSECSOITEM) * iItems ) ;
  memset ( pstItem, 0, (sizeof(TSECSOITEM) * iItems) ) ;

  while ( NULL != ( fgets (sBuffer, SIZE_1K, pfFile) ) )
  {
    if ( i >= iItems )
    {
      /**< should not happen, but just in case */
      tlog_warn ( g_pstCat, 0,0, "WARN: tsec_load_info_i, line counts %i not correct", iItems ) ;
      break ;
    }

    sscanf ( sBuffer, "%s%s%s%s%s",
        pstItem[i].sSvcName,
        pstItem[i].sMType,
        pstItem[i].sCtrFlag,
        pstItem[i].sMPath,
        pstItem[i].stArgFields.sArgs
    ) ;

    pstItem[i].stArgFields.iArgsCnt =
        tsec_parse_args ( pstItem[i].stArgFields.sArgs, pstItem[i].stArgFields.stArgs, TSEC_MAX_ARGS ) ;

    i ++ ; /**< point to next item struct */

  } /**< end while() read so functional so setting */

  fclose ( pfFile ) ;
  pfFile = NULL ;

  *a_piItemCnts = iItems ;
  *a_pstItems = pstItem ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_load_info_i ( '%s', TSECSOITEM **, %i )=%i", a_pszFile, *a_piItemCnts, iRet ) ;

  return iRet ;
}


/**
  @brief
  @param
  @retval
  @retval
  @note
*/
int tsec_dup_info_i ( IN const TSECSOITEM a_stSrcItem, INOUT TSECSOITEM *a_pstDstItem )
{
  int iRet = TSEC_OK ;

  tlog_debug(g_pstCat,0,0, "CALL: tsec_dup_info_i ( const TSECSOITEM, TSECSOITEM * )" ) ;

  STRNCPY ( a_pstDstItem->sSvcName, a_stSrcItem.sSvcName, sizeof(a_pstDstItem->sSvcName) ) ;

  STRNCPY ( a_pstDstItem->sMType, a_stSrcItem.sMType, sizeof(a_pstDstItem->sMType) ) ;

  STRNCPY ( a_pstDstItem->sCtrFlag, a_stSrcItem.sCtrFlag, sizeof(a_pstDstItem->sCtrFlag) ) ;

  STRNCPY ( a_pstDstItem->sMPath, a_stSrcItem.sMPath, sizeof(a_pstDstItem->sMPath) ) ;

  STRNCPY ( a_pstDstItem->stArgFields.sArgs, a_stSrcItem.stArgFields.sArgs, sizeof(a_pstDstItem->stArgFields.sArgs) ) ;

  a_pstDstItem->stArgFields.iArgsCnt =
        tsec_parse_args ( a_pstDstItem->stArgFields.sArgs, a_pstDstItem->stArgFields.stArgs, TSEC_MAX_ARGS ) ;

  tlog_debug(g_pstCat,0,0, "RETN: tsec_dup_info_i ( const TSECSOITEM, TSECSOITEM * )=%i", iRet ) ;

  return iRet ;
}

