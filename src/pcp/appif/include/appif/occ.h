/**
********************************************************************************
\file   appif/occ.h

\brief  Header file for configuration channel output module

This file contains definitions for the configuration channel output module.

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

#ifndef _INC_appif_occ_H_
#define _INC_appif_occ_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <appif/pcpglobal.h>

#include <appifcommon/cc.h>

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

/**
 * \brief Initialization parameters of the occ module
 */
typedef struct {
    UINT8                 id_m;             ///< Id of the triple buffer
    tTbufCcStructure*     pTbufBase_m;      ///< Base address of the triple buffer
    UINT8*                pProdAckBase_m;   ///< Producer acknowledge register base
    UINT32                tbufSize_m;       ///< Size of the triple buffer
} tOccInitStruct;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
tAppIfStatus occ_init(tOccInitStruct* pInitParam_p);
void occ_exit(void);
tAppIfStatus occ_handleOutgoing(void);

#endif /* _INC_appif_occ_H_ */


