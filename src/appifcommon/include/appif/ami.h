/**
********************************************************************************
\file       ami.h

\brief      Header file for the ami endian conversion module

This header file contains definitions for the endian conversion module.

Copyright 2013 BERNECKER + RAINER, AUSTRIA, 5142 EGGELSBERG, B&R STRASSE 1
All rights reserved. All use of this software and documentation is
subject to the License Agreement located at the end of this file below.

*******************************************************************************/

#ifndef _AMI_H_
#define _AMI_H_

#include <ipcore/target.h>

/******************************************************************************/
/* includes */

/******************************************************************************/
/* defines */

/******************************************************************************/
/* external variable declarations */

/******************************************************************************/
/* global variables */

/******************************************************************************/
/* function declarations */

/******************************************************************************/
/* private functions */

/******************************************************************************/
/* functions */

//---------------------------------------------------------------------------
//
// write functions
//
// To save code space the function AmiSetByte is replaced by
// an macro.
// void AmiSetByte  (void * pAddr_p, UINT8  bByteVal_p);
//---------------------------------------------------------------------------

#define AmiSetByteToBe(pAddr_p, bByteVal_p)  {*(UINT8  *)(pAddr_p) = (bByteVal_p);}
#define AmiSetByteToLe(pAddr_p, bByteVal_p)  {*(UINT8  *)(pAddr_p) = (bByteVal_p);}

void AmiSetWordToBe  (void* pAddr_p, UINT16 wWordVal_p);
void AmiSetDwordToBe (void* pAddr_p, UINT32 dwDwordVal_p);
void AmiSetWordToLe  (void* pAddr_p, UINT16 wWordVal_p);
void AmiSetDwordToLe (void* pAddr_p, UINT32 dwDwordVal_p);


//---------------------------------------------------------------------------
//
// read functions
//
// To save code space the function AmiGetByte is replaced by
// an macro.
// UINT8  AmiGetByte  (void * pAddr_p);
//---------------------------------------------------------------------------

#define AmiGetByteFromBe(pAddr_p)  (*(UINT8  *)(pAddr_p))
#define AmiGetByteFromLe(pAddr_p)  (*(UINT8  *)(pAddr_p))

UINT16 AmiGetWordFromBe  (void * pAddr_p);
UINT32 AmiGetDwordFromBe (void * pAddr_p);
UINT16 AmiGetWordFromLe  (void * pAddr_p);
UINT32 AmiGetDwordFromLe (void * pAddr_p);

//---------------------------------------------------------------------------
//
// Function:    AmiSetDword24()
//
// Description: sets a 24 bit value to a buffer
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              dwDwordVal_p    = value to set
//
// Return:      void
//
//---------------------------------------------------------------------------

void AmiSetDword24ToBe (void * pAddr_p, UINT32 dwDwordVal_p);
void AmiSetDword24ToLe (void * pAddr_p, UINT32 dwDwordVal_p);


//---------------------------------------------------------------------------
//
// Function:    AmiGetDword24()
//
// Description: reads a 24 bit value from a buffer
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      UINT32           = read value
//
//---------------------------------------------------------------------------

UINT32 AmiGetDword24FromBe (void * pAddr_p);
UINT32 AmiGetDword24FromLe (void * pAddr_p);


//#ifdef USE_VAR64

//---------------------------------------------------------------------------
//
// Function:    AmiSetQword40()
//
// Description: sets a 40 bit value to a buffer
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:      void
//
//---------------------------------------------------------------------------

void AmiSetQword40ToBe (void * pAddr_p, UINT64 qwQwordVal_p);
void AmiSetQword40ToLe (void * pAddr_p, UINT64 qwQwordVal_p);


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword40()
//
// Description: reads a 40 bit value from a buffer
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      UINT64
//
//---------------------------------------------------------------------------

UINT64 AmiGetQword40FromBe (void * pAddr_p);
UINT64 AmiGetQword40FromLe (void * pAddr_p);


//---------------------------------------------------------------------------
//
// Function:    AmiSetQword48()
//
// Description: sets a 48 bit value to a buffer
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:      void
//
//---------------------------------------------------------------------------

void AmiSetQword48ToBe (void * pAddr_p, UINT64 qwQwordVal_p);
void AmiSetQword48ToLe (void * pAddr_p, UINT64 qwQwordVal_p);


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword48()
//
// Description: reads a 48 bit value from a buffer
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      UINT64
//
//---------------------------------------------------------------------------

UINT64 AmiGetQword48FromBe (void * pAddr_p);
UINT64 AmiGetQword48FromLe (void * pAddr_p);


//---------------------------------------------------------------------------
//
// Function:    AmiSetQword56()
//
// Description: sets a 56 bit value to a buffer
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:      void
//
//---------------------------------------------------------------------------

void AmiSetQword56ToBe (void * pAddr_p, UINT64 qwQwordVal_p);
void AmiSetQword56ToLe (void * pAddr_p, UINT64 qwQwordVal_p);


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword56()
//
// Description: reads a 56 bit value from a buffer
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      UINT64
//
//---------------------------------------------------------------------------

UINT64 AmiGetQword56FromBe (void * pAddr_p);
UINT64 AmiGetQword56FromLe (void * pAddr_p);


//---------------------------------------------------------------------------
//
// Function:    AmiSetQword64()
//
// Description: sets a 64 bit value to a buffer
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:      void
//
//---------------------------------------------------------------------------

void AmiSetQword64ToBe (void * pAddr_p, UINT64 qwQwordVal_p);
void AmiSetQword64ToLe (void * pAddr_p, UINT64 qwQwordVal_p);


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword64()
//
// Description: reads a 64 bit value from a buffer
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      void
//
//---------------------------------------------------------------------------

UINT64 AmiGetQword64FromBe (void * pAddr_p);
UINT64 AmiGetQword64FromLe (void * pAddr_p);


//---------------------------------------------------------------------------
//
// Function:    AmiSetTimeOfDay()
//
// Description: sets a TIME_OF_DAY (CANopen) value to a buffer
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              pTimeOfDay_p    = pointer to struct TIME_OF_DAY
//
// Return:      void
//
//---------------------------------------------------------------------------

void AmiSetTimeOfDay (void * pAddr_p, tTimeOfDay * pTimeOfDay_p);


//---------------------------------------------------------------------------
//
// Function:    AmiGetTimeOfDay()
//
// Description: reads a TIME_OF_DAY (CANopen) value from a buffer
//
// Parameters:  pAddr_p         = pointer to source buffer
//              pTimeOfDay_p    = pointer to struct TIME_OF_DAY
//
// Return:      void
//
//---------------------------------------------------------------------------

void AmiGetTimeOfDay (void * pAddr_p, tTimeOfDay * pTimeOfDay_p);

#endif  /* _CNAPIAMI_H_ */

/*******************************************************************************
*
* License Agreement
*
* Copyright 2013 BERNECKER + RAINER, AUSTRIA, 5142 EGGELSBERG, B&R STRASSE 1
* All rights reserved.
*
* Redistribution and use in source and binary forms,
* with or without modification,
* are permitted provided that the following conditions are met:
*
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer
*     in the documentation and/or other materials provided with the
*     distribution.
*   * Neither the name of the B&R nor the names of its contributors
*     may be used to endorse or promote products derived from this software
*     without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*******************************************************************************/
/* END-OF-FILE */

