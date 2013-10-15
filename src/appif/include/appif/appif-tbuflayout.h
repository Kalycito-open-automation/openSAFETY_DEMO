/**
********************************************************************************
\file   appif-tbuflayout.h

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

#ifndef _INC_APPIF_TBUFLAYOUT_H_
#define _INC_APPIF_TBUFLAYOUT_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <appif/appif-tbuflayoutstatus.h>
#include <appif/appif-tbuflayoutcc.h>
#include <appif/appif-tbuflayoutrpdo.h>
#include <appif/appif-tbuflayouttpdo.h>
#include <appif/appif-tbuflayoutasync.h>

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

typedef UINT32 tTbufAckRegister;    ///< Acknowledge register size type

/**
 * \brief This type assigns a number to each triple buffer
 */
typedef enum {
    kTbufAckRegisterCons     = 0x00,     ///< Number of the consumer acknowledge register
    kTbufNumStatusOut        = 0x01,      ///< Number of the status output triple buffer
    kTbufNumOutputConfChan   = 0x02,     ///< Number of the output configuration channel triple buffer
    kTbufNumRpdoImage        = 0x03,     ///< Number of the RPDO triple buffer image
    kTbufNumAsyncReceive0    = 0x04,     ///< Number of the asynchronous rx triple buffer 0
    kTbufNumStatusIn         = 0x05,     ///< Number of the status input triple buffer
    kTbufNumInputConfChan    = 0x06,     ///< Number of the input configuration channel triple buffer
    kTbufNumTpdoImage        = 0x07,     ///< Number of the TPDO triple buffer image
    kTbufNumAsyncTransmit0   = 0x08,     ///< Number of the asynchronous tx triple buffer 0
    kTbufAckRegisterProd     = 0x09,     ///< Number of the producer acknowledge register
    kTbufCount               = 0x0A,     ///< Total number of triple buffers
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
    tTbufAsyncRxStructure     tbufAsyncRx0_m;       ///< Asynchronous receive channel 0 structure

    // Consumer triple buffers
    tTbufStatusInStructure    tbufStatusIn_m;       ///< Status incoming triple buffer
    tTbufCcStructure          tbufIcc_m;            ///< Input configuration channel triple buffer
    tTbufTpdoImage            tbufTpdoImg_m;        ///< Structure of the tpdo image triple buffer
    tTbufAsyncTxStructure     tbufAsyncTx0_m;       ///< Asynchronous transmit channel 0 structure

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
                                | APPIF_MODULE_ASYNC \
                                )

// Detect configuration errors
#if(((APPIF_MODULE_INTEGRATION) & (APPIF_MODULE_STATUS)) == 0)
#error "Status module is not active! This module is mandatory for the application interface"
#endif

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#endif /* _INC_APPIF_TBUFLAYOUT_H_ */

