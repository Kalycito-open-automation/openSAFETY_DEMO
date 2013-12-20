/**
********************************************************************************
\file   amibig.c

\brief  Abstract Memory Interface for big endian processors

\ingroup module_ami
*******************************************************************************/

/*------------------------------------------------------------------------------
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
------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include <ami/ami.h>

//============================================================================//
//            G L O B A L   D E F I N I T I O N S                             //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// module global vars
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// global function prototypes
//------------------------------------------------------------------------------


//============================================================================//
//            P R I V A T E   D E F I N I T I O N S                           //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------

typedef struct
{
   UINT16  word_m;
} twStruct;


typedef struct
{
   UINT32  dword_m;
} tdwStruct;


typedef struct
{
   UINT64  qword_m;
} tqwStruct;

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    AmiSetWordToBe

Sets a 16 bit value to a buffer in big endian

\param[out] pAddr_p         Pointer to the destination buffer
\param[in]  wordVal_p       The source value to convert

\ingroup module_ami
*/
//------------------------------------------------------------------------------
void AmiSetWordToBe(void* pAddr_p, UINT16 wordVal_p)
{
#if defined (__MICROBLAZE__)
    UINT8 *pDst = (UINT8 *) pAddr_p;
    UINT8 *pSrc = (UINT8 *)&wordVal_p;

    *pDst = *pSrc;
    *(pDst+1) = *(pSrc+1);
#else
    twStruct*  pwStruct;

    pwStruct = (twStruct *)pAddr_p;
    pwStruct->word_m = wordVal_p;
#endif
}

//------------------------------------------------------------------------------
/**
\brief    AmiSetWordToLe

Sets a 16 bit value to a buffer in little endian

\param[out] pAddr_p         Pointer to the destination buffer
\param[in]  wordVal_p       The source value to convert

\ingroup module_ami
*/
//------------------------------------------------------------------------------
void AmiSetWordToLe(void* pAddr_p, UINT16 wordVal_p)
{
#if defined (__MICROBLAZE__)
    UINT8* dst = (UINT8*)pAddr_p;
    UINT8* src = (UINT8*)&wordVal_p;

    *(dst+0) = *(src+1);
    *(dst+1) = *(src+0);
#else
    twStruct *  pwStruct;
    twStruct wValue;

    wValue.word_m   = (UINT16)((wordVal_p & 0x00FF) << 8); //LSB to MSB
    wValue.word_m  |= (UINT16)((wordVal_p & 0xFF00) >> 8); //MSB to LSB

    pwStruct = (twStruct *)pAddr_p;
    pwStruct->word_m = wValue.word_m;
#endif
}

//------------------------------------------------------------------------------
/**
\brief    AmiGetWordFromBe

Reads a 16 bit value from a buffer in big endian

\param[in]  pAddr_p         Pointer to the source buffer

\return UINT16
\retval Value       The data in platform endian

\ingroup module_ami
*/
//------------------------------------------------------------------------------
UINT16 AmiGetWordFromBe(void* pAddr_p)
{
#if defined (__MICROBLAZE__)
    UINT8 *pbSrc = pAddr_p;

    return ((UINT16)(pbSrc[0]) << 8) |
          ((UINT16)(pbSrc[1]) << 0);
#else
    twStruct*  pwStruct;

    pwStruct = (twStruct *)pAddr_p;

    return ( pwStruct->word_m );
#endif
}

//------------------------------------------------------------------------------
/**
\brief    AmiGetWordFromLe

Reads a 16 bit value from a buffer in little endian

\param[in]  pAddr_p         Pointer to the source buffer

\return UINT16
\retval Value       The data in platform endian

\ingroup module_ami
*/
//------------------------------------------------------------------------------
UINT16 AmiGetWordFromLe(void* pAddr_p)
{
#if defined (__MICROBLAZE__)
    UINT8 *pSrc = pAddr_p;
    UINT16 wDst;

    wDst = (*(pSrc+1) << 8) | (*(pSrc+0));

    return wDst;
#else
    twStruct *  pwStruct;
    twStruct wValue;

    pwStruct = (twStruct *)pAddr_p;

    wValue.word_m   = (UINT16)((pwStruct->word_m & 0x00FF) << 8); //LSB to MSB
    wValue.word_m  |= (UINT16)((pwStruct->word_m & 0xFF00) >> 8); //MSB to LSB

    return wValue.word_m;
#endif
}

