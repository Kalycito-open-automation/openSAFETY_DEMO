/**
********************************************************************************
\file   amiBig.c

\brief  Abstract Memory Interface for big endian processors

\ingroup amiBig
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

#include "ami.h"

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
   UINT16  m_wWord;

} twStruct;

typedef struct
{
   UINT32  m_dwDword;

} tdwStruct;

typedef struct
{
   UINT64  m_qwQword;

} tqwStruct;



//=========================================================================//
//                                                                         //
//          P U B L I C   F U N C T I O N S                                //
//                                                                         //
//=========================================================================//

//------------< write UINT16 in little endian >--------------------------

void AmiSetWordToLe (void* pAddr_p, UINT16 wWordVal_p)
{
#if defined (__MICROBLAZE__)
UINT8 *dst=(UINT8*)pAddr_p, *src=(UINT8*)&wWordVal_p;

   *(dst+0) = *(src+1);
   *(dst+1) = *(src+0);
#else
twStruct *  pwStruct;
twStruct wValue;

   wValue.m_wWord   = (UINT16)((wWordVal_p & 0x00FF) << 8); //LSB to MSB
   wValue.m_wWord  |= (UINT16)((wWordVal_p & 0xFF00) >> 8); //MSB to LSB

   pwStruct = (twStruct *)pAddr_p;
   pwStruct->m_wWord = wValue.m_wWord;
#endif
}



//------------< write UINT32 in little endian >-------------------------

void AmiSetDwordToLe (void * pAddr_p, UINT32 dwDwordVal_p)
{
#if defined (__MICROBLAZE__)
UINT8 *dst=(UINT8*)pAddr_p, *src=(UINT8*)&dwDwordVal_p;
    
   *(dst+0) = *(src+3);
   *(dst+1) = *(src+2);
   *(dst+2) = *(src+1);
   *(dst+3) = *(src+0);
#else
tdwStruct *  pdwStruct;
tdwStruct dwValue;


   dwValue.m_dwDword = ((dwDwordVal_p & 0x000000FF)<<24); //LSB to MSB
   dwValue.m_dwDword|= ((dwDwordVal_p & 0x0000FF00)<<8);
   dwValue.m_dwDword|= ((dwDwordVal_p & 0x00FF0000)>>8 );
   dwValue.m_dwDword|= ((dwDwordVal_p & 0xFF000000)>>24); //MSB to LSB

   pdwStruct = (tdwStruct *)pAddr_p;
   pdwStruct->m_dwDword = dwValue.m_dwDword;
#endif

}


//------------< write UINT16 in big endian >--------------------------

void AmiSetWordToBe (void * pAddr_p, UINT16 wWordVal_p)
{
#if defined (__MICROBLAZE__)
UINT8 *pDst = (UINT8 *) pAddr_p, *pSrc = (UINT8 *)&wWordVal_p;
    
   *pDst = *pSrc;
   *(pDst+1) = *(pSrc+1);
#else
twStruct *  pwStruct;

   pwStruct = (twStruct *)pAddr_p;
   pwStruct->m_wWord = wWordVal_p;
#endif

}



//------------< write UINT32 in big endian >-------------------------

void AmiSetDwordToBe (void * pAddr_p, UINT32 dwDwordVal_p)
{
#if defined (__MICROBLAZE__)
UINT8 *dst=(UINT8*)pAddr_p, *src=(UINT8*)&dwDwordVal_p;
    
   *(dst+0) = *(src+0);
   *(dst+1) = *(src+1);
   *(dst+2) = *(src+2);
   *(dst+3) = *(src+3);
#else
tdwStruct *  pdwStruct;

   pdwStruct = (tdwStruct *)pAddr_p;
   pdwStruct->m_dwDword = dwDwordVal_p;
#endif

}

//------------< read UINT16 in little endian >---------------------------

UINT16 AmiGetWordFromLe (void * pAddr_p)
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

   wValue.m_wWord   = (UINT16)((pwStruct->m_wWord & 0x00FF) << 8); //LSB to MSB
   wValue.m_wWord  |= (UINT16)((pwStruct->m_wWord & 0xFF00) >> 8); //MSB to LSB

   return ( wValue.m_wWord );
#endif
}




//------------< read UINT32 in little endian >--------------------------

UINT32  AmiGetDwordFromLe (void * pAddr_p)
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

   dwValue.m_dwDword = ((pdwStruct->m_dwDword & 0x000000FF)<<24); //LSB to MSB
   dwValue.m_dwDword|= ((pdwStruct->m_dwDword & 0x0000FF00)<<8);
   dwValue.m_dwDword|= ((pdwStruct->m_dwDword & 0x00FF0000)>>8 );
   dwValue.m_dwDword|= ((pdwStruct->m_dwDword & 0xFF000000)>>24); //MSB to LSB

   return ( dwValue.m_dwDword );
#endif

}

//------------< read UINT16 in big endian >---------------------------

UINT16 AmiGetWordFromBe (void * pAddr_p)
{
#if defined (__MICROBLAZE__)
UINT8 *pbSrc = pAddr_p;
    
   return ((UINT32)(pbSrc[0]) << 8) | 
          ((UINT32)(pbSrc[1]) << 0);
#else
twStruct *  pwStruct;

   pwStruct = (twStruct *)pAddr_p;
   return ( pwStruct->m_wWord );
#endif

}




//------------< read UINT32 in big endian >--------------------------

UINT32 AmiGetDwordFromBe (void * pAddr_p)
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
   return ( pdwStruct->m_dwDword );
#endif

}


//---------------------------------------------------------------------------
//
// Function:    AmiSetDword24ToLe()
//
// Description: sets a 24 bit value to a buffer in little endian
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              dwDwordVal_p    = value to set
//
// Return:      void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void AmiSetDword24ToLe (void * pAddr_p, UINT32 dwDwordVal_p)
{

    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &dwDwordVal_p)[2];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &dwDwordVal_p)[1];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &dwDwordVal_p)[0];

}


//---------------------------------------------------------------------------
//
// Function:    AmiSetDword24ToBe()
//
// Description: sets a 24 bit value to a buffer in big endian
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              dwDwordVal_p    = value to set
//
// Return:      void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void AmiSetDword24ToBe (void * pAddr_p, UINT32 dwDwordVal_p)
{

    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &dwDwordVal_p)[0];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &dwDwordVal_p)[1];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &dwDwordVal_p)[2];

}


//---------------------------------------------------------------------------
//
// Function:    AmiGetDword24FromLe()
//
// Description: reads a 24 bit value from a buffer in little endian
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      UINT32           = read value
//
// State:       not tested
//
//---------------------------------------------------------------------------

UINT32 AmiGetDword24FromLe (void * pAddr_p)
{

tdwStruct      dwStruct;

    dwStruct.m_dwDword  = AmiGetDwordFromLe (pAddr_p) & 0x00FFFFFF;

    return ( dwStruct.m_dwDword );

}


//---------------------------------------------------------------------------
//
// Function:    AmiGetDword24FromBe()
//
// Description: reads a 24 bit value from a buffer in big endian
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      UINT32           = read value
//
// State:       not tested
//
//---------------------------------------------------------------------------

UINT32 AmiGetDword24FromBe (void * pAddr_p)
{

tdwStruct      dwStruct;

    dwStruct.m_dwDword  = AmiGetDwordFromBe (pAddr_p);
    dwStruct.m_dwDword >>= 8;

    return ( dwStruct.m_dwDword );

}


//---------------------------------------------------------------------------
//
// Function:    AmiSetQword64ToLe()
//
// Description: sets a 64 bit value to a buffer in little endian
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:      void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void AmiSetQword64ToLe (void * pAddr_p, UINT64 qwQwordVal_p)
{

    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &qwQwordVal_p)[7];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &qwQwordVal_p)[6];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &qwQwordVal_p)[5];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &qwQwordVal_p)[4];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &qwQwordVal_p)[3];
    ((UINT8 *) pAddr_p)[5] = ((UINT8 *) &qwQwordVal_p)[2];
    ((UINT8 *) pAddr_p)[6] = ((UINT8 *) &qwQwordVal_p)[1];
    ((UINT8 *) pAddr_p)[7] = ((UINT8 *) &qwQwordVal_p)[0];

}


//---------------------------------------------------------------------------
//
// Function:    AmiSetQword64ToBe()
//
// Description: sets a 64 bit value to a buffer in big endian
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:      void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void AmiSetQword64ToBe (void * pAddr_p, UINT64 qwQwordVal_p)
{
#if defined (__MICROBLAZE__)
UINT8 *dst=(UINT8*)pAddr_p, *src=(UINT8*)&qwQwordVal_p;
    
    *(dst+0) = *(src+0);
    *(dst+1) = *(src+1);
    *(dst+2) = *(src+2);
    *(dst+3) = *(src+3);
    *(dst+4) = *(src+4);
    *(dst+5) = *(src+5);
    *(dst+6) = *(src+6);
    *(dst+7) = *(src+7);
#else
UINT64 * pqwDst;

    pqwDst  = (UINT64 *) pAddr_p;
    *pqwDst = qwQwordVal_p;
#endif

}


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword64FromLe()
//
// Description: reads a 64 bit value from a buffer in little endian
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      void
//
// State:       not tested
//
//---------------------------------------------------------------------------

UINT64 AmiGetQword64FromLe (void * pAddr_p)
{

tqwStruct      qwStruct = { 0 };

    ((UINT8 *) &qwStruct.m_qwQword)[0] = ((UINT8 *) pAddr_p)[7];
    ((UINT8 *) &qwStruct.m_qwQword)[1] = ((UINT8 *) pAddr_p)[6];
    ((UINT8 *) &qwStruct.m_qwQword)[2] = ((UINT8 *) pAddr_p)[5];
    ((UINT8 *) &qwStruct.m_qwQword)[3] = ((UINT8 *) pAddr_p)[4];
    ((UINT8 *) &qwStruct.m_qwQword)[4] = ((UINT8 *) pAddr_p)[3];
    ((UINT8 *) &qwStruct.m_qwQword)[5] = ((UINT8 *) pAddr_p)[2];
    ((UINT8 *) &qwStruct.m_qwQword)[6] = ((UINT8 *) pAddr_p)[1];
    ((UINT8 *) &qwStruct.m_qwQword)[7] = ((UINT8 *) pAddr_p)[0];

    return ( qwStruct.m_qwQword );

}


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword64FromBe()
//
// Description: reads a 64 bit value from a buffer in big endian
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      void
//
// State:       not tested
//
//---------------------------------------------------------------------------

UINT64 AmiGetQword64FromBe (void * pAddr_p)
{
#if defined (__MICROBLAZE__)
tqwStruct      qwStruct = { 0 };

    ((UINT8 *) &qwStruct.m_qwQword)[0] = ((UINT8 *) pAddr_p)[0];
    ((UINT8 *) &qwStruct.m_qwQword)[1] = ((UINT8 *) pAddr_p)[1];
    ((UINT8 *) &qwStruct.m_qwQword)[2] = ((UINT8 *) pAddr_p)[2];
    ((UINT8 *) &qwStruct.m_qwQword)[3] = ((UINT8 *) pAddr_p)[3];
    ((UINT8 *) &qwStruct.m_qwQword)[4] = ((UINT8 *) pAddr_p)[4];
    ((UINT8 *) &qwStruct.m_qwQword)[5] = ((UINT8 *) pAddr_p)[5];
    ((UINT8 *) &qwStruct.m_qwQword)[6] = ((UINT8 *) pAddr_p)[6];
    ((UINT8 *) &qwStruct.m_qwQword)[7] = ((UINT8 *) pAddr_p)[7];

    return ( qwStruct.m_qwQword );
#else
tqwStruct * pqwStruct;
tqwStruct      qwStruct;

    pqwStruct = (tqwStruct *) pAddr_p;
    qwStruct.m_qwQword = pqwStruct->m_qwQword;

    return ( qwStruct.m_qwQword );
#endif
}


//---------------------------------------------------------------------------
//
// Function:    AmiSetQword40ToLe()
//
// Description: sets a 40 bit value to a buffer in little endian
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:      void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void AmiSetQword40ToLe (void * pAddr_p, UINT64 qwQwordVal_p)
{
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &qwQwordVal_p)[7];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &qwQwordVal_p)[6];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &qwQwordVal_p)[5];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &qwQwordVal_p)[4];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &qwQwordVal_p)[3];
}


//---------------------------------------------------------------------------
//
// Function:    AmiSetQword40ToBe()
//
// Description: sets a 40 bit value to a buffer in big endian
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:      void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword40ToBe (void * pAddr_p, UINT64 qwQwordVal_p)
{
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &qwQwordVal_p)[3];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &qwQwordVal_p)[4];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &qwQwordVal_p)[5];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &qwQwordVal_p)[6];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &qwQwordVal_p)[7];
}


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword40FromLe()
//
// Description: reads a 40 bit value from a buffer in little endian
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      UINT64
//
// State:       not tested
//
//---------------------------------------------------------------------------

UINT64  AmiGetQword40FromLe (void * pAddr_p)
{
tqwStruct      qwStruct = { 0 };

    qwStruct.m_qwQword  = AmiGetQword64FromLe (pAddr_p);
    qwStruct.m_qwQword &= 0x000000FFFFFFFFFFLL;

    return ( qwStruct.m_qwQword );
}


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword40FromBe()
//
// Description: reads a 40 bit value from a buffer in big endian
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      UINT64
//
// State:       not tested
//
//---------------------------------------------------------------------------

UINT64  AmiGetQword40FromBe (void * pAddr_p)
{
tqwStruct      qwStruct = { 0 };

    qwStruct.m_qwQword  = AmiGetQword64FromBe (pAddr_p);
    qwStruct.m_qwQword >>= 24;
#if defined (__MICROBLAZE__)
    qwStruct.m_qwQword &= 0x000000FFFFFFFFFFLL;
#endif
    return ( qwStruct.m_qwQword );
}


//---------------------------------------------------------------------------
//
// Function:    AmiSetQword48ToLe()
//
// Description: sets a 48 bit value to a buffer in little endian
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:      void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword48ToLe (void * pAddr_p, UINT64 qwQwordVal_p)
{
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &qwQwordVal_p)[7];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &qwQwordVal_p)[6];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &qwQwordVal_p)[5];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &qwQwordVal_p)[4];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &qwQwordVal_p)[3];
    ((UINT8 *) pAddr_p)[5] = ((UINT8 *) &qwQwordVal_p)[2];
}


//---------------------------------------------------------------------------
//
// Function:    AmiSetQword48ToBe()
//
// Description: sets a 48 bit value to a buffer in big endian
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:      void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword48ToBe (void * pAddr_p, UINT64 qwQwordVal_p)
{
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &qwQwordVal_p)[2];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &qwQwordVal_p)[3];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &qwQwordVal_p)[4];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &qwQwordVal_p)[5];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &qwQwordVal_p)[6];
    ((UINT8 *) pAddr_p)[5] = ((UINT8 *) &qwQwordVal_p)[7];
}


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword48FromLe()
//
// Description: reads a 48 bit value from a buffer in little endian
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      UINT64
//
// State:       not tested
//
//---------------------------------------------------------------------------

UINT64  AmiGetQword48FromLe (void * pAddr_p)
{
tqwStruct      qwStruct = { 0 };

    qwStruct.m_qwQword  = AmiGetQword64FromLe (pAddr_p);
    qwStruct.m_qwQword &= 0x0000FFFFFFFFFFFFLL;

    return ( qwStruct.m_qwQword );
}


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword48FromBe()
//
// Description: reads a 48 bit value from a buffer in big endian
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      UINT64
//
// State:       not tested
//
//---------------------------------------------------------------------------

UINT64  AmiGetQword48FromBe (void * pAddr_p)
{
tqwStruct      qwStruct = { 0 };

    qwStruct.m_qwQword  = AmiGetQword64FromBe (pAddr_p);
    qwStruct.m_qwQword >>= 16;
#if defined (__MICROBLAZE__)
    qwStruct.m_qwQword &= 0x0000FFFFFFFFFFFFLL;
#endif
    return ( qwStruct.m_qwQword );
}


//---------------------------------------------------------------------------
//
// Function:    AmiSetQword56ToLe()
//
// Description: sets a 56 bit value to a buffer in little endian
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:      void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword56ToLe (void * pAddr_p, UINT64 qwQwordVal_p)
{
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &qwQwordVal_p)[7];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &qwQwordVal_p)[6];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &qwQwordVal_p)[5];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &qwQwordVal_p)[4];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &qwQwordVal_p)[3];
    ((UINT8 *) pAddr_p)[5] = ((UINT8 *) &qwQwordVal_p)[2];
    ((UINT8 *) pAddr_p)[6] = ((UINT8 *) &qwQwordVal_p)[1];
}


//---------------------------------------------------------------------------
//
// Function:    AmiSetQword56ToBe()
//
// Description: sets a 56 bit value to a buffer in big endian
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:      void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword56ToBe (void * pAddr_p, UINT64 qwQwordVal_p)
{
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &qwQwordVal_p)[1];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &qwQwordVal_p)[2];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &qwQwordVal_p)[3];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &qwQwordVal_p)[4];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &qwQwordVal_p)[5];
    ((UINT8 *) pAddr_p)[5] = ((UINT8 *) &qwQwordVal_p)[6];
    ((UINT8 *) pAddr_p)[6] = ((UINT8 *) &qwQwordVal_p)[7];
}


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword56FromLe()
//
// Description: reads a 56 bit value from a buffer in little endian
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      UINT64
//
// State:       not tested
//
//---------------------------------------------------------------------------

UINT64  AmiGetQword56FromLe(void * pAddr_p)
{
tqwStruct      qwStruct = { 0 };

    qwStruct.m_qwQword  = AmiGetQword64FromLe (pAddr_p);
    qwStruct.m_qwQword &= 0x00FFFFFFFFFFFFFFLL;

    return ( qwStruct.m_qwQword );
}


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword56FromBe()
//
// Description: reads a 56 bit value from a buffer in big endian
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:      UINT64
//
// State:       not tested
//
//---------------------------------------------------------------------------

UINT64 AmiGetQword56FromBe(void * pAddr_p)
{
tqwStruct      qwStruct = { 0 };

    qwStruct.m_qwQword  = AmiGetQword64FromBe (pAddr_p);
    qwStruct.m_qwQword >>= 8;
#if defined (__MICROBLAZE__)
    qwStruct.m_qwQword &= 0x00FFFFFFFFFFFFFFLL;
#endif

    return ( qwStruct.m_qwQword );
}


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
// State:       not tested
//
//---------------------------------------------------------------------------

void AmiSetTimeOfDay(void * pAddr_p, tTimeOfDay * pTimeOfDay_p)
{
    AmiSetDwordToLe(((UINT8 *) pAddr_p),     pTimeOfDay_p->m_dwMs & 0x0FFFFFFF);
    AmiSetWordToLe(((UINT8 *) pAddr_p) + 4, pTimeOfDay_p->m_wDays);
}


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
// State:       not tested
//
//---------------------------------------------------------------------------

void AmiGetTimeOfDay (void * pAddr_p, tTimeOfDay * pTimeOfDay_p)
{
    pTimeOfDay_p->m_dwMs  = AmiGetDwordFromLe (((UINT8 *) pAddr_p)) & 0x0FFFFFFF;
    pTimeOfDay_p->m_wDays = AmiGetWordFromLe  (((UINT8 *) pAddr_p) + 4);
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{


/// \}



