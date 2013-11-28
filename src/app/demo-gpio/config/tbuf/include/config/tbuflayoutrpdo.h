/**
********************************************************************************
\file   config/tbuflayoutrpdo.h

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

#ifndef _INC_config_tbuflayoutrpdo_H_
#define _INC_config_tbuflayoutrpdo_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <appifcommon/global.h>

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------
#define RPDO_NUM_OBJECTS    4       ///< Number of mapped RPDO objects

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

/**
 * \brief List of all mappable objects
 */
typedef struct {
    UINT8          digitalInput0;   ///< Digital input RPDO object 0
    UINT8          digitalInput1;   ///< Digital input RPDO object 1
    UINT8          digitalInput2;   ///< Digital input RPDO object 2
    UINT8          digitalInput3;   ///< Digital input RPDO object 3
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

#define TBUF_DIGINPUT0_SPDO_DOM_OFF     TBUF_RPDO_MAPPED_OBJ_OFF + offsetof(tRpdoMappedObj, digitalInput0)
#define TBUF_DIGINPUT1_SPDO_DOM_OFF     TBUF_RPDO_MAPPED_OBJ_OFF + offsetof(tRpdoMappedObj, digitalInput1)
#define TBUF_DIGINPUT2_SPDO_DOM_OFF     TBUF_RPDO_MAPPED_OBJ_OFF + offsetof(tRpdoMappedObj, digitalInput2)
#define TBUF_DIGINPUT3_SPDO_DOM_OFF     TBUF_RPDO_MAPPED_OBJ_OFF + offsetof(tRpdoMappedObj, digitalInput3)

//------------------------------------------------------------------------------
// object linking parameters
//------------------------------------------------------------------------------

// List of object index, subindex and destination address offset
#define RPDO_LINKING_LIST_INIT_VECTOR   { {0x6200, 0x01, TBUF_DIGINPUT0_SPDO_DOM_OFF, 1}, \
                                          {0x6200, 0x02, TBUF_DIGINPUT1_SPDO_DOM_OFF, 1}, \
                                          {0x6200, 0x03, TBUF_DIGINPUT2_SPDO_DOM_OFF, 1}, \
                                          {0x6200, 0x04, TBUF_DIGINPUT3_SPDO_DOM_OFF, 1}  \
                                        }

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#endif /* _INC_config_tbuflayoutrpdo_H_ */

