/**
********************************************************************************
\file   appif-stream.h

\brief  Streaming module  header for streamed transfer of input/output data

This module handles the transfer of the input of output buffers via the steam
handler. It enables to insert pre- and post actions before and after transfer.

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

#ifndef _INC_APPIF_STREAM_H_
#define _INC_APPIF_STREAM_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <appif-apglobal.h>

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

/**
 * \brief Descriptor element of segmented descriptor list
 */
typedef struct {
    UINT8*     pBuffRxBase_m;    ///< Base address of the receive buffer
    UINT8*     pBuffTxBase_m;    ///< Base address of the transmit buffer
    UINT16     transSize_m;     ///< Size to be transfered
} tBuffSegDesc;

/**
 * \brief Parameters of the stream handler callback function
 */
typedef struct {
    tBuffSegDesc* pFirstSegDesc_m;      ///< First segmented descriptor
    UINT8 segDescCount_m;               ///< Number of segmented descriptors
} tHandlerParam;

/**
 * \brief Handler callback foe input/output stream filling
 */
typedef tAppIfStatus (*tStreamHandler) (tHandlerParam* pHandlParam_p);

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------


#endif /* _INC_APPIF_STREAM_H_ */


