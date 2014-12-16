/**
********************************************************************************
\file   sodstore.c

\brief  Stores and restores the SOD to non volatile memory

This module stores the whole parameter set to non volatile memory which enables
a fast bootup on SN start.

\ingroup module_sapl
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

/*----------------------------------------------------------------------------*/
/* includes                                                                   */
/*----------------------------------------------------------------------------*/

#include <sapl/sodstore.h>

#include <sn/nvs.h>

#include <SCFMapi.h>

/*============================================================================*/
/*            G L O B A L   D E F I N I T I O N S                             */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/
#define NVS_IMG_OFFSET_MAGIC        (UINT8)0x00         /**< Offset of the magic word field in the image */
#define NVS_IMG_OFFSET_LENGTH       (UINT8)0x04         /**< Offset of the length field in the image */
#define NVS_IMG_OFFSET_CRC32        (UINT8)0x08         /**< Offset of the crc32 field in the image */
#define NVS_IMG_OFFSET_DATA         (UINT8)0x0C         /**< Offset of the data field in the image */


/*----------------------------------------------------------------------------*/
/* module global vars                                                         */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* global function prototypes                                                 */
/*----------------------------------------------------------------------------*/
extern UINT32 HNFiff_Crc32CalcSwp(UINT32 w_initCrc, INT32 l_length,
                                  const void *pv_data);


/*============================================================================*/
/*            P R I V A T E   D E F I N I T I O N S                           */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/* const defines                                                              */
/*----------------------------------------------------------------------------*/
#define NVS_MAGIC_WORD      0xdeadbeefUL     /**< Magic word which indicates the start of the SOD image */

#define NVS_IMAGE_DATA_CHUNK_SIZE       20   /**< Size of one chunk written to the NVS in one process cycle */

/*----------------------------------------------------------------------------*/
/* local types                                                                */
/*----------------------------------------------------------------------------*/

/**
 * \brief This type represents the states of the SOD store module
 */
typedef enum
{
    kSodStoreStateInvalid       = 0x0,  /**< Invalid state of the SOD store module */
    kSodStoreStateInit          = 0x1,  /**< SOD store is in init state */
    kSodStoreStateAddCrc        = 0x2,  /**< Calculate a CRC over the parameter set */
    kSodStoreStateProcess       = 0x3,  /**< SOD store is currently processing */
    kSodStoreStateFinished      = 0x4,  /**< Finished to store objects to the NVS */
} tSodStoreState;

/**
 * \brief SOD store module instance type
 */
typedef struct
{
    tSodStoreState sodStoreState_m;      /**< The current state of the SOD store module */
    UINT32 currDataPos_m;                /**< Current image write position */
    UINT32 currParamSetOffs_m;           /**< Write offset in the current parameter set */
} tSodStoreInstance;

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/
static tSodStoreInstance sodStoreInstance_l;

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/
static BOOLEAN eraseNvsSector(UINT32 offset_p);
static BOOLEAN storeHeaderToNvs(UINT32 paramSetLen_p);

static BOOLEAN verifyMagic(void);
static BOOLEAN verifyParamSetCrc(UINT32 paramSetLen_p);

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief    Initialize the SOD storage module

\retval TRUE    Successfully initialized the SOD storage
\retval FALSE   Error on init

