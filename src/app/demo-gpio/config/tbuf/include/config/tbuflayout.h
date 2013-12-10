/**
********************************************************************************
\file   config/tbuflayout.h

\brief  Global header file for the triple buffers layout

This file contains of the layout of the triple buffers.

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

#ifndef _INC_config_tbuflayout_H_
#define _INC_config_tbuflayout_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include <config/tbuflayoutstatus.h>
#include <config/tbuflayoutcc.h>
#include <config/tbuflayoutrpdo.h>
#include <config/tbuflayouttpdo.h>
#include <config/tbuflayoutssdo.h>

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

typedef UINT32 tTbufAckRegister;    ///< Acknowledge register size type

/**
 * \brief This type assigns a number to each triple buffer
 */
typedef enum {
    kTbufAckRegisterCons     = 0x00,     ///< ID of the consumer acknowledge register
    kTbufNumStatusOut        = 0x01,     ///< ID of the status output triple buffer
    kTbufNumOutputConfChan   = 0x02,     ///< ID of the output configuration channel triple buffer
    kTbufNumRpdoImage        = 0x03,     ///< ID of the RPDO triple buffer image
    kTbufNumSsdoReceive0     = 0x04,     ///< ID of the SSDO receive triple buffer 0
    kTbufNumStatusIn         = 0x05,     ///< ID of the status input triple buffer
    kTbufNumInputConfChan    = 0x06,     ///< ID of the input configuration channel triple buffer
    kTbufNumTpdoImage        = 0x07,     ///< ID of the TPDO triple buffer image
    kTbufNumSsdoTransmit0    = 0x08,     ///< ID of the SSDO transmit triple buffer 0
    kTbufAckRegisterProd     = 0x09,     ///< ID of the producer acknowledge register
    kTbufCount               = 0x0A,     ///< Total count of triple buffers
} tTbufNumLayout;

/**
 * \brief This type provides the memory layout of the triple buffer core
 */
typedef struct {
    tTbufAckRegister          consAck_m;            ///< Consumer ACK register
    // Producer triple buffers
    tTbufStatusOutStructure   tbufStatusOut_m;      ///< Status outgoing triple buffer
    tTbufCcStructure          tbufOcc_m;            ///< Output configuration channel triple buffer
    tTbufRpdoImage            tbufRpdoImg_m;        ///< Structure of the rpdo image triple buffer
    tTbufSsdoRxStructure      tbufSsdoRx0_m;        ///< SSDO receive channel 0 structure

    // Consumer triple buffers
    tTbufStatusInStructure    tbufStatusIn_m;       ///< Status incoming triple buffer
    tTbufCcStructure          tbufIcc_m;            ///< Input configuration channel triple buffer
    tTbufTpdoImage            tbufTpdoImg_m;        ///< Structure of the tpdo image triple buffer
    tTbufSsdoTxStructure      tbufSsdoTx0_m;        ///< SSDO transmit channel 0 structure

    tTbufAckRegister          prodAck_m;            ///< Producer ACK register
} tTbufMemLayout;

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

/**
 * \brief Application interface module list
 */
#define APPIF_MODULE_INTEGRATION  (0 \
                                | APPIF_MODULE_STATUS \
                                | APPIF_MODULE_CC \
                                | APPIF_MODULE_PDO \
                                | APPIF_MODULE_SSDO \
                                )

// Detect configuration errors
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_STATUS)) == 0)
#error "Status module is not active! This module is mandatory for the application interface"
#endif

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#endif /* _INC_config_tbuflayout_H_ */