//------------------------------------------------------------------------------
/**
\brief    AmiSetDwordToBe

Sets a 32 bit value to a buffer in big endian

\param[out] pAddr_p         Pointer to the destination buffer
\param[in]  dwordVal_p      The source value to convert

\ingroup module_ami
*/
//------------------------------------------------------------------------------
void AmiSetDwordToBe(void* pAddr_p, UINT32 dwordVal_p)
{
#if defined (__MICROBLAZE__)
    UINT8* dst = (UINT8*)pAddr_p;
    UINT8* src = (UINT8*)&dwordVal_p;

    *(dst+0) = *(src+0);
    *(dst+1) = *(src+1);
    *(dst+2) = *(src+2);
    *(dst+3) = *(src+3);
#else
    tdwStruct *  pdwStruct;

    pdwStruct = (tdwStruct *)pAddr_p;
    pdwStruct->dword_m = dwordVal_p;
#endif
}

//------------------------------------------------------------------------------
/**
\brief    AmiSetDwordToLe

Sets a 32 bit value to a buffer in little endian

\param[out] pAddr_p         Pointer to the destination buffer
\param[in]  dwordVal_p      The source value to convert

\ingroup module_ami
*/
//------------------------------------------------------------------------------
void AmiSetDwordToLe(void* pAddr_p, UINT32 dwordVal_p)
{
#if defined (__MICROBLAZE__)
    UINT8* dst = (UINT8*)pAddr_p;
    UINT8* src = (UINT8*)&dwordVal_p;

   *(dst+0) = *(src+3);
   *(dst+1) = *(src+2);
   *(dst+2) = *(src+1);
   *(dst+3) = *(src+0);
#else
    tdwStruct *  pdwStruct;
    tdwStruct dwValue;


   dwValue.dword_m = ((dwordVal_p & 0x000000FF)<<24); //LSB to MSB
   dwValue.dword_m |= ((dwordVal_p & 0x0000FF00)<<8);
   dwValue.dword_m |= ((dwordVal_p & 0x00FF0000)>>8 );
   dwValue.dword_m |= ((dwordVal_p & 0xFF000000)>>24); //MSB to LSB

   pdwStruct = (tdwStruct *)pAddr_p;
   pdwStruct->dword_m = dwValue.dword_m;
#endif
}

//------------------------------------------------------------------------------
/**
\brief    AmiGetDwordFromBe

Reads a 32 bit value from a buffer in big endian

\param[in]  pAddr_p         Pointer to the source buffer

\return UINT32
\retval Value       The data in platform endian

\ingroup module_ami
*/
//------------------------------------------------------------------------------
UINT32 AmiGetDwordFromBe(void* pAddr_p)
{
#if defined (__MICROBLAZE__)
    UINT8 *pbSrc = pAddr_p;

    return (((UINT32)pbSrc[0]) << 24) |
           (((UINT32)pbSrc[1]) << 16) |
           (((UINT32)pbSrc[2]) <<  8) |
           (((UINT32)pbSrc[3]) <<  0);
#else
    tdwStruct *  pdwStruct;

    pdwStruct = (tdwStruct *)pAddr_p;

    return ( pdwStruct->dword_m );
#endif
}

//------------------------------------------------------------------------------
/**
\brief    AmiGetDwordFromLe

Reads a 32 bit value from a buffer in little endian

\param[in]  pAddr_p         Pointer to the source buffer

\return UINT32
\retval Value       The data in platform endian

\ingroup module_ami
*/
//------------------------------------------------------------------------------
UINT32  AmiGetDwordFromLe(void* pAddr_p)
{
#if defined (__MICROBLAZE__)
    UINT8 *pbSrc = pAddr_p;

    return (((UINT32)pbSrc[0]) <<  0) |
           (((UINT32)pbSrc[1]) <<  8) |
           (((UINT32)pbSrc[2]) << 16) |
           (((UINT32)pbSrc[3]) << 24);
#else
    tdwStruct *  pdwStruct;
    tdwStruct dwValue;

    pdwStruct = (tdwStruct *)pAddr_p;

    dwValue.dword_m = ((pdwStruct->dword_m & 0x000000FF)<<24); //LSB to MSB
    dwValue.dword_m|= ((pdwStruct->dword_m & 0x0000FF00)<<8);
    dwValue.dword_m|= ((pdwStruct->dword_m & 0x00FF0000)>>8 );
    dwValue.dword_m|= ((pdwStruct->dword_m & 0xFF000000)>>24); //MSB to LSB

    return dwValue.dword_m;
#endif
}

