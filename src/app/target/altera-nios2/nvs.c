/**
********************************************************************************
\file   nvs.c

\brief  Target specific functions to access the non volatile storage.

This module implements the hardware near target specific functions of the
flash memory for Altera Nios2.

\ingroup module_nvs
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
#include <apptarget/nvs.h>

#include <system.h>
#include "sys/alt_flash.h"

//============================================================================//
//            G L O B A L   D E F I N I T I O N S                             //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------
#define FLASH_DEV_NAME    CFI_FLASH_NAME

#define FLASH_BASE        CFI_FLASH_BASE

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

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------
static alt_flash_fd* pFlashDesc_l = (alt_flash_fd*)0;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static INT16 getBlockByOffset(UINT32 offset_p);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief    Initialize the non volatile storage

\return 0 on success; 1 on error

\ingroup module_nvs
*/
//------------------------------------------------------------------------------
UINT8 nvs_init(void)
{
    UINT8 ret = 1;

    pFlashDesc_l = alt_flash_open_dev(FLASH_DEV_NAME);
    if(pFlashDesc_l != (alt_flash_fd*)0)
    {
        ret = 0;
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Close the non volatile storage

\ingroup module_nvs
*/
//------------------------------------------------------------------------------
void nvs_close(void)
{
    /* Close the open flash device */
    alt_flash_close_dev(pFlashDesc_l);
}

//------------------------------------------------------------------------------
/**
\brief    Write data to the non volatile storage

\param offset_p  The offset of the data in the storage
\param pData_p   Pointer to the data to write
\param length_p  The length of the data to write

\return 0 on success; 1 on error

\ingroup module_nvs
*/
//------------------------------------------------------------------------------
UINT8 nvs_write(UINT32 offset_p, UINT8 * pData_p, UINT32 length_p)
{
    UINT8 ret = 1;
    int blockOffset;

    if(pData_p != (UINT8*)0 && length_p > 0)
    {
        blockOffset = getBlockByOffset(offset_p);

        if(alt_write_flash_block(pFlashDesc_l, blockOffset, offset_p, pData_p, length_p) == 0)
        {
            ret = 0;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Read data from the non volatile storage

\param offset_p  The offset of the data in the storage
\param pData_p   Pointer to the data to write
\param length_p  The length of the data to write

\return 0 on success; 1 on error

\ingroup module_nvs
*/
//------------------------------------------------------------------------------
UINT8 nvs_read(UINT32 offset_p, UINT8 * pReadData_p, UINT32 length_p)
{
    UINT8 ret = 1;

    if(pReadData_p != (UINT8*)0 && length_p > 0)
    {
        if(alt_read_flash(pFlashDesc_l, offset_p, pReadData_p, length_p) == 0)
        {
            ret = 0;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Erase the sector behind the offset

\param offset_p  The offset of the sector to erase

\return 0 on success; 1 on error

\ingroup module_nvs
*/
//------------------------------------------------------------------------------
UINT8 nvs_erase(UINT32 offset_p)
{
    UINT8 ret = 1;

    if(alt_erase_flash_block(pFlashDesc_l, offset_p, 0) == 0)
    {
        ret = 0;
    }

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief    Get the base address of the NVS memory offset

This functions enables to bypass the nvs_read function and to access the data
directly. This can be done on parallel flashes where no command is needed
to read data from the flash.

\return Pointer to the offset address

\ingroup module_nvs
*/
//------------------------------------------------------------------------------
UINT8* nvs_getAddress(UINT32 offset_p)
{
    return (UINT8*)(FLASH_BASE + offset_p);
}


//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief    Get the flash block index by offset

\param offset_p  The global flash offset

\return The id of the block the offset is in

\ingroup module_nvs
*/
//------------------------------------------------------------------------------
static INT16 getBlockByOffset(UINT32 offset_p)
{
    UINT16 i, j;
    flash_region* pFirstReg = (flash_region*)0;
    flash_region* pCurrReg = (flash_region*)0;
    int regionCount = 0;
    INT16 isInBlock = -1;

    if(alt_get_flash_info(pFlashDesc_l, &pFirstReg, &regionCount) == 0)
    {
        /* Iterate over all regions */
        for(i=0; i<(UINT16)regionCount; i++)
        {
            pCurrReg = &pFirstReg[i];

            if(offset_p > (UINT32)pCurrReg->offset)
                continue;

            /* Iterate over all blocks */
            for(j=0; j<(UINT16)pCurrReg->number_of_blocks; j++)
            {
                /* Check if the provided block is in the region */
                if(offset_p <= ((UINT32)pCurrReg->block_size * (j+1)))
                {
                    isInBlock = j;
                    break;
                }
            }

            /* If already found -> return */
            if(isInBlock != -1)
                break;

        }
    }

    return isInBlock;
}

/// \}
