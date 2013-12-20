/**
********************************************************************************
\file   appifcommon/global.h

\brief  Global header file for the application interface project

Global header file for the application interface project

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

#ifndef _INC_appifcommon_global_H_
#define _INC_appifcommon_global_H_

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include <ipcore/tbuf-cfg.h>

#ifdef APPIF_BUILD_PCP
  #include <pcptarget/target.h>
  #include <EplAmi.h>
#else
  #include <apptarget/target.h>
  #include <ami/ami.h>
#endif

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

// Create module defines
#define APPIF_MODULE_STATUS      0x00000001L    ///< Status module
#define APPIF_MODULE_CC          0x00000002L    ///< Configuration channel module
#define APPIF_MODULE_PDO         0x00000004L    ///< PDO module
#define APPIF_MODULE_SSDO        0x00000008L    ///< SSDO module

// Alignment macros
#define ALIGN16(ptr)        (((UINT16)(ptr) + 1U) & 0xFFFFFFFEU)   ///< aligns the pointer to UINT16 (2 byte)
#define ALIGN32(ptr)        (((UINT32)(ptr) + 3U) & 0xFFFFFFFCU)   ///< aligns the pointer to UINT32 (4 byte)

#define UNALIGNED16(ptr)    ((UINT16)(ptr) & 1U)    ///< checks if the pointer is UINT16-aligned
#define UNALIGNED32(ptr)    ((UINT32)(ptr) & 3U)    ///< checks if the pointer is UINT32-aligned

// Check if a bit is set
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

#ifndef UNUSED_PARAMETER
  #define UNUSED_PARAMETER(par)   (void)par
#endif

//------------------------------------------------------------------------------
// Simple return values
//------------------------------------------------------------------------------
#ifndef ERROR
  #define    ERROR    -1
#endif

#ifndef OK
  #define    OK        0
#endif

//------------------------------------------------------------------------------
// Boolean values
//------------------------------------------------------------------------------

#ifndef TRUE
  #define TRUE  0xFF
#endif

#ifndef FALSE
  #define FALSE 0x00
#endif


//------------------------------------------------------------------------------
// typedef
//------------------------------------------------------------------------------

/**
 * \brief Asynchronous frame handling sequence number type
 */
typedef enum {
    kSeqNrValueInvalid    = 0x00,   ///< Invalid sequence number (Other values are also invalid)
    kSeqNrValueFirst      = 0x56,   ///< Sequence number first packet arriving
    kSeqNrValueSecond     = 0xAA    ///< Sequence number second packet arriving
} tSeqNrValue;

/**
 * \brief Descriptor element of descriptor list
 */
typedef struct {
    UINT32      buffOffset_m;       ///< Offset of the triple buffer
    UINT16      buffSize_m;         ///< Size of the buffer
    UINT8       isConsumer_m;       ///< Descriptor is a consuming buffer
} tTbufDescriptor;

/**
 * \brief List of object sizes
 */
typedef enum eObjTypeSize {
    kTypeUint8Size   = 0x01,
    kTypeInt8Size    = 0x01,
    kTypeUint16Size  = 0x02,
    kTypeInt16Size   = 0x02,
    kTypeUint32Size  = 0x04,
    kTypeInt32Size   = 0x04,
    kTypeUint64Size  = 0x08,
    kTypeInt64Size   = 0x08
} tObjTypeSize;

/**
 * \brief One object from the cc object list
 */
typedef struct {
    UINT16 objIdx;
    UINT8  objSubIdx;
    UINT8  objSize;
} tCcObject;

/**
 * \brief Configuration channel object of local object list
 */
typedef struct {
    UINT16 objIdx_m;
    UINT8 objSubIdx_m;
    UINT8 objSize_m;
    UINT32 objPayloadLow_m;
    UINT32 objPayloadHigh_m;
} tConfChanObject;

/**
 * \brief List of all available application interface modules
 */
typedef enum {
    kAppIfModuleInvalid   = 0x00,
    kAppIfModuleStatus    = 0x01,
    kAppIfModuleCc        = 0x02,
    kAppIfModuleCcObject  = 0x03,
    kAppIfModulePdo       = 0x04,
    kAppIfModuleSsdo      = 0x05,
    kAppIfModuleTimeout   = 0x06,
    kAppIfModuleStream    = 0x07,
    kAppIfModuleInternal  = 0x08,
} tAppIfModules;