\ingroup module_sodstore
*/
/*----------------------------------------------------------------------------*/
BOOLEAN sodstore_init(void)
{
    BOOLEAN fReturn = FALSE;

    MEMSET(&sodStoreInstance_l, 0, sizeof(tSodStoreInstance));

    sodStoreInstance_l.sodStoreState_m = kSodStoreStateInit;

    /* Initialize the non volatile storage on this target */
    if(nvs_init())
    {
        fReturn = TRUE;
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Close the SOD storage

\ingroup module_sodstore
*/
/*----------------------------------------------------------------------------*/
void sodstore_close(void)
{
    /* Close the non volatile storage */
    nvs_close();
}

/*----------------------------------------------------------------------------*/
/**
\brief    Process the SOD storage module

\param pParamSetBase_p      The base address of the parameter set
\param paramSetLen_p        The length of the parameter set

\retval kSodStoreProcError      Error on processing
\retval kSodStoreProcBusy       Processing is currently busy
\retval kSodStoreProcFinished   Finished processing

\ingroup module_sodstore
*/
/*----------------------------------------------------------------------------*/
tProcStoreRet sodstore_process(UINT8* pParamSetBase_p, UINT32 paramSetLen_p)
{
    tProcStoreRet sodStoreRet = kSodStoreProcError;
    UINT32 writeChunkSize;
    UINT8 writeRet;
    UINT32 paramCrc;

    if(pParamSetBase_p != NULL && paramSetLen_p > 0)
    {
        switch(sodStoreInstance_l.sodStoreState_m)
        {
            case kSodStoreStateInit:
            {
                DEBUG_TRACE(DEBUG_LVL_ALWAYS, "\nStore parameter set to NVS -> ");

                sodStoreInstance_l.currDataPos_m = NVS_IMG_OFFSET_DATA;
                sodStoreInstance_l.currParamSetOffs_m = 0;

                /* Erase the sector behind the offset before storing data to it */
                if(eraseNvsSector(NVS_IMG_OFFSET_MAGIC))
                {
                    /* Store the header information to the NVS */
                    if(storeHeaderToNvs(paramSetLen_p))
                    {
                        /* Switch the current state to add CRC */
                        sodStoreInstance_l.sodStoreState_m = kSodStoreStateAddCrc;

                        sodStoreRet = kSodStoreProcBusy;
                    }
                }

                break;
            }
            case kSodStoreStateAddCrc:
            {
                /* Calculate a CRC32 over all parameters */
                paramCrc = HNFiff_Crc32CalcSwp(0, paramSetLen_p, pParamSetBase_p);
                if(paramCrc > 0)
                {
                    /* Store the CRC32 at the end of the header info in the NVS */
                    writeRet = nvs_write(NVS_IMG_OFFSET_CRC32, (UINT8*)&paramCrc, sizeof(paramCrc));
                    if(writeRet == TRUE)
                    {
                        /* Perform switch to process state */
                        sodStoreInstance_l.sodStoreState_m = kSodStoreStateProcess;
                        sodStoreRet = kSodStoreProcBusy;
                    }
                    else
                    {
                        errh_postFatalError(kErrSourceSapl, kErrorSodStoreWriteError, 0);
                    }
                }
                else
                {
                    errh_postFatalError(kErrSourceSapl, kErrorSodStoreCrcError, 0);
                }
                break;
            }
            case kSodStoreStateProcess:
            {
                if((sodStoreInstance_l.currParamSetOffs_m + NVS_IMAGE_DATA_CHUNK_SIZE) < paramSetLen_p)
                {
                    /* The next write cycle is a full chunk */
                    writeChunkSize = NVS_IMAGE_DATA_CHUNK_SIZE;
                }
                else
                {
                    /* Get the length of the last chunk of data */
                    writeChunkSize = paramSetLen_p - sodStoreInstance_l.currParamSetOffs_m;
                }

                /* Write the next data chunk to the NVS */
                writeRet = nvs_write(sodStoreInstance_l.currDataPos_m,
                                     pParamSetBase_p + sodStoreInstance_l.currParamSetOffs_m,
                                     writeChunkSize);
                if(writeRet == TRUE)
                {
                    sodStoreInstance_l.currDataPos_m += writeChunkSize;
                    sodStoreInstance_l.currParamSetOffs_m += writeChunkSize;

                    if(sodStoreInstance_l.currParamSetOffs_m < paramSetLen_p)
                    {
                        sodStoreRet = kSodStoreProcBusy;
                    }
                    else if(sodStoreInstance_l.currParamSetOffs_m == paramSetLen_p)
                    {
                        /* Switch to finished state */
                        sodStoreInstance_l.sodStoreState_m = kSodStoreStateFinished;
                        sodStoreRet = kSodStoreProcBusy;
                    }
                    else
                    {
                        /* Image has an invalid size! */
                        errh_postFatalError(kErrSourceSapl, kErrorSodStoreSizeMissmatch,
                                            sodStoreInstance_l.currParamSetOffs_m);
                    }
                }
                else
                {
                    errh_postFatalError(kErrSourceSapl, kErrorSodStoreWriteError, 0);
                }

                break;
            }
            case kSodStoreStateFinished:
            {
                /* Reset internal structures */
                sodStoreInstance_l.currDataPos_m = 0;
                sodStoreInstance_l.currParamSetOffs_m = 0;

                /* Reset state to init */
                sodStoreInstance_l.sodStoreState_m = kSodStoreStateInit;

                /* call the Control Flow Monitoring */
                SCFM_TACK_PATH();

                sodStoreRet = kSodStoreProcFinished;

                DEBUG_TRACE(DEBUG_LVL_ALWAYS, "Success!\n");
                break;
            }
            default:
            {
                errh_postFatalError(kErrSourceSapl, kErrorInvalidState, 0);
                break;
            }
        }
    }
    else
    {
        errh_postFatalError(kErrSourceSapl, kErrorInvalidParameter, 0);
    }

    return sodStoreRet;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Verify the SOD image in the NVS and return the image parameters

\param[out] ppParamSetBase_p      Pointer to the base address of the image
\param[out] pParamSetLen_p        Pointer to the length of the image

\retval TRUE    The image is valid and can be restored
\retval FALSE   No valid image available

\ingroup module_sodstore
*/
/*----------------------------------------------------------------------------*/
BOOLEAN sodstore_getSodImage(UINT8** ppParamSetBase_p, UINT32* pParamSetLen_p)
{
    BOOLEAN fReturn = FALSE;
    UINT32 * pParamSetLen = NULL;

    if(ppParamSetBase_p != NULL && pParamSetLen_p != NULL)
    {
        /* Verify if there is a magic word */
        if(verifyMagic())
        {
            /* Read the length of the parameter set */
            if(nvs_readUint32(NVS_IMG_OFFSET_LENGTH, &pParamSetLen))
            {
                if(*pParamSetLen > 0 && *pParamSetLen < UINT32_MAX)
                {
                    /* Verify if the CRC stored in the flash matches the data stream */
                    if(verifyParamSetCrc(*pParamSetLen))
                    {
                        /* Set the output values */
                        *ppParamSetBase_p = nvs_getAddress(NVS_IMG_OFFSET_DATA);
                        *pParamSetLen_p = *pParamSetLen;

                        fReturn = TRUE;
                    }
                }
            }
        }
    }
    else
    {
        errh_postFatalError(kErrSourceSapl, kErrorInvalidParameter, 0);
    }

    return fReturn;
}

/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/* \name Private Functions */
/* \{ */

/*----------------------------------------------------------------------------*/
/**
\brief    Erase the memory before storing the new parameter set

\param offset_p      The offset of the memory to erase

\retval TRUE    Success on erase
\retval FALSE   Error on erase

\ingroup module_sodstore
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN eraseNvsSector(UINT32 offset_p)
{
    BOOLEAN fReturn = FALSE;

    if(nvs_erase(offset_p))
    {
        fReturn = TRUE;
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Store the header of the parameter set into flash

\param paramSetLen_p      The length of the parameter set

\retval TRUE    Success on erase
\retval FALSE   Error on erase

\ingroup module_sodstore
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN storeHeaderToNvs(UINT32 paramSetLen_p)
{
    BOOLEAN fReturn = FALSE;
    UINT32 nvsMagic = NVS_MAGIC_WORD;
    BOOLEAN writeRet = FALSE;

    /* Write magic to the start of the SOD image */
    writeRet = nvs_write(NVS_IMG_OFFSET_MAGIC, (UINT8*)&nvsMagic, sizeof(nvsMagic));
    if(writeRet == TRUE)
    {
        /* Write the size of the image to the NVS */
        writeRet = nvs_write(NVS_IMG_OFFSET_LENGTH,
                             (UINT8*)&paramSetLen_p, sizeof(paramSetLen_p));
        if(writeRet == TRUE)
        {
            fReturn = TRUE;
        }
        else
        {
            errh_postFatalError(kErrSourceSapl, kErrorSodStoreWriteError, 0);
        }
    }
    else
    {
        errh_postFatalError(kErrSourceSapl, kErrorSodStoreWriteError, 0);
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Verify the magic word in the flash

\retval TRUE    The magic word in the NVS is valid
\retval FALSE   No magic word in the NVS

\ingroup module_sodstore
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN verifyMagic(void)
{
    BOOLEAN magicValid = FALSE;
    UINT32 * pReadMagic = NULL;
    UINT32 refMagic = NVS_MAGIC_WORD;

    /* Verify the magic word */
    if(nvs_readUint32(NVS_IMG_OFFSET_MAGIC, &pReadMagic) == TRUE)
    {
        if(*pReadMagic == refMagic)
            magicValid = TRUE;
    }

    return magicValid;
}

/*----------------------------------------------------------------------------*/
/**
\brief    Verify the NVS image by using the parameter CRC

\param paramSetLen_p    The length of the parameter set

\retval TRUE    The CRC was successfully verified
\retval FALSE   CRC error -> The image is not valid

\ingroup module_sodstore
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN verifyParamSetCrc(UINT32 paramSetLen_p)
{
    BOOLEAN fCrcValid = FALSE;
    UINT32 * pReadCrc = NULL;
    UINT32 calcCrc;
    UINT8* pParamSet = nvs_getAddress(NVS_IMG_OFFSET_DATA);

    /* Read the parameter set CRC from NVS */
    if(nvs_readUint32(NVS_IMG_OFFSET_CRC32, &pReadCrc) == TRUE)
    {
        /* Calculate the parameter CRC over the flash image */
        calcCrc = HNFiff_Crc32CalcSwp(0, paramSetLen_p, pParamSet);
        if(calcCrc == *pReadCrc)
        {
            fCrcValid = TRUE;
        }
    }

    return fCrcValid;
}

/* \} */

