/**
********************************************************************************
\file   config/tbuflayouttpdo.h

\brief  Header defines the layout of the tpdo triple buffer

This header gives the basic structure of the transmit pdo triple buffers.

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

#ifndef _INC_config_tbuflayouttpdo_H_
#define _INC_config_tbuflayouttpdo_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <appifcommon/global.h>

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

#define TPDO_NUM_OBJECTS    1       ///< Number of mapped TPDO objects

#define T0SPDO_DOM_SIZE     36      ///< Size of the TPDO domain object

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------
/**
 * \brief List of all mappable objects
 */
typedef struct {
    UINT8  t0SpdoDom_m[T0SPDO_DOM_SIZE];
} tTpdoMappedObj;

/**
 * \brief The layout of the transmit PDO image
 */
typedef struct {
    tTpdoMappedObj mappedObjList_m;
} tTbufTpdoImage;

//------------------------------------------------------------------------------
// offsetof defines
//------------------------------------------------------------------------------
#define TBUF_TPDO_MAPPED_OBJ_OFF    offsetof(tTbufTpdoImage, mappedObjList_m)

#define TBUF_TPDO0_SPDO_DOM_OFF     TBUF_TPDO_MAPPED_OBJ_OFF + offsetof(tTpdoMappedObj, t0SpdoDom_m)

//------------------------------------------------------------------------------
// object linking parameters
//------------------------------------------------------------------------------

// List of object index, subindex and destination address offset
#define TPDO_LINKING_LIST_INIT_VECTOR   { {0x6000, 0x00, TBUF_TPDO0_SPDO_DOM_OFF, T0SPDO_DOM_SIZE} \
                                        }

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#endif /* _INC_config_tbuflayouttpdo_H_ */

