/**
********************************************************************************
\file   pcptarget/target.h

\brief  Platform dependent header file for the application interface.

This file contains of platform dependent code. (The platform is Altera Nios2)

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

#ifndef _INC_pcptarget_H_
#define _INC_pcptarget_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <global.h>    // POWERLINK stack global.h

#include <system.h>
#include <unistd.h>    // for usleep
#include <stdlib.h>    // for malloc, free
#include <string.h>    // for memcpy() memset()

#include <EplAmi.h>


//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

#define ACK_REGISTER_COUNT      2       ///< Number of acknowledge registers


#ifdef TRIPLEBUFFER_BASE
  #define TBUF_BASE_ADDRESS           TRIPLEBUFFER_BASE
#else
  #error "Triple buffer base address invalid! Check your FPGA design parameters!"
#endif  //SOPC_TBUF_BASE_ADDRESS

// Number of producing triple buffers (Consumer <-> Producer buffer defines are swapped in system.h)
#ifdef TRIPLEBUFFER_TBUF_NUM_CON
  #define PROD_TRIPLE_BUFFER_COUNT     TRIPLEBUFFER_TBUF_NUM_CON
#else
  #error "Triple buffer consumer count not found! Check your FPGA design parameters!"
#endif

// Number of consumer triple buffers (Consumer <-> Producer buffer defines are swapped in system.h)
#ifdef TRIPLEBUFFER_TBUF_NUM_PRO
  #define CONS_TRIPLE_BUFFER_COUNT     TRIPLEBUFFER_TBUF_NUM_PRO
#else
  #error "Triple buffer producer count not found! Check your FPGA design parameters!"
#endif

#define TRIPLE_BUFFER_COUNT    (PROD_TRIPLE_BUFFER_COUNT + CONS_TRIPLE_BUFFER_COUNT + \
                                ACK_REGISTER_COUNT)

// Guard standard library functions
#define APPIF_MEMSET(ptr, bVal, bCnt)  memset(ptr, bVal, bCnt)
#define APPIF_MEMCPY(ptr, bVal, bSize) memcpy(ptr, bVal, bSize)
#define APPIF_USLEEP(x)                usleep(x)
#define APPIF_MALLOC(siz)              malloc(siz)
#define APPIF_FREE(ptr)                free(ptr)

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#endif /* _INC_pcptarget_H_ */


