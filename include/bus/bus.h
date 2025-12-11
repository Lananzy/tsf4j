/** @file $RCSfile: bus.h,v $
  Invoked function header file for tsf4g-bus module
  $Id: bus.h,v 1.1.1.1 2008/05/28 07:34:59 kent Exp $
@author $Author: kent $
@date $Date: 2008/05/28 07:34:59 $
@version $Revision: 1.1.1.1 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef _BUS_H
#define _BUS_H

#include "bus/bus_platform.h"
#include "bus_route.h"


/**
  @brief Global init function
  @retval 0 -- service inited ok
  @retval !0 -- service inited failed
  @param 
  @note This function should be invoked before using any bus functions
*/
int bus_init ( IN const unsigned int a_iMapKey, IN const unsigned int a_iSize ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_new ( INOUT int *a_piHandle ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_getopt ( IN const int a_iHandle, IN const int a_iLevel, IN const int a_iOptName, IN void *a_pvOptVal, IN size_t *a_piOptLen ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_setopt (
	IN const int a_iHandle,
	IN const int a_iLevel,
	IN const int a_iOptName,
	IN const void *a_pvOptVal,
	IN const size_t a_iOptLen
) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_bind ( IN const int a_iHandle, IN const char *a_szSrcAddr ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_connect ( IN const int a_iHandle, IN const char *a_szDstAddr ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_send (
	IN const int a_iHandle,
	IN const unsigned int a_iDst,
	IN const void *a_pvData,
	IN const size_t a_iDataLen,
	IN const int a_iFlag
) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_sendv ( IN const int a_iHandle, IN const int a_iDst, IN const DATAVEC *a_ptMultiData, IN const int a_iFlag ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_recv (
	IN const int a_iHandle,
	IN const unsigned int a_iSrc,
	OUT void *a_pvData,
	INOUT size_t *a_piDataLen,
	IN const int a_iFlag
) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_recvv ( IN const int a_iHandle, IN const int a_iSrc, IN DATAVEC *a_ptMultiData, IN const int a_iFlag ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_forward (
	IN const int a_iHandle,
	IN const unsigned int a_iDst,
	IN const void *a_pvData,
	IN const size_t a_iDataLen,
	IN const int a_iFlag
) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_forwardv ( IN const int a_iHandle, IN const int a_iDst, IN const DATAVEC *a_ptMultiData, IN const int a_iFlag ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_backward (
	IN const int a_iHandle,
	INOUT void *a_pvData,
	IN const size_t a_iDataLen,
	IN const int a_iFlag
) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_backwardv ( IN const int a_iHandle, IN const int a_iSrc, IN DATAVEC *a_ptMultiData, IN const int a_iFlag ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_delete ( INOUT int *a_piHandle ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int bus_fini ( ) ;


#endif /**< _BUS_H */

