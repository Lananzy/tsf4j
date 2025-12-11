/** @file $RCSfile: bus_channel.h,v $
  Invoked function header file for tsf4g-bus module
  $Id: bus_channel.h,v 1.1.1.1 2008/05/28 07:34:59 kent Exp $
@author $Author: kent $
@date $Date: 2008/05/28 07:34:59 $
@version $Revision: 1.1.1.1 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef _BUS_CHANNEL_H
#define _BUS_CHANNEL_H

#include "bus/bus_platform.h"
#include "bus/bus.h"
#include "bus/bus_misc.h"

#include "bus/og_bus.h"


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param 
  @note
*/
int channelGet ( CHANNEL* pstChannel, BUSPKG *pstPkg ) ;


/**
  @brief
  @retval 0 --
  @retval !0 --
  @param
  @note
*/
int channelPush ( CHANNEL* pstChannel, BUSPKG *pstPkg ) ;


#endif /**< _BUS_CHANNEL_H */

