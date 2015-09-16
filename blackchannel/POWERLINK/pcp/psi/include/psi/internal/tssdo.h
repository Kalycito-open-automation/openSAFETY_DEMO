/**
********************************************************************************
\file   psi/internal/tssdo.h

\brief  Internal header file of the transmit SSDO module

This file contains internal definitions for the transmit SSDO module. It
defines the internal instance parameters of the module.

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

#ifndef _INC_psi_int_tssdo_H_
#define _INC_psi_int_tssdo_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <psi/pcpglobal.h>

#include <psi/tbuf.h>

#include <libpsicommon/timeout.h>
#include <config/ssdo.h>

#include <oplk/oplk.h>

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
\brief SSDO transmit channel user instance

The SSDO instance holds configuration information of each SSDO channel.
*/
struct eTssdoInstance
{
    tSsdoChanNum      instId_m;             ///< Id of the SSDO instance

    tTbufInstance     pTbufConsTxInst_m;    ///< Instance pointer to the consuming transmit triple buffer
    tSeqNrValue       currConsSeq_m;        ///< Consuming buffer sequence number
    tConsTxState      consTxState_m;        ///< State of the consuming transmit buffer
    tSdoComConHdl     sdoComConHdl_m;       ///< SDO connection handler
    UINT8*            pConsTxPayl_m;        ///< Pointer to transmit buffer
    tTimeoutInstance  pArpTimeoutInst_m;    ///< Timer for ARP request retry
};

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#endif /* _INC_psi_int_tssdo_H_ */


