/**
********************************************************************************
\file   appif/appif.h

\brief  Header file for the application interface main module

This file contains definitions for the application interface main module.

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

#ifndef _INC_appif_H_
#define _INC_appif_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <appif/pcpglobal.h>

#include <oplk/oplk.h>

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

/**
 *  \brief Direction of the PDO
 */
typedef enum {
    tPdoDirInvalid = 0x00,
    tPdoDirRpdo    = 0x01,
    tPdoDirTpdo    = 0x02,
} tAppIfPdoDir;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

tAppIfStatus appif_init(UINT8 nodeId_p, tAppIfCritSec pfnCritSec_p);
void appif_exit(void);
tAppIfStatus appif_configureModules(void);

tAppIfStatus appif_handleAsync(void);
tAppIfStatus appif_handleSync(void);

void appif_pdoProcFinished(tAppIfPdoDir pdoDir_p);
tAppIfStatus appif_sdoAccFinished(tSdoComFinished* pSdoComFinParam_p );


#endif /* _INC_appif_H_ */


