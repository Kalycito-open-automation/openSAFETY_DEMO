/**
********************************************************************************
\file   amile.c

\defgroup module_psicom_amile Endian module (Little endian)
\{

\brief  Generic implementation of the Abstract Memory Interface (little endian)

This file implements the AMI interface in little endian for architectures
where access to unaligned addresses is not possible. (This implementation
always copies bytewise)

\ingroup group_libpsicommon
*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2014, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
Copyright (c) 2016, Kalycito Infotech Private Ltd
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

/*----------------------------------------------------------------------------*/
/* includes                                                                   */
/*----------------------------------------------------------------------------*/
#include <libpsicommon/ami.h>

/*============================================================================*/
/*            G L O B A L   D E F I N I T I O N S                             */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* module global vars                                                         */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* global function prototypes                                                 */
/*----------------------------------------------------------------------------*/


/*============================================================================*/
/*            P R I V A T E   D E F I N I T I O N S                           */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local types                                                                */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief    Set Uint16 to big endian

Sets a 16 bit value to a buffer in big endian

\param[out] pAddr_p         Pointer to the destination buffer
\param[in]  uint16Val_p       The source value to convert
*/
/*----------------------------------------------------------------------------*/
void ami_setUint16Be(void* pAddr_p, UINT16 uint16Val_p)
{
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &uint16Val_p)[0];
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &uint16Val_p)[1];
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set Uint16 to little endian

Sets a 16 bit value to a buffer in little endian

\param[in]  pAddr_p         Pointer to the destination buffer
\param[out] uint16Val_p     The source value to convert
*/
/*----------------------------------------------------------------------------*/
void ami_setUint16Le(void* pAddr_p, UINT16 uint16Val_p)
{
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &uint16Val_p)[0];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &uint16Val_p)[1];
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get Uint16 from big endian

Reads a 16 bit value from a buffer in big endian

\param[in]  pAddr_p         Pointer to the source buffer

\return UINT16
\retval Value       The data in platform endian
*/
/*----------------------------------------------------------------------------*/
UINT16 ami_getUint16Be(const void* pAddr_p)
{
    UINT16 val;

    ((UINT8 *) &val)[1] = ((UINT8 *) pAddr_p)[0];
    ((UINT8 *) &val)[0] = ((UINT8 *) pAddr_p)[1];

    return val;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get Uint16 from little endian

Reads a 16 bit value from a buffer in little endian

\param[in]  pAddr_p         Pointer to the source buffer

\return UINT16
\retval Value       The data in platform endian
*/
/*----------------------------------------------------------------------------*/
UINT16 ami_getUint16Le(const void* pAddr_p)
{
    UINT16 val;

    ((UINT8 *) &val)[0] = ((UINT8 *) pAddr_p)[0];
    ((UINT8 *) &val)[1] = ((UINT8 *) pAddr_p)[1];

    return val;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set Uint32 to big endian

Sets a 32 bit value to a buffer in big endian

\param[out] pAddr_p         Pointer to the destination buffer
\param[in]  uint32Val_p     The source value to convert
*/
/*----------------------------------------------------------------------------*/
void ami_setUint32Be(void* pAddr_p, UINT32 uint32Val_p)
{
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &uint32Val_p)[0];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &uint32Val_p)[1];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &uint32Val_p)[2];
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &uint32Val_p)[3];
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set Uint32 to little endian

Sets a 32 bit value to a buffer in little endian

\param[out] pAddr_p         Pointer to the destination buffer
\param[in]  uint32Val_p     The source value to convert
*/
/*----------------------------------------------------------------------------*/
void ami_setUint32Le(void* pAddr_p, UINT32 uint32Val_p)
{
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &uint32Val_p)[0];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &uint32Val_p)[1];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &uint32Val_p)[2];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &uint32Val_p)[3];
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get Uint32 from big endian

Reads a 32 bit value from a buffer in big endian

\param[in]  pAddr_p         Pointer to the source buffer

