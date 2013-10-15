/**
********************************************************************************
\file   amiLittle.c

\brief  Abstract Memory Interface for ARM processors

\ingroup amiLittle
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
#include <appif/ami.h>

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
   UINT32  m_dwDword;

} tdwStruct;

typedef struct
{
   UINT64  m_qwQword;

} tqwStruct;

typedef union
{
    UINT32   m_dwValue;
    UINT8    m_abValue[4];

} tSplittDword;

typedef union
{
    UINT64   m_qwValue;
    UINT8    m_abValue[8];

} tSplittQword;

#define AMI_LOUINT8(val)                     (UINT8) (val      )
#define AMI_HIUINT8(val)                     (UINT8) (val >>  8)
#define AMI_LOUINT16(val)                     (UINT16) (val      )
#define AMI_HIUINT16(val)                     (UINT16) (val >> 16)

#define AMI_READ_UINT8(val,ptr)              (val) =                          *(UINT8 *) (ptr)
#define AMI_READ_UINT16(val,ptr)              (val) =                (UINT16)  (*((UINT8 *) (ptr) + 1));\
                                            (val) = ((val) << 8) | (UINT16)  (*((UINT8 *) (ptr) + 0));
#define AMI_READ_UINT32(val,ptr)             (val) =                (UINT32) (*((UINT8 *) (ptr) + 3));\
                                            (val) = ((val) << 8) | (UINT32) (*((UINT8 *) (ptr) + 2));\
                                            (val) = ((val) << 8) | (UINT32) (*((UINT8 *) (ptr) + 1));\
                                            (val) = ((val) << 8) | (UINT32) (*((UINT8 *) (ptr) + 0));

#define AMI_WRITE_UINT8(ptr,val)               *(UINT8 *) (ptr)       =                        (val)
#define AMI_WRITE_UINT16(ptr,val)             (*((UINT8 *) (ptr) + 0)) =             AMI_LOUINT8 (val);\
                                            (*((UINT8 *) (ptr) + 1)) =             AMI_HIUINT8 (val);
#define AMI_WRITE_UINT32(ptr,val)            (*((UINT8 *) (ptr) + 0)) = AMI_LOUINT8 (AMI_LOUINT16 (val));\
                                            (*((UINT8 *) (ptr) + 1)) = AMI_HIUINT8 (AMI_LOUINT16 (val));\
                                            (*((UINT8 *) (ptr) + 2)) = AMI_LOUINT8 (AMI_HIUINT16 (val));\
                                            (*((UINT8 *) (ptr) + 3)) = AMI_HIUINT8 (AMI_HIUINT16 (val));


//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//---------------------------------------------------------------------------
//
// Function:    AmiSetXXXToBe()
//
// Description: writes the specified value to the absolute address in
//              big endian
//
// Parameters:  pAddr_p                 = absolute address
//              xXXXVal_p               = value
//
// Returns:     (none)
//
// State:
//
//---------------------------------------------------------------------------

//------------< write UINT8 in big endian >--------------------------
/*
void    AmiSetByteToBe (void * pAddr_p, UINT8 bByteVal_p)
{

   *(UINT8 *)pAddr_p = bByteVal_p;

}
*/



//------------< write UINT16 in big endian >--------------------------

void    AmiSetWordToBe (void * pAddr_p, UINT16 wWordVal_p)
{
    (*((UINT8 *) (pAddr_p) + 0)) = AMI_HIUINT8 (wWordVal_p);
    (*((UINT8 *) (pAddr_p) + 1)) = AMI_LOUINT8 (wWordVal_p);
}



//------------< write UINT32 in big endian >-------------------------

void    AmiSetDwordToBe (void * pAddr_p, UINT32 dwDwordVal_p)
{
    (*((UINT8 *) (pAddr_p) + 0)) = AMI_HIUINT8 (AMI_HIUINT16 (dwDwordVal_p));
    (*((UINT8 *) (pAddr_p) + 1)) = AMI_LOUINT8 (AMI_HIUINT16 (dwDwordVal_p));
    (*((UINT8 *) (pAddr_p) + 2)) = AMI_HIUINT8 (AMI_LOUINT16 (dwDwordVal_p));
    (*((UINT8 *) (pAddr_p) + 3)) = AMI_LOUINT8 (AMI_LOUINT16 (dwDwordVal_p));
}




