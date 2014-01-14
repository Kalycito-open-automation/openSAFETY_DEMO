/**
********************************************************************************
\file   appif/internal/async.h

\brief  Internal header file for async module

This file contains internal definitions for the asynchronous module.

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

#ifndef _INC_appif_int_async_H_
#define _INC_appif_int_async_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <appif/pcpglobal.h>

#include <appif/tbuf.h>
#include <appif/fifo.h>

#include <appifcommon/timeout.h>
#include <config/tbuflayoutasync.h>

#include <Epl.h>

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

/**
 * \brief State machine type for the consuming transmit buffer
 */
typedef enum {
    kConsTxStateInvalid            = 0x00,
    kConsTxStateWaitForFrame       = 0x01,
    kConsTxStateProcessFrame       = 0x02,
    kConsTxStateWaitForTxFinished  = 0x03,
    kConsTxStateWaitForNextArpRetry = 0x04,
    kConsTxStateTxFinished         = 0x05,
} tConsTxState;

/**
 * \brief State machine type for the producing receive buffer
 */
typedef enum {
    kProdRxStateInvalid            = 0x00,
    kProdRxStateWaitForFrame       = 0x01,
    kProdRxStateRepostFrame        = 0x02,
} tProdRxState;

/**
 * \brief
 */
typedef struct {
    UINT8    ssdoStubDataBuff_m[SSDO_STUB_DATA_DOM_SIZE];
    UINT32   ssdoStubSize_m;
} tProdRxBuffer;

/**
 * \brief Parameter type of the consuming transmit buffer
 */
typedef struct {
    tTbufInstance     pTbufConsTxInst_m;    ///< Instance pointer to the consuming transmit triple buffer
    tSeqNrValue       currConsSeq_m;        ///< Consuming buffer sequence number
    tConsTxState      consTxState_m;        ///< State of the consuming transmit buffer
    tEplSdoComConHdl  sdoComConHdl_m;       ///< SDO connection handler
    UINT8*            pConsTxPayl_m;        ///< Pointer to transmit buffer
    tTimeoutInstance  pArpTimeoutInst_m;    ///< Timer for ARP request retry
} tAsyncConsTx;

/**
 * \brief Parameter type of the producing receive buffer
 */
typedef struct {
    tTbufInstance     pTbufProdRxInst_m;    ///< Instance pointer to the producing receive triple buffer
    tFifoInstance     pRxFifoInst_m;        ///< producing receive FIFO instance pointer
    tProdRxState      prodRxState_m;        ///< State of the producing receive buffer
    tSeqNrValue       currProdSeq_m;        ///< Current producing buffer sequence number
    tProdRxBuffer     prodRecvBuff_m;       ///< Producing receive buffer for packet retransmission
    tTimeoutInstance  pTimeoutInst_m;       ///< Timer for asynchronous transmissions over the tbuf
} tAsyncProdRx;

/**
\brief Asynchronous channel user instance

The asynchronous instance holds configuration information of each asynchronous
channel.
*/
struct eAsyncInstance
{
    tAsyncChanNum   instId_m;             ///< Id of the async instance
    tAsyncConsTx    consTxParam_m;        ///< Consuming receive buffer parameters
    tAsyncProdRx    prodRxParam_m;        ///< Producing transmit buffer parameters
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#endif /* _INC_appif_int_async_H_ */