\return UINT32
\retval Value       The data in platform endian
*/
/*----------------------------------------------------------------------------*/
UINT32 ami_getUint32Be(const void* pAddr_p)
{
    UINT32 val;

    ((UINT8 *) &val)[3] = ((UINT8 *) pAddr_p)[0];
    ((UINT8 *) &val)[2] = ((UINT8 *) pAddr_p)[1];
    ((UINT8 *) &val)[1] = ((UINT8 *) pAddr_p)[2];
    ((UINT8 *) &val)[0] = ((UINT8 *) pAddr_p)[3];

    return val;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get Uint32 from little endian

Reads a 32 bit value from a buffer in little endian

\param[in]  pAddr_p         Pointer to the source buffer

\return UINT32
\retval Value       The data in platform endian
*/
/*----------------------------------------------------------------------------*/
UINT32 ami_getUint32Le(const void* pAddr_p)
{
    UINT32 val;

    ((UINT8 *) &val)[0] = ((UINT8 *) pAddr_p)[0];
    ((UINT8 *) &val)[1] = ((UINT8 *) pAddr_p)[1];
    ((UINT8 *) &val)[2] = ((UINT8 *) pAddr_p)[2];
    ((UINT8 *) &val)[3] = ((UINT8 *) pAddr_p)[3];

    return val;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set Uint64 to big endian

Sets a 64 bit value to a buffer in big endian

\param[out] pAddr_p         Pointer to the destination buffer
\param[in]  uint64Val_p     The source value to convert
*/
/*----------------------------------------------------------------------------*/
void ami_setUint64Be(void* pAddr_p, UINT64 uint64Val_p)
{
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &uint64Val_p)[7];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &uint64Val_p)[6];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &uint64Val_p)[5];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &uint64Val_p)[4];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &uint64Val_p)[3];
    ((UINT8 *) pAddr_p)[5] = ((UINT8 *) &uint64Val_p)[2];
    ((UINT8 *) pAddr_p)[6] = ((UINT8 *) &uint64Val_p)[1];
    ((UINT8 *) pAddr_p)[7] = ((UINT8 *) &uint64Val_p)[0];
}

/*----------------------------------------------------------------------------*/
/**
\brief    Set Uint64 to little endian

Sets a 64 bit value to a buffer in little endian

\param[out] pAddr_p         Pointer to the destination buffer
\param[in]  uint64Val_p     The source value to convert
*/
/*----------------------------------------------------------------------------*/
void ami_setUint64Le(void* pAddr_p, UINT64 uint64Val_p)
{
    ((UINT8 *) pAddr_p)[0] = ((UINT8 *) &uint64Val_p)[0];
    ((UINT8 *) pAddr_p)[1] = ((UINT8 *) &uint64Val_p)[1];
    ((UINT8 *) pAddr_p)[2] = ((UINT8 *) &uint64Val_p)[2];
    ((UINT8 *) pAddr_p)[3] = ((UINT8 *) &uint64Val_p)[3];
    ((UINT8 *) pAddr_p)[4] = ((UINT8 *) &uint64Val_p)[4];
    ((UINT8 *) pAddr_p)[5] = ((UINT8 *) &uint64Val_p)[5];
    ((UINT8 *) pAddr_p)[6] = ((UINT8 *) &uint64Val_p)[6];
    ((UINT8 *) pAddr_p)[7] = ((UINT8 *) &uint64Val_p)[7];
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get Uint64 from big endian

Reads a 64 bit value from a buffer in big endian

\param[in]  pAddr_p         Pointer to the source buffer

\return UINT64
\retval Value       The data in platform endian
*/
/*----------------------------------------------------------------------------*/
UINT64 ami_getUint64Be(const void* pAddr_p)
{
    UINT64 val;

    ((UINT8 *) &val)[0] = ((UINT8 *) pAddr_p)[7];
    ((UINT8 *) &val)[1] = ((UINT8 *) pAddr_p)[6];
    ((UINT8 *) &val)[2] = ((UINT8 *) pAddr_p)[5];
    ((UINT8 *) &val)[3] = ((UINT8 *) pAddr_p)[4];
    ((UINT8 *) &val)[4] = ((UINT8 *) pAddr_p)[3];
    ((UINT8 *) &val)[5] = ((UINT8 *) pAddr_p)[2];
    ((UINT8 *) &val)[6] = ((UINT8 *) pAddr_p)[1];
    ((UINT8 *) &val)[7] = ((UINT8 *) pAddr_p)[0];

    return val;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Get Uint64 from little endian

Reads a 64 bit value from a buffer in little endian

\param[in]  pAddr_p         Pointer to the source buffer

\return UINT64
\retval Value       The data in platform endian
*/
/*----------------------------------------------------------------------------*/
UINT64 ami_getUint64Le(const void* pAddr_p)
{
    UINT64 val;

    ((UINT8 *) &val)[0] = ((UINT8 *) pAddr_p)[0];
    ((UINT8 *) &val)[1] = ((UINT8 *) pAddr_p)[1];
    ((UINT8 *) &val)[2] = ((UINT8 *) pAddr_p)[2];
    ((UINT8 *) &val)[3] = ((UINT8 *) pAddr_p)[3];
    ((UINT8 *) &val)[4] = ((UINT8 *) pAddr_p)[4];
    ((UINT8 *) &val)[5] = ((UINT8 *) pAddr_p)[5];
    ((UINT8 *) &val)[6] = ((UINT8 *) pAddr_p)[6];
    ((UINT8 *) &val)[7] = ((UINT8 *) pAddr_p)[7];

    return val;
}

/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/** \name Private Functions */
/** \{ */

/**
 * \}
 * \}
 */
