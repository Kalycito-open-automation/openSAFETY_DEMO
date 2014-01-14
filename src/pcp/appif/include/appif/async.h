/**
********************************************************************************
\file   appif/async.h

\brief  Header file for async module

This file contains definitions for the async module.

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

#ifndef _INC_appif_async_H_
#define _INC_appif_async_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <appif/pcpglobal.h>

#include <config/tbuflayoutasync.h>

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

/**
 * \brief Asynchronous channel initialization parameters
 */
typedef struct {
    tAsyncChanNum            chanId_m;          ///< Id of the asynchronous channel

    UINT8                    tbufRxId_m;        ///< Id of the receive triple buffer
    tTbufAsyncRxStructure*   pTbufRxBase_m;     ///< Base address of the receive triple buffer
    UINT32                   tbufRxSize_m;      ///< Size of the receive triple buffer
    UINT8*                   pProdAckBase_m;    ///< Producer acknowledge register base

    UINT8                    tbufTxId_m;        ///< Id of the transmit triple buffer
    tTbufAsyncTxStructure*   pTbufTxBase_m;     ///< Base address of the transmit triple buffer
    UINT32                   tbufTxSize_m;      ///< Size of the transmit triple buffer
    UINT8*                   pConsAckBase_m;    ///< Consumer acknowledge register base
} tAsyncInitStruct;

typedef struct eAsyncInstance *tAsyncInstance;

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
void async_init(UINT8 nodeId_p, UINT16 idxSsdoStub_p, UINT16 idxSsdoStubData_p);
tAsyncInstance async_create(tAsyncInitStruct* pInitParam_p);
void async_destroy(tAsyncInstance pInstance_p);
tAppIfStatus async_process(tAsyncInstance pInstance_p);
tAppIfStatus async_consTxTransferFinished(tAsyncInstance pInstance_p);
tAppIfStatus async_handleIncoming(tAsyncInstance pInstance_p);

#endif /* _INC_appif_async_H_ */


