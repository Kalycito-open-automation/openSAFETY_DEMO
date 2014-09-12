/**
********************************************************************************
\file   config/tpdo.h

\brief  This header defines the configurable parameters of the tpdo module

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

#ifndef _INC_config_tpdo_H_
#define _INC_config_tpdo_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <libappifcommon/global.h>

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

#define TPDO_NUM_OBJECTS    4       ///< Number of mapped TPDO objects

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------
/**
 * \brief List of all mappable objects
 */
typedef struct {
    UINT8          digitalOutput0;   ///< Digital output TPDO object 0
    UINT8          digitalOutput1;   ///< Digital output TPDO object 1
    UINT8          digitalOutput2;   ///< Digital output TPDO object 2
    UINT8          digitalOutput3;   ///< Digital output TPDO object 3
} tTpdoMappedObj;

//------------------------------------------------------------------------------
// offsetof defines
//------------------------------------------------------------------------------
#define TBUF_TPDO_DIGOUTPUT0_OFF     offsetof(tTpdoMappedObj, digitalOutput0)
#define TBUF_TPDO_DIGOUTPUT1_OFF     offsetof(tTpdoMappedObj, digitalOutput1)
#define TBUF_TPDO_DIGOUTPUT2_OFF     offsetof(tTpdoMappedObj, digitalOutput2)
#define TBUF_TPDO_DIGOUTPUT3_OFF     offsetof(tTpdoMappedObj, digitalOutput3)

//------------------------------------------------------------------------------
// object linking parameters
//------------------------------------------------------------------------------

// Link between object and buffer address: objIdx | objSubIdx | addressOffset            | objSize
#define TPDO_LINKING_LIST_INIT_VECTOR   { {0x6000 , 0x01      , TBUF_TPDO_DIGOUTPUT0_OFF , 1       }, \
                                          {0x6000 , 0x02      , TBUF_TPDO_DIGOUTPUT1_OFF , 1       }, \
                                          {0x6000 , 0x03      , TBUF_TPDO_DIGOUTPUT2_OFF , 1       }, \
                                          {0x6000 , 0x04      , TBUF_TPDO_DIGOUTPUT3_OFF , 1       }  \
                                        }

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#endif /* _INC_config_tpdo_H_ */