/**
 * \brief Global application interface status type
 */
typedef enum {
    kAppIfSuccessful                  = 0x00,
    kAppIfInitError                   = 0x01,
    kAppIfSpiSendReceiveError         = 0x02,
    kAppIfBufferSizeMismatch          = 0x03,
    kAppIfInvalidBuffer               = 0x04,
    kAppIfProcessSyncFailed           = 0x05,
    kAppIfProcessAsyncFailed          = 0x06,

    kAppIfMainPlkStackInitError       = 0x12,
    kAppIfMainPlkStackStartError      = 0x13,
    kAppIfMainPlkStackShutdownError   = 0x14,
    kAppIfMainPlkStackProcessError    = 0x15,

    kAppIfStatusInitError             = 0x20,
    kAppIfStatusRelTimeStateError     = 0x21,
    kAppIfStatusBufferSizeMismatch    = 0x22,
    kAppIfStatusProcessSyncFailed     = 0x23,

    kAppIfTbuffInitError              = 0x30,
    kAppIfTbuffReadError              = 0x31,
    kAppIfTbuffWriteError             = 0x32,

    kAppIfConfChanInitError           = 0x40,
    kAppIfConfChanBufferSizeMismatch  = 0x41,
    kAppIfConfChanObjectInitFailed    = 0x42,
    kAppIfConfChanObjectNotFound      = 0x43,
    kAppIfConfChanWriteToObDictFailed = 0x44,
    kAppIfConfChanInvalidSizeOfObj    = 0x45,
    kAppIfConfChanChannelBusy         = 0x46,
    kAppIfConfChanObjListOutOfSync    = 0x47,
    kAppIfConfChanInvalidParameter    = 0x48,
    kAppIfConfChanInvalidBuffer       = 0x49,
    kAppIfConfChanWriteToObjectFailed = 0x4A,
    kAppIfConfChanInitCcObjectFailed  = 0x4B,
    kAppIfConfChanObjLinkFailed       = 0x4C,

    kAppIfPdoInitError                = 0x50,
    kAppIfPdoProcessSyncFailed        = 0x51,

    kAppIfRpdoInitError               = 0x60,
    kAppIfRpdoBufferSizeMismatch      = 0x61,
    kAppIfRpdoInvalidBuffer           = 0x62,

    kAppIfTpdoInitError               = 0x70,
    kAppIfTpdoBufferSizeMismatch      = 0x71,
    kAppIfTpdoInvalidBuffer           = 0x72,

    kAppIfStreamInitError             = 0x80,
    kAppIfStreamInvalidBuffer         = 0x81,
    kAppIfStreamTransferError         = 0x82,
    kAppIfStreamInvalidParameter      = 0x83,
    kAppIfStreamNoFreeElementFound    = 0x84,
    kAppIfStreamProcessActionFailed   = 0x85,
    kAppIfStreamSyncError             = 0x86,

    kAppIfSsdoInitError               = 0x90,
    kAppIfSsdoInvalidParameter        = 0x91,
    kAppIfSsdoProcessingFailed        = 0x92,
    kAppIfSsdoWriteToObDictFailed     = 0x93,
    kAppIfSsdoDestinationUnknown      = 0x94,
    kAppIfSsdoTxConsSizeInvalid       = 0x95,
    kAppIfSsdoSendError               = 0x96,
    kAppIfSsdoNoFreeBuffer            = 0x97,
    kAppIfSsdoBufferSizeMismatch      = 0x98,
    kAppIfSsdoInvalidBuffer           = 0x99,
    kAppIfSsdoInvalidState            = 0x9A,
    kAppIfSsdoChannelBusy             = 0x9B,
    kAppIfSsdoInvalidTargetInfo       = 0x9C,

    kAppIfFifoInitFailed              = 0x100,
    kAppIfFifoInvalidParam            = 0x101,
    kAppIfFifoEmpty                   = 0x102,
    kAppIfFifoFull                    = 0x103,
    kAppIfFifoElementSizeOverflow     = 0x104,
    kAppIfFifoAlignError              = 0x105,

    kAppIfTimeoutOccured              = 0x200,
} tAppIfStatus;

/**
 * \brief Type for critical section entry function
 */
typedef void (* tAppIfCritSec) ( UINT8 fEnable_p );

//------------------------------------------------------------------------------
// function prototypes
//------------------------------------------------------------------------------

#endif /* _INC_appifcommon_global_H_ */



