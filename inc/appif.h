/**
********************************************************************************
\file   appif.h

\brief  Header file for application interface module

This file contains definitions for the application interface module.

*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2013, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holders nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------*/

#ifndef _INC_APPIF_H_
#define _INC_APPIF_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <appif-apglobal.h>
#include <appif-tbuflayout.h>

#include <appif-stream.h>
#include <appif-status.h>
#include <appif-pdo.h>
#include <appif-async.h>
#include <appif-cc.h>



//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

/**
 * \brief Application interface initialization parameters
 */
typedef struct {
    tTbufDescriptor*    pBuffDescList_m;      ///< Triple buffer descriptor list
    tStreamHandler      pfnStreamHandler_m;   ///< Stream handler callback function
    tAppIfCritSec       pfnEnterCritSec_m;    ///< Pointer to critical section entry function
    UINT8               countConsBuff_m;      ///< Number of consuming buffer
    UINT8               countProdBuff_m;      ///< Number of producing buffer
} tAppIfInitParam;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
tAppIfStatus appif_init(tAppIfInitParam* pInitParam_p);
void appif_exit(void);

tAppIfStatus appif_finishModuleInit(void);
tAppIfStatus appif_processSync(void);
tAppIfStatus appif_processAsync(tAsyncInstance* ppInstance_p);

#endif /* _INC_APPIF_H_ */