//---------------------------------------------------------------------------
//
// Function:    AmiSetXXXToLe()
//
// Description: writes the specified value to the absolute address in
//              little endian
//
// Parameters:  pAddr_p                 = absolute address
//              xXXXVal_p               = value
//
// Returns:     (none)
//
// State:
//
//---------------------------------------------------------------------------

//------------< write UINT8 in little endian >--------------------------
/*
void    AmiSetByteToLe (void * pAddr_p, UINT8 bByteVal_p)
{

// Diese Funktion dient zum Schreiben eines Bytes auf der angegebenen
// absoluten Adresse. Die Funktionen der Gruppe <AmiSetxxx> werden verwendet,
// um Daten zum Austausch mit anderen Systemen im Intel-Format abzulegen.


   AMI_WRITE_UINT8 (pAddr_p, bByteVal_p);

}
*/



//------------< write UINT16 in little endian >--------------------------

void    AmiSetWordToLe (void * pAddr_p, UINT16 wWordVal_p)
{
   AMI_WRITE_UINT16 (pAddr_p, wWordVal_p);
}




//------------< write UINT32 in little endian >-------------------------

void    AmiSetDwordToLe (void * pAddr_p, UINT32 dwDwordVal_p)
{
   AMI_WRITE_UINT32 (pAddr_p, dwDwordVal_p);
}


//------------< read UINT16 in big endian >---------------------------

 UINT16    AmiGetWordFromBe (void * pAddr_p)
{
UINT16 wValue;

    (wValue) =                   (UINT16)  (*((UINT8 *) (pAddr_p) + 0));
    (wValue) = ((wValue) << 8) | (UINT16)  (*((UINT8 *) (pAddr_p) + 1));

    return ( wValue );
}




//------------< read UINT32 in big endian >--------------------------

 UINT32    AmiGetDwordFromBe (void * pAddr_p)
{
UINT32 dwValue;

    (dwValue) =                    (UINT32) (*((UINT8 *) (pAddr_p) + 0));\
    (dwValue) = ((dwValue) << 8) | (UINT32) (*((UINT8 *) (pAddr_p) + 1));\
    (dwValue) = ((dwValue) << 8) | (UINT32) (*((UINT8 *) (pAddr_p) + 2));\
    (dwValue) = ((dwValue) << 8) | (UINT32) (*((UINT8 *) (pAddr_p) + 3));

    return ( dwValue );

}

//------------< read UINT8 in little endian >---------------------------
/*
UINT8    AmiGetByteFromLe (void * pAddr_p)
{

// Diese Funktion dient zum Lesen eines Bytes von der angegebenen absoluten
// Adresse. Die Funktionen der Gruppe <AmiGetxxx> werden verwendet, um Daten,
// die mit einem anderen System ausgetauscht wurden und im Intel-Format abge-
// legt sind, zu lesen.


UINT8 bValue;

    AMI_READ_UINT8 (bValue, pAddr_p);

    return ( bValue );

}
*/



//------------< read UINT16 in little endian >---------------------------

 UINT16    AmiGetWordFromLe (void * pAddr_p)
{

// Diese Funktion dient zum Lesen eines Words von der angegebenen absoluten
// Adresse. Die Funktionen der Gruppe <AmiGetxxx> werden verwendet, um Daten,
// die mit einem anderen System ausgetauscht wurden und im Intel-Format abge-
// legt sind, zu lesen.


UINT16 wValue;

    AMI_READ_UINT16 (wValue, pAddr_p);

    return ( wValue );

}




