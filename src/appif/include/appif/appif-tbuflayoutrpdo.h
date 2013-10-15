/**
********************************************************************************
\file   <appifff/appif-tbuflayoutrpdo.h

\brief  Header defines the layout of the rpdo triple buffer

This header gives the basic structure of the receive pdo triple buffers.

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

#ifndef _INC_APPIF_TBUFLAYOUTRPDO_H_
#define _INC_APPIF_TBUFLAYOUTRPDO_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <appif/appif-global.h>

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------
#define RPDO_NUM_OBJECTS    1       ///< Number of mapped RPDO objects

#define R0SPDO_DOM_SIZE     24      ///< Size of the RPDO domain object

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

/**
 * \brief List of all mappable objects
 */
typedef struct {
    UINT8          r0SpdoDom_m[R0SPDO_DOM_SIZE];
} tRpdoMappedObj;

/**
 * \brief The layout of the receive PDO image
 */
typedef struct {
    UINT32         relativeTimeLow_m;
    tRpdoMappedObj mappedObjList_m;
} tTbufRpdoImage;

//------------------------------------------------------------------------------
// offsetof defines
//------------------------------------------------------------------------------
#define TBUF_RPDO_RELTIME_OFF       offsetof(tTbufRpdoImage, relativeTimeLow_m)
#define TBUF_RPDO_MAPPED_OBJ_OFF    offsetof(tTbufRpdoImage, mappedObjList_m)

#define TBUF_RPDO0_SPDO_DOM_OFF     TBUF_RPDO_MAPPED_OBJ_OFF + offsetof(tRpdoMappedObj, r0SpdoDom_m)

//------------------------------------------------------------------------------
// object linking parameters
//------------------------------------------------------------------------------

// List of object index, subindex and destination address offset
#define RPDO_LINKING_LIST_INIT_VECTOR   { {0x6200, 0x00, TBUF_RPDO0_SPDO_DOM_OFF, R0SPDO_DOM_SIZE} \
                                        }

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#endif /* _INC_APPIF_TBUFLAYOUTRPDO_H_ */

