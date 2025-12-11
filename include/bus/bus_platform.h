/** @file $RCSfile: bus_platform.h,v $
  Platform compatibility for tsf4g-bus module
  $Id: bus_platform.h,v 1.1.1.1 2008/05/28 07:34:59 kent Exp $
@author $Author: kent $
@date $Date: 2008/05/28 07:34:59 $
@version $Revision: 1.1.1.1 $
@note Editor: Vim 6.3, Gcc 4.0.2, tab=4
@note Platform: Linux
*/


#ifndef _BUS_PLATFORM_H
#define _BUS_PLATFORM_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <assert.h>

#include "pal/pal.h"

#ifdef _WIN32
  #ifdef BUS_DATA_EXPORTS
  #    define BUS_DATA __declspec(dllexport)
  #else
  #    define BUS_DATA extern __declspec(dllimport)
  #endif

  #ifdef BUS_API_EXPORTS
  #    define BUS_API __declspec(dllexport)
  #else
  #    define BUS_API extern __declspec(dllimport)
  #endif
#else /* linux platform */
  #define BUS_DATA
  #define BUS_API
#endif


#endif /**< _BUS_PLATFORM_H */

