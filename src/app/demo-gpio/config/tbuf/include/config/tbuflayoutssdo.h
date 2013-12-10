/**
********************************************************************************
\file   config/tbuflayoutssdo.h

\brief  Header defines the layout of the SSDO triple buffer

This header gives the basic structure of the SSDO receive and
transmit buffers.

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

#ifndef _INC_config_tbuflayoutssdo_H_
#define _INC_config_tbuflayoutssdo_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <appifcommon/global.h>

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------
#define SSDO_STUB_OBJECT_INDEX           0x3000     ///< Object index of the SSDO stub
#define SSDO_STUB_DATA_OBJECT_INDEX      0x3100     ///< Object index of the SSDO stub data


#define SSDO_STUB_DATA_DOM_SIZE     0x20      ///< Size of the SSDO stub data object
#define TSSDO_TRANSMIT_DATA_SIZE    0x20      ///< Size of the SSDO channel transmit data

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

/**
 * \brief Number of SSDO channel instances
 */
typedef enum {
    kNumSsdoChan0     = 0x00,
    kNumSsdoInstCount = 0x01,
} tSsdoChanNum;

/**
 * \brief Memory layout of the receive channel
 */
typedef struct {
    UINT8   seqNr_m;
    UINT8   reserved;
    UINT16  paylSize_m;
    UINT8   ssdoStubDataDom_m[SSDO_STUB_DATA_DOM_SIZE];
} tTbufSsdoRxStructure;

/**
 * \brief Memory layout of the transmit channel
 */
typedef struct {
    UINT8   seqNr_m;
    UINT8   reserved;
    UINT16  paylSize_m;
    UINT8   tssdoTransmitData_m[TSSDO_TRANSMIT_DATA_SIZE];
} tTbufSsdoTxStructure;

//------------------------------------------------------------------------------
// offsetof defines
//------------------------------------------------------------------------------

#define TBUF_SSDORX_SEQNR_OFF                 offsetof(tTbufSsdoRxStructure, seqNr_m)
#define TBUF_SSDORX_PAYLSIZE_OFF              offsetof(tTbufSsdoRxStructure, paylSize_m)
#define TBUF_SSDORX_SSDO_STUB_DATA_DOM_OFF    offsetof(tTbufSsdoRxStructure, ssdoStubDataDom_m)

#define TBUF_SSDOTX_SEQNR_OFF                 offsetof(tTbufSsdoTxStructure, seqNr_m)
#define TBUF_SSDOTX_PAYLSIZE_OFF              offsetof(tTbufSsdoTxStructure, paylSize_m)
#define TBUF_SSDOTX_TSSDO_TRANSMIT_DATA_OFF   offsetof(tTbufSsdoTxStructure, tssdoTransmitData_m)

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#endif /* _INC_config_tbuflayoutssdo_H_ */