//------------------------------------------------------------------------------
/**
\brief    AmiSetQword64ToBe

Sets a 64 bit value to a buffer in big endian

\param[out] pAddr_p         Pointer to the destination buffer
\param[in]  qwordVal_p      The source value to convert

\ingroup module_ami
*/
//------------------------------------------------------------------------------
void AmiSetQword64ToBe(void* pAddr_p, UINT64 qwordVal_p)
{
#if defined (__MICROBLAZE__)
    UINT8* dst = (UINT8*)pAddr_p;
    UINT8* src = (UINT8*)&qwordVal_p;

    *(dst+0) = *(src+0);
    *(dst+1) = *(src+1);
    *(dst+2) = *(src+2);
    *(dst+3) = *(src+3);
    *(dst+4) = *(src+4);
    *(dst+5) = *(src+5);
    *(dst+6) = *(src+6);
    *(dst+7) = *(src+7);
#else
    UINT64* pqwDst;

    pqwDst = (UINT64 *) pAddr_p;
    *pqwDst = qwordVal_p;
#endif
}

//------------------------------------------------------------------------------
/**
\brief    AmiSetQword64ToLe

Sets a 64 bit value to a buffer in little endian

\param[out] pAddr_p         Pointer to the destination buffer
\param[in]  qwordVal_p      The source value to convert

\ingroup module_ami
*/
//------------------------------------------------------------------------------
void AmiSetQword64ToLe(void* pAddr_p, UINT64 qwordVal_p)
{
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &qwordVal_p)[7];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &qwordVal_p)[6];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &qwordVal_p)[5];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &qwordVal_p)[4];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &qwordVal_p)[3];
    ((UINT8 *) pAddr_p)[5] = ((UINT8 *) &qwordVal_p)[2];
    ((UINT8 *) pAddr_p)[6] = ((UINT8 *) &qwordVal_p)[1];
    ((UINT8 *) pAddr_p)[7] = ((UINT8 *) &qwordVal_p)[0];
}

//------------------------------------------------------------------------------
/**
\brief    AmiGetQword64FromBe

Reads a 64 bit value from a buffer in big endian

\param[in]  pAddr_p         Pointer to the source buffer

\return UINT64
\retval Value       The data in platform endian

\ingroup module_ami
*/
//------------------------------------------------------------------------------
UINT64 AmiGetQword64FromBe(void * pAddr_p)
{
#if defined (__MICROBLAZE__)
    tqwStruct qwStruct = { 0 };

    ((UINT8 *) &qwStruct.qword_m)[0] = ((UINT8 *) pAddr_p)[0];
    ((UINT8 *) &qwStruct.qword_m)[1] = ((UINT8 *) pAddr_p)[1];
    ((UINT8 *) &qwStruct.qword_m)[2] = ((UINT8 *) pAddr_p)[2];
    ((UINT8 *) &qwStruct.qword_m)[3] = ((UINT8 *) pAddr_p)[3];
    ((UINT8 *) &qwStruct.qword_m)[4] = ((UINT8 *) pAddr_p)[4];
    ((UINT8 *) &qwStruct.qword_m)[5] = ((UINT8 *) pAddr_p)[5];
    ((UINT8 *) &qwStruct.qword_m)[6] = ((UINT8 *) pAddr_p)[6];
    ((UINT8 *) &qwStruct.qword_m)[7] = ((UINT8 *) pAddr_p)[7];

    return ( qwStruct.qword_m );
#else
    tqwStruct* pqwStruct;
    tqwStruct qwStruct;

    pqwStruct = (tqwStruct *) pAddr_p;
    qwStruct.qword_m = pqwStruct->qword_m;

    return ( qwStruct.qword_m );
#endif
}

//------------------------------------------------------------------------------
/**
\brief    AmiGetQword64FromLe

Reads a 64 bit value from a buffer in little endian

\param[in]  pAddr_p         Pointer to the source buffer

\return UINT64
\retval Value       The data in platform endian

\ingroup module_ami
*/
//------------------------------------------------------------------------------
UINT64 AmiGetQword64FromLe(void * pAddr_p)
{
    tqwStruct qwStruct = { 0 };

    ((UINT8 *) &qwStruct.qword_m)[0] = ((UINT8 *) pAddr_p)[7];
    ((UINT8 *) &qwStruct.qword_m)[1] = ((UINT8 *) pAddr_p)[6];
    ((UINT8 *) &qwStruct.qword_m)[2] = ((UINT8 *) pAddr_p)[5];
    ((UINT8 *) &qwStruct.qword_m)[3] = ((UINT8 *) pAddr_p)[4];
    ((UINT8 *) &qwStruct.qword_m)[4] = ((UINT8 *) pAddr_p)[3];
    ((UINT8 *) &qwStruct.qword_m)[5] = ((UINT8 *) pAddr_p)[2];
    ((UINT8 *) &qwStruct.qword_m)[6] = ((UINT8 *) pAddr_p)[1];
    ((UINT8 *) &qwStruct.qword_m)[7] = ((UINT8 *) pAddr_p)[0];

    return qwStruct.qword_m;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{


/// \}

