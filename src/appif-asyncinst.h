/**
********************************************************************************
\file   appif-asyncinst.h

\brief  Application interface async module internal instance header

Internal header for the asynchronous channel which holds the instance
declaration of one channel.

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

#ifndef _INC_APPIF_ASYNCINST_H_
#define _INC_APPIF_ASYNCINST_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <appif-tbuflayoutasync.h>
#include <appif-timeout.h>

#include "appif-async.h"

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

typedef struct {
    UINT8                   isLocked_m;        ///< Is buffer free for filling
    tTbufAsyncTxStructure*  pAsyncTxPayl_m;    ///< Pointer to transmit buffer
} tTbufAsyncTxBuffer;

/**
 * \brief Parameter type of the transmit buffer
 */
typedef struct {
    tTbufNumLayout        idTxBuff_m;           ///< Id of the transmit buffer
    tTbufAsyncTxBuffer    asyncTxBuffer_m;      ///< Asynchronous transmit buffer copy
    tSeqNrValue           currTxSeqNr_m;        ///< Current transmit sequence number
    UINT8                 currTxBuffer_m;       ///< Current active transmit buffer
    tTimeoutInstance      pTimeoutInst_m;       ///< Timer instance for asynchronous transmissions
} tAsyncTxChannel;

/**
 * \brief Parameter type of the receive buffer
 */
typedef struct {
    tTbufNumLayout         idRxBuff_m;          ///< Id of the receive buffer
    tAsyncRxHandler        pfnRxHandler_m;      ///< Async module receive handler
    tTbufAsyncRxStructure* pAsyncRxBuffer_m;    ///< Pointer to receive buffer
    tSeqNrValue            currRxSeqNr_m;       ///< Current receive sequence number
    UINT8                  fRxFrameIncoming_m;  ///< Receive buffer incoming flag
} tAsyncRxChannel;

/**
\brief Asynchronous channel user instance

The asynchronous instance holds configuration information of each asynchronous
channel.
*/
struct eAsyncInstance
{
    tAsyncChanNum       chanId_m;           ///< Id of the asynchronous channel
    tAsyncTxChannel     txBuffParam_m;      ///< Parameters of the transmit channel
    tAsyncRxChannel     rxBuffParam_m;      ///< Parameters of the receive channel
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#endif /* _INC_APPIF_ASYNCINST_H_ */


