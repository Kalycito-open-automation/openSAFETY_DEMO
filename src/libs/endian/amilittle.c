/**
********************************************************************************
\file   amilittle.c

\brief  Abstract Memory Interface for little endian processors

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
    UINT64 qWord_m;
} tqwStruct;

typedef union
{
    UINT32 value_m;
    UINT8  abValue_m[4];
} tSplittDword;

typedef union
{
    UINT64 qvalue_m;
    UINT8  abValue_m[8];
} tSplittQword;

#define AMI_LOUINT8(val)             (UINT8) (val)
#define AMI_HIUINT8(val)             (UINT8) (val >>  8)
#define AMI_LOUINT16(val)            (UINT16) (val)
#define AMI_HIUINT16(val)            (UINT16) (val >> 16)

#define AMI_READ_UINT8(val,ptr)      (val) = *(UINT8 *) (ptr)
#define AMI_READ_UINT16(val,ptr)     (val) = (UINT16)  (*((UINT8 *) (ptr) + 1)); \
                                     (val) = ((val) << 8) | (UINT16)  (*((UINT8 *) (ptr) + 0));
#define AMI_READ_UINT32(val,ptr)     (val) = (UINT32) (*((UINT8 *) (ptr) + 3)); \
                                     (val) = ((val) << 8) | (UINT32) (*((UINT8 *) (ptr) + 2)); \
                                     (val) = ((val) << 8) | (UINT32) (*((UINT8 *) (ptr) + 1)); \
                                     (val) = ((val) << 8) | (UINT32) (*((UINT8 *) (ptr) + 0));

#define AMI_WRITE_UINT8(ptr,val)     *(UINT8 *) (ptr) = (val)
#define AMI_WRITE_UINT16(ptr,val)    (*((UINT8 *) (ptr) + 0)) = AMI_LOUINT8(val); \
                                     (*((UINT8 *) (ptr) + 1)) = AMI_HIUINT8(val);
#define AMI_WRITE_UINT32(ptr,val)    (*((UINT8 *) (ptr) + 0)) = AMI_LOUINT8(AMI_LOUINT16 (val)); \
                                     (*((UINT8 *) (ptr) + 1)) = AMI_HIUINT8(AMI_LOUINT16 (val)); \
                                     (*((UINT8 *) (ptr) + 2)) = AMI_LOUINT8(AMI_HIUINT16 (val)); \
                                     (*((UINT8 *) (ptr) + 3)) = AMI_HIUINT8(AMI_HIUINT16 (val));


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
    (*((UINT8 *) (pAddr_p) + 0)) = AMI_HIUINT8(wordVal_p);
    (*((UINT8 *) (pAddr_p) + 1)) = AMI_LOUINT8(wordVal_p);
}

//------------------------------------------------------------------------------
/**
\brief    AmiSetWordToLe

Sets a 16 bit value to a buffer in little endian

\param[in]  pAddr_p         Pointer to the destination buffer
\param[out] wordVal_p       The source value to convert

\ingroup module_ami
*/
//------------------------------------------------------------------------------
void AmiSetWordToLe(void* pAddr_p, UINT16 wordVal_p)
{
   AMI_WRITE_UINT16 (pAddr_p, wordVal_p);
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
    UINT16 value;

    (value) =                  (UINT16) (*((UINT8 *) (pAddr_p) + 0));
    (value) = ((value) << 8) | (UINT16) (*((UINT8 *) (pAddr_p) + 1));

    return value;
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
    UINT16 value;

    AMI_READ_UINT16(value, pAddr_p);

    return value;
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
    (*((UINT8 *) (pAddr_p) + 0)) = AMI_HIUINT8(AMI_HIUINT16(dwordVal_p));
    (*((UINT8 *) (pAddr_p) + 1)) = AMI_LOUINT8(AMI_HIUINT16(dwordVal_p));
    (*((UINT8 *) (pAddr_p) + 2)) = AMI_HIUINT8(AMI_LOUINT16(dwordVal_p));
    (*((UINT8 *) (pAddr_p) + 3)) = AMI_LOUINT8(AMI_LOUINT16(dwordVal_p));
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
   AMI_WRITE_UINT32(pAddr_p, dwordVal_p);
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
    UINT32 value;

    (value) =                    (UINT32) (*((UINT8 *) (pAddr_p) + 0));\
    (value) = ((value) << 8) | (UINT32) (*((UINT8 *) (pAddr_p) + 1));\
    (value) = ((value) << 8) | (UINT32) (*((UINT8 *) (pAddr_p) + 2));\
    (value) = ((value) << 8) | (UINT32) (*((UINT8 *) (pAddr_p) + 3));

    return value;
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
 UINT32 AmiGetDwordFromLe(void* pAddr_p)
{
    UINT32 value;

    AMI_READ_UINT32(value, pAddr_p);

    return value;
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
    ((UINT8 *)pAddr_p)[0] = ((UINT8 *)&qwordVal_p)[7];
    ((UINT8 *)pAddr_p)[1] = ((UINT8 *)&qwordVal_p)[6];
    ((UINT8 *)pAddr_p)[2] = ((UINT8 *)&qwordVal_p)[5];
    ((UINT8 *)pAddr_p)[3] = ((UINT8 *)&qwordVal_p)[4];
    ((UINT8 *)pAddr_p)[4] = ((UINT8 *)&qwordVal_p)[3];
    ((UINT8 *)pAddr_p)[5] = ((UINT8 *)&qwordVal_p)[2];
    ((UINT8 *)pAddr_p)[6] = ((UINT8 *)&qwordVal_p)[1];
    ((UINT8 *)pAddr_p)[7] = ((UINT8 *)&qwordVal_p)[0];
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
    tSplittQword value;

    value.qvalue_m = qwordVal_p;

    ((UINT8*)pAddr_p)[0] = value.abValue_m[0];
    ((UINT8*)pAddr_p)[1] = value.abValue_m[1];
    ((UINT8*)pAddr_p)[2] = value.abValue_m[2];
    ((UINT8*)pAddr_p)[3] = value.abValue_m[3];
    ((UINT8*)pAddr_p)[4] = value.abValue_m[4];
    ((UINT8*)pAddr_p)[5] = value.abValue_m[5];
    ((UINT8*)pAddr_p)[6] = value.abValue_m[6];
    ((UINT8*)pAddr_p)[7] = value.abValue_m[7];
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
UINT64 AmiGetQword64FromBe(void* pAddr_p)
{
    tqwStruct qwStruct = { 0 };

    ((UINT8 *) &qwStruct.qWord_m)[0] = ((UINT8 *) pAddr_p)[7];
    ((UINT8 *) &qwStruct.qWord_m)[1] = ((UINT8 *) pAddr_p)[6];
    ((UINT8 *) &qwStruct.qWord_m)[2] = ((UINT8 *) pAddr_p)[5];
    ((UINT8 *) &qwStruct.qWord_m)[3] = ((UINT8 *) pAddr_p)[4];
    ((UINT8 *) &qwStruct.qWord_m)[4] = ((UINT8 *) pAddr_p)[3];
    ((UINT8 *) &qwStruct.qWord_m)[5] = ((UINT8 *) pAddr_p)[2];
    ((UINT8 *) &qwStruct.qWord_m)[6] = ((UINT8 *) pAddr_p)[1];
    ((UINT8 *) &qwStruct.qWord_m)[7] = ((UINT8 *) pAddr_p)[0];

    return qwStruct.qWord_m;
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
UINT64 AmiGetQword64FromLe(void* pAddr_p)
{
    tSplittQword    qvalue = { 0 };

    qvalue.abValue_m[0] = ((UINT8*) pAddr_p)[0];
    qvalue.abValue_m[1] = ((UINT8*) pAddr_p)[1];
    qvalue.abValue_m[2] = ((UINT8*) pAddr_p)[2];
    qvalue.abValue_m[3] = ((UINT8*) pAddr_p)[3];
    qvalue.abValue_m[4] = ((UINT8*) pAddr_p)[4];
    qvalue.abValue_m[5] = ((UINT8*) pAddr_p)[5];
    qvalue.abValue_m[6] = ((UINT8*) pAddr_p)[6];
    qvalue.abValue_m[7] = ((UINT8*) pAddr_p)[7];

    return (UINT64) qvalue.qvalue_m;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{


/// \}

