/*****************************************
 Copyright (c) 2001-2007
 Sigma Designs, Inc. All Rights Reserved
 Proprietary and Confidential
 *****************************************/
/**
  @file   rmmmimplementation.h
  @brief

  @author Sebastian Frias Feltrer
  @date   2007-10-12
*/

/*
  **********************************************
  DISCLAIMER:

  - THIS IS TEST CODE, provided as sample code
  to help you understand what you should do to
  develop your own application based in RMFP.

  - This is NOT production grade code; It is not
  even a library, so any API defined here, CAN
  and WILL CHANGE without notice.

  **********************************************
*/

#ifndef __RMMMIMPLEMENTATION_H__
#define __RMMMIMPLEMENTATION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ALLOW_OS_CODE 1
#include "rmdef/rmdef.h"


// this function is declared locally and it is not inside rmmm.h that's why we have it here
void RMCheckMemory(void);


#endif // __RMMMIMPLEMENTATION_H__

