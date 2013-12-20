/**
********************************************************************************
\file   ami/ami.h

\brief  Header file of the abstract memory interface library

This file provides an abstract memory interface for endian conversion.

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


#ifndef _INC_ami_H_
#define _INC_ami_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------

#include <apptarget/target.h>

//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

#define AmiSetByteToBe(pAddr_p, bByteVal_p) {*(UINT8  *)(pAddr_p) = (bByteVal_p);}
#define AmiSetByteToLe(pAddr_p, bByteVal_p) {*(UINT8  *)(pAddr_p) = (bByteVal_p);}

#define AmiGetByteFromBe(pAddr_p)    (*(UINT8 *)(pAddr_p))
#define AmiGetByteFromLe(pAddr_p)    (*(UINT8 *)(pAddr_p))

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------
DLLEXPORT void AmiSetWordToBe(void* pAddr_p, UINT16 wordVal_p);
DLLEXPORT void AmiSetWordToLe(void* pAddr_p, UINT16 wordVal_p);
DLLEXPORT UINT16 AmiGetWordFromBe(void* pAddr_p);
DLLEXPORT UINT16 AmiGetWordFromLe(void* pAddr_p);

DLLEXPORT void AmiSetDwordToBe(void* pAddr_p, UINT32 dwordVal_p);
DLLEXPORT void AmiSetDwordToLe(void* pAddr_p, UINT32 dwordVal_p);
DLLEXPORT UINT32 AmiGetDwordFromBe(void* pAddr_p);
DLLEXPORT UINT32 AmiGetDwordFromLe(void* pAddr_p);

DLLEXPORT void AmiSetQword64ToBe(void* pAddr_p, UINT64 qwordVal_p);
DLLEXPORT void AmiSetQword64ToLe(void* pAddr_p, UINT64 qwordVal_p);
DLLEXPORT UINT64 AmiGetQword64FromBe(void* pAddr_p);
DLLEXPORT UINT64 AmiGetQword64FromLe(void* pAddr_p);

#endif  /* _INC_ami_H_ */