//------------< read UINT32 in little endian >--------------------------

 UINT32    AmiGetDwordFromLe (void * pAddr_p)
{

// Diese Funktion dient zum Lesen eines Dwords von der angegebenen absoluten
// Adresse. Die Funktionen der Gruppe <AmiGetxxx> werden verwendet, um Daten,
// die mit einem anderen System ausgetauscht wurden und im Intel-Format abge-
// legt sind, zu lesen.


UINT32 dwValue;

    AMI_READ_UINT32 (dwValue, pAddr_p);

    return ( dwValue );

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
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetDword24ToBe (void * pAddr_p, UINT32 dwDwordVal_p)
{
tSplittDword dwValue;

    dwValue.m_dwValue = dwDwordVal_p;

    ((UINT8 *) pAddr_p)[0] = dwValue.m_abValue[0];
    ((UINT8 *) pAddr_p)[1] = dwValue.m_abValue[1];
    ((UINT8 *) pAddr_p)[2] = dwValue.m_abValue[2];

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
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetDword24ToLe (void * pAddr_p, UINT32 dwDwordVal_p)
{

    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &dwDwordVal_p)[0];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &dwDwordVal_p)[1];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &dwDwordVal_p)[2];

}


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
// State:       not tested
//
//---------------------------------------------------------------------------

 UINT32  AmiGetDword24FromBe (void * pAddr_p)
{

tdwStruct      dwStruct = { 0 };

    dwStruct.m_dwDword  = AmiGetDwordFromBe (pAddr_p);
    dwStruct.m_dwDword >>= 8;

    return ( dwStruct.m_dwDword );

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

 UINT32  AmiGetDword24FromLe (void * pAddr_p)
{

tdwStruct      dwStruct;

    dwStruct.m_dwDword  = AmiGetDwordFromLe (pAddr_p);
    dwStruct.m_dwDword &= 0x00FFFFFF;

    return ( dwStruct.m_dwDword );

}

//#ifdef USE_VAR64

//---------------------------------------------------------------------------
//
// Function:    AmiSetQword64ToBe()
//
// Description: sets a 64 bit value to a buffer in big endian
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword64ToBe (void * pAddr_p, UINT64 qwQwordVal_p)
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
// Function:    AmiSetQword64ToLe()
//
// Description: sets a 64 bit value to a buffer in little endian
//
// Parameters:  pAddr_p         = pointer to destination buffer
//              qwQwordVal_p    = quadruple word value
//
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword64ToLe (void * pAddr_p, UINT64 qwQwordVal_p)
{
tSplittQword    qwValue;

    qwValue.m_qwValue = qwQwordVal_p;

    ((UINT8*) pAddr_p)[0] = qwValue.m_abValue[0];
    ((UINT8*) pAddr_p)[1] = qwValue.m_abValue[1];
    ((UINT8*) pAddr_p)[2] = qwValue.m_abValue[2];
    ((UINT8*) pAddr_p)[3] = qwValue.m_abValue[3];
    ((UINT8*) pAddr_p)[4] = qwValue.m_abValue[4];
    ((UINT8*) pAddr_p)[5] = qwValue.m_abValue[5];
    ((UINT8*) pAddr_p)[6] = qwValue.m_abValue[6];
    ((UINT8*) pAddr_p)[7] = qwValue.m_abValue[7];

}


//---------------------------------------------------------------------------
//
// Function:    AmiGetQword64FromBe()
//
// Description: reads a 64 bit value from a buffer in big endian
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

UINT64  AmiGetQword64FromBe (void * pAddr_p)
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
// Function:    AmiGetQword64FromLe()
//
// Description: reads a 64 bit value from a buffer in little endian
//
// Parameters:  pAddr_p         = pointer to source buffer
//
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

UINT64  AmiGetQword64FromLe (void * pAddr_p)
{
tSplittQword    qwValue = { 0 };

    qwValue.m_abValue[0] = ((UINT8*) pAddr_p)[0];
    qwValue.m_abValue[1] = ((UINT8*) pAddr_p)[1];
    qwValue.m_abValue[2] = ((UINT8*) pAddr_p)[2];
    qwValue.m_abValue[3] = ((UINT8*) pAddr_p)[3];
    qwValue.m_abValue[4] = ((UINT8*) pAddr_p)[4];
    qwValue.m_abValue[5] = ((UINT8*) pAddr_p)[5];
    qwValue.m_abValue[6] = ((UINT8*) pAddr_p)[6];
    qwValue.m_abValue[7] = ((UINT8*) pAddr_p)[7];

    return (UINT64) qwValue.m_qwValue;
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
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword40ToBe (void * pAddr_p, UINT64 qwQwordVal_p)
{

    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &qwQwordVal_p)[4];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &qwQwordVal_p)[3];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &qwQwordVal_p)[2];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &qwQwordVal_p)[1];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &qwQwordVal_p)[0];

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
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword40ToLe (void * pAddr_p, UINT64 qwQwordVal_p)
{
tSplittQword qwValue = { 0 };

    qwValue.m_qwValue = qwQwordVal_p;

    ((UINT8*) pAddr_p)[0] = qwValue.m_abValue[0];
    ((UINT8*) pAddr_p)[1] = qwValue.m_abValue[1];
    ((UINT8*) pAddr_p)[2] = qwValue.m_abValue[2];
    ((UINT8*) pAddr_p)[3] = qwValue.m_abValue[3];
    ((UINT8*) pAddr_p)[4] = qwValue.m_abValue[4];

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

    return ( qwStruct.m_qwQword );

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

UINT64      qwValue;

    qwValue  = AmiGetQword64FromLe (pAddr_p);
    qwValue &= 0x000000FFFFFFFFFFLL;

    return ( qwValue );

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
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword48ToBe (void * pAddr_p, UINT64 qwQwordVal_p)
{

    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &qwQwordVal_p)[5];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &qwQwordVal_p)[4];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &qwQwordVal_p)[3];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &qwQwordVal_p)[2];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &qwQwordVal_p)[1];
    ((UINT8 *) pAddr_p)[5] = ((UINT8 *) &qwQwordVal_p)[0];

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
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword48ToLe (void * pAddr_p, UINT64 qwQwordVal_p)
{
tSplittQword qwValue = { 0 };

    qwValue.m_qwValue = qwQwordVal_p;

    ((UINT8*) pAddr_p)[0] = qwValue.m_abValue[0];
    ((UINT8*) pAddr_p)[1] = qwValue.m_abValue[1];
    ((UINT8*) pAddr_p)[2] = qwValue.m_abValue[2];
    ((UINT8*) pAddr_p)[3] = qwValue.m_abValue[3];
    ((UINT8*) pAddr_p)[4] = qwValue.m_abValue[4];
    ((UINT8*) pAddr_p)[5] = qwValue.m_abValue[5];

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

    return ( qwStruct.m_qwQword );

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

UINT64      qwValue;

    qwValue  = AmiGetQword64FromLe (pAddr_p);
    qwValue &= 0x0000FFFFFFFFFFFFLL;

    return ( qwValue );

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
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword56ToBe (void * pAddr_p, UINT64 qwQwordVal_p)
{

    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &qwQwordVal_p)[6];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &qwQwordVal_p)[5];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &qwQwordVal_p)[4];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &qwQwordVal_p)[3];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &qwQwordVal_p)[2];
    ((UINT8 *) pAddr_p)[5] = ((UINT8 *) &qwQwordVal_p)[1];
    ((UINT8 *) pAddr_p)[6] = ((UINT8 *) &qwQwordVal_p)[0];

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
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetQword56ToLe (void * pAddr_p, UINT64 qwQwordVal_p)
{
tSplittQword qwValue = { 0 };

    qwValue.m_qwValue = qwQwordVal_p;

    ((UINT8*) pAddr_p)[0] = qwValue.m_abValue[0];
    ((UINT8*) pAddr_p)[1] = qwValue.m_abValue[1];
    ((UINT8*) pAddr_p)[2] = qwValue.m_abValue[2];
    ((UINT8*) pAddr_p)[3] = qwValue.m_abValue[3];
    ((UINT8*) pAddr_p)[4] = qwValue.m_abValue[4];
    ((UINT8*) pAddr_p)[5] = qwValue.m_abValue[5];
    ((UINT8*) pAddr_p)[6] = qwValue.m_abValue[6];

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

UINT64  AmiGetQword56FromBe (void * pAddr_p)
{

tqwStruct      qwStruct = { 0 };

    qwStruct.m_qwQword  = AmiGetQword64FromBe (pAddr_p);
    qwStruct.m_qwQword >>= 8;

    return ( qwStruct.m_qwQword );

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

UINT64  AmiGetQword56FromLe (void * pAddr_p)
{

UINT64      qwValue;

    qwValue  = AmiGetQword64FromLe (pAddr_p);
    qwValue &= 0x00FFFFFFFFFFFFFFLL;

    return ( qwValue );

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
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiSetTimeOfDay (void * pAddr_p, tTimeOfDay * pTimeOfDay_p)
{

    AmiSetDwordToLe (((UINT8 *) pAddr_p),     pTimeOfDay_p->m_dwMs & 0x0FFFFFFF);
    AmiSetWordToLe  (((UINT8 *) pAddr_p) + 4, pTimeOfDay_p->m_wDays);

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
// Return:     void
//
// State:       not tested
//
//---------------------------------------------------------------------------

void  AmiGetTimeOfDay (void * pAddr_p, tTimeOfDay * pTimeOfDay_p)
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

