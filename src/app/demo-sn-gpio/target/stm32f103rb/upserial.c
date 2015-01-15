/**
********************************************************************************
\file   demo-sn-gpio/target/stm32f103rb/upserial.c

\defgroup module_sn_stm32f103_upserial Cross communication serial module
\{

\brief  Implements the driver for the uP interconnect serial device

Defines the platform specific functions for the serial to interconnect the
uP-Master with the uP-Slave. (Target is the stm32f103rb board)

\ingroup group_app_sn_targ_stm32f103
*******************************************************************************/

/*------------------------------------------------------------------------------
* License Agreement
*
* Copyright 2014 BERNECKER + RAINER, AUSTRIA, 5142 EGGELSBERG, B&R STRASSE 1
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
#include <sn/upserial.h>

#include <common/platform.h>

#include <stm32f1xx_it.h>
#include <stm32f10x_usart.h>
#include <stm32f10x_gpio.h>
#include <stm32f10x_dma.h>

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
/* Defines for the USART peripheral */
#define USARTx                          USART1
#define USARTx_RCC_PERIPH               RCC_APB2Periph_USART1


/* Definition for USARTx Pins */
#define USARTx_RX_PIN                   GPIO_Pin_10
#define USARTx_TX_PIN                   GPIO_Pin_9

#define USARTx_GPIO_PORT                GPIOA

#define USARTx_GPIO_RCC_PERIPH          RCC_APB2Periph_GPIOA

/* Defines for the DMA channels */
#define DMAx_Channel_Tx                 DMA1_Channel4
#define DMAx_Channel_Rx                 DMA1_Channel5

#define DMAx_ChannelTx_IRQn             DMA1_Channel4_IRQn
#define DMAx_ChannelRx_IRQn             DMA1_Channel5_IRQn

#define DMAx_ChannelTx_IRQHandler       DMA1_Channel4_IRQHandler
#define DMAx_ChannelRx_IRQHandler       DMA1_Channel5_IRQHandler

#define DMAx_RCC_PERIPH                 RCC_AHBPeriph_DMA1

#define DMAx_FLAG_TE_Tx                 DMA1_FLAG_TE4
#define DMAx_FLAG_TE_Rx                 DMA1_FLAG_TE5
#define DMAx_FLAG_TC_Tx                 DMA1_FLAG_TC4
#define DMAx_FLAG_TC_Rx                 DMA1_FLAG_TC5
#define DMAx_IT_TE_Tx                   DMA1_IT_TE4
#define DMAx_IT_TE_Rx                   DMA1_IT_TE5
#define DMAx_IT_TC_Tx                   DMA1_IT_TC4
#define DMAx_IT_TC_Rx                   DMA1_IT_TC5

/*----------------------------------------------------------------------------*/
/* local types                                                                */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/
static tUpSerialTransferFin pfnTransfFin_l = NULL;      /**< This callback is called when a transfer is finished */
static tUpSerialReceiveFin pfnReceiveFin_l = NULL;      /**< This callback is called on reception of a new frame */
static tUpSerialTransferError pfnTransfError_l = NULL;  /**< This callback is called on serial error */

static volatile BOOLEAN fTxFinished_l = FALSE;       /**< Transfer finished flag for blocking mode */
static volatile BOOLEAN fRxFinished_l = FALSE;       /**< Receive finished flag for blocking mode */

/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/
static void initGpio(void);
static void initUsart(void);
static void initDmaRx(volatile UINT8 * pTargBase_p, UINT32 targLen_p);
static void initDmaTx(volatile UINT8 * pSrcBase_p, UINT32 srcLen_p);
static void closeDma(void);
static void initNvic(void);

static void usartReceive(volatile UINT8 * pData_p, UINT32 size_p);
static void usartTransmit(volatile UINT8 * pData_p, UINT32 size_p);

static BOOLEAN waitForTransferFinished(volatile BOOLEAN * pTransFin_p, UINT32 timeoutMs_p);

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize the up serial device

This function initializes the serial device connecting the uP-Master with
the uP-Slave.

\retval TRUE    On success
\retval FALSE   USART initialization failed
*/
/*----------------------------------------------------------------------------*/
BOOLEAN upserial_init(void)
{
    /* Initialize all needed peripherals */
    initGpio();
    initUsart();

    /* The DMA channels are initialized on transfer */
    closeDma();

    initNvic();

    /* Globally enable the USARTx core */
    USART_Cmd(USARTx, ENABLE);

    return TRUE;
}


/*----------------------------------------------------------------------------*/
/**
\brief  Close the USART serial
*/
/*----------------------------------------------------------------------------*/
void upserial_exit(void)
{
    pfnTransfFin_l =  NULL;
    pfnReceiveFin_l = NULL;
    pfnTransfError_l = NULL;

    USART_DeInit(USARTx);

    /* Close the DMAx channels */
    closeDma();

    /* Close the GPIO pins */
    GPIO_DeInit(USARTx_GPIO_PORT);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Register serial callback functions

\param pfnTransfFin_p     Pointer to the transfer finished callback
\param pfnReceivefFin_p   Pointer to the receive finished callback
\param pfnTransfError_p   Pointer to the transfer error callback
*/
/*----------------------------------------------------------------------------*/
void upserial_registerCb(tUpSerialTransferFin pfnTransfFin_p,
                         tUpSerialReceiveFin pfnReceivefFin_p,
                         tUpSerialTransferError pfnTransfError_p)
{
    pfnTransfFin_l = pfnTransfFin_p;
    pfnReceiveFin_l = pfnReceivefFin_p;
    pfnTransfError_l = pfnTransfError_p;
}

/*----------------------------------------------------------------------------*/
/**
\brief  Deregister serial callback functions
*/
/*----------------------------------------------------------------------------*/
void upserial_deRegisterCb(void)
{
    pfnTransfFin_l = NULL;
    pfnReceiveFin_l = NULL;
    pfnTransfError_l = NULL;
}

/*----------------------------------------------------------------------------*/
/**
\brief  Perform an USART transfer

This function starts an USART transfer to send data to the second processor.

\note This function is blocking

\param[in] pData_p       Pointer to the transmit data
\param[in] size_p        Size of the transmit data

\retval TRUE        On success
\retval FALSE       USART send failed
*/
/*----------------------------------------------------------------------------*/
BOOLEAN upserial_transmitBlock(volatile UINT8 * pData_p, UINT32 size_p)
{
    BOOLEAN fReturn = FALSE;

    if(pData_p != NULL && size_p > 0)
    {
        usartTransmit(pData_p, size_p);

        /* Wait until DMA transfer is finished */
        if(waitForTransferFinished(&fTxFinished_l, 0xFFFF))
        {
            /* Frame successfully transmitted */
            fReturn = TRUE;
        }
        else
        {
            /* Timeout occurred */
            USART_DMACmd(USARTx, USART_DMAReq_Tx, DISABLE);
        }

        fTxFinished_l = FALSE;
    }

    return fReturn;
}


/*----------------------------------------------------------------------------*/
/**
\brief  Perform an USART reception

This function blocks until the desired amount of bytes is received from the uart
serial.

\note This function is blocking

\param[in] pData_p       Pointer to the received data
\param[in] size_p        Size of the received data
\param[in] timeoutMs_p   The timeout in ms

\retval TRUE        On success
\retval FALSE       USART receive failed
*/
/*----------------------------------------------------------------------------*/
BOOLEAN upserial_receiveBlock(volatile UINT8 * pData_p, UINT32 size_p, UINT32 timeoutMs_p)
{
    BOOLEAN fReturn = FALSE;

    if(pData_p != NULL && size_p > 0)
    {
        usartReceive(pData_p, size_p);

        /* Wait until DMA transfer is finished */
        if(waitForTransferFinished(&fRxFinished_l, timeoutMs_p))
        {
            /* Frame successfully received */
            fReturn = TRUE;
        }
        else
        {
            /* Timeout occurred */
            USART_DMACmd(USARTx, USART_DMAReq_Rx, DISABLE);
        }

        fRxFinished_l = FALSE;

    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief  Enable a DMA reception

This function enables a DMA reception via the local uP-Master <-> uP-Slave
serial device.

\param[in] pData_p       Pointer to the received data
\param[in] size_p        Size of the received data

\retval TRUE        On success
\retval FALSE       USART receive failed
*/
/*----------------------------------------------------------------------------*/
BOOLEAN upserial_enableReceive(volatile UINT8 * pData_p, UINT32 size_p)
{
    BOOLEAN fReturn = FALSE;

    if(pData_p != NULL && size_p > 0)
    {
        usartReceive(pData_p, size_p);

        fReturn = TRUE;
    }

    return fReturn;
}

/*----------------------------------------------------------------------------*/
/**
\brief  Start an USART transfer

This function starts an USART DMA transfer to send data to the second
processor.

\param[in] pData_p       Pointer to the transmit data
\param[in] size_p        Size of the transmit data

\retval TRUE        On success
\retval FALSE       USART send failed
*/
/*----------------------------------------------------------------------------*/
BOOLEAN upserial_transmit(volatile UINT8 * pData_p, UINT32 size_p)
{
    BOOLEAN fReturn = FALSE;

    if(pData_p != NULL && size_p > 0)
    {
        usartTransmit(pData_p, size_p);

        fReturn = TRUE;
    }

    return fReturn;
}

/*============================================================================*/
/*            P R I V A T E   F U N C T I O N S                               */
/*============================================================================*/
/** \name Private Functions */
/** \{ */

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize the USART GPIO pins
*/
/*----------------------------------------------------------------------------*/
static void initGpio(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitTypeDef));

    /* Enable peripheral clocks */
    RCC_APB2PeriphClockCmd(USARTx_GPIO_RCC_PERIPH, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    /*  USARTx: TX */
    GPIO_InitStructure.GPIO_Pin = USARTx_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(USARTx_GPIO_PORT, &GPIO_InitStructure);

    /*  USARTx: RX */
    GPIO_InitStructure.GPIO_Pin = USARTx_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USARTx_GPIO_PORT, &GPIO_InitStructure);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize the USART core

Setup USART core for receive and transmit
*/
/*----------------------------------------------------------------------------*/
static void initUsart(void)
{
    USART_InitTypeDef USART_InitStructure;

    memset(&USART_InitStructure, 0, sizeof(USART_InitTypeDef));

    /* Enable the USART periph */
    RCC_APB2PeriphClockCmd(USARTx_RCC_PERIPH, ENABLE);

    /* Configure the USART core */
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = 3686400; //1843200//; //10500000;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USARTx, &USART_InitStructure);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize USART DMA for receive

Setup the DMA for receive from USART to RAM.

\param[in] pTargBase_p      Pointer to the base address of the target buffer
\param[in] targLen_p        The length of the target buffer
*/
/*----------------------------------------------------------------------------*/
static void initDmaRx(volatile UINT8 * pTargBase_p, UINT32 targLen_p)
{
    DMA_InitTypeDef DMA_InitStructure;

    /* Configure DMAx - ChannelRx (USART -> memory) */
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USARTx->DR;     /* Address of peripheral the DMA must map to */
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pTargBase_p;         /* Variable to which received data will be stored */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = targLen_p;                         /* Buffer size */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMAx_Channel_Rx, &DMA_InitStructure);

    DMA_ITConfig(DMAx_Channel_Rx, DMA_IT_TC | DMA_IT_TE, ENABLE);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize USART DMA for transmit

Setup the DMA for transmit from RAM to USART.

\param[in] pSrcBase_p      Pointer to the base address of the source buffer
\param[in] srcLen_p        The length of the source buffer
*/
/*----------------------------------------------------------------------------*/
static void initDmaTx(volatile UINT8 * pSrcBase_p, UINT32 srcLen_p)
{
    DMA_InitTypeDef DMA_InitStructure;

    /* Configure DMAx - ChannelTx (memory -> USART) */
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USARTx->DR;     /* Address of peripheral the DMA must map to */
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pSrcBase_p;          /* Variable from which data will be transmitted */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = srcLen_p;                          /* Buffer size */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMAx_Channel_Tx, &DMA_InitStructure);

    DMA_ITConfig(DMAx_Channel_Tx, DMA_IT_TC | DMA_IT_TE, ENABLE);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Close the DMA channels
*/
/*----------------------------------------------------------------------------*/
static void closeDma(void)
{
    /* Disable DMA interrupts */
    DMA_ITConfig(DMAx_Channel_Rx, DMA_IT_TC | DMA_IT_TE, DISABLE);
    DMA_ITConfig(DMAx_Channel_Tx, DMA_IT_TC | DMA_IT_TE, DISABLE);

    /* Close the DMA channels */
    DMA_DeInit(DMAx_Channel_Rx);
    DMA_DeInit(DMAx_Channel_Tx);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize the DMA interrupts in NVIC

Setup transfer finished and transfer error interrupts for both DMA
channels.
*/
/*----------------------------------------------------------------------------*/
static void initNvic(void)
{
    NVIC_InitTypeDef   NVIC_InitStructure;

    memset(&NVIC_InitStructure, 0, sizeof(NVIC_InitTypeDef));

    /* Enable DMA1 Channel2 interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DMAx_ChannelRx_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0E;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable DMA1 Channel3 interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DMAx_ChannelTx_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0E;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Start an USART DMA reception

\param[in] pData_p      Pointer to the base address of the target buffer
\param[in] size_p       The length of the target buffer
*/
/*----------------------------------------------------------------------------*/
static void usartReceive(volatile UINT8 * pData_p, UINT32 size_p)
{
    /* Initialize the DMA receive channel with the new base address */
    initDmaRx(pData_p, size_p);

    /* Enable the DMA receive channel */
    DMA_Cmd(DMAx_Channel_Rx, ENABLE);

    /* Enable the USART DMA receive channel */
    USART_DMACmd(USARTx, USART_DMAReq_Rx, ENABLE);
}

/*----------------------------------------------------------------------------*/
/**
\brief  Start an USART DMA transmission

\param[in] pData_p      Pointer to the base address of the source buffer
\param[in] size_p       The length of the source buffer
*/
/*----------------------------------------------------------------------------*/
static void usartTransmit(volatile UINT8 * pData_p, UINT32 size_p)
{
    /* Initialize the DMA receive channel with the new base address */
    initDmaTx(pData_p, size_p);

    /* Enable the DMA transmit channel */
    DMA_Cmd(DMAx_Channel_Tx, ENABLE);

    /* Enable the USARTx DMA interface (This starts the transfer!) */
    USART_DMACmd(USARTx, USART_DMAReq_Tx, ENABLE);
}

/*----------------------------------------------------------------------------*/
/**
\brief  DMA receive channel interrupt handler (USART -> memory)
*/
/*----------------------------------------------------------------------------*/
void DMAx_ChannelRx_IRQHandler(void)
{
    if(DMA_GetITStatus(DMAx_IT_TC_Rx) == SET)
    {
        /* Disable the DMA channel */
        DMA_Cmd(DMAx_Channel_Rx, DISABLE);

        fRxFinished_l = TRUE;

        /* Call receive finished callback function */
        if(pfnReceiveFin_l != NULL)
            pfnReceiveFin_l();

        /* Clear the interrupt flag */
        DMA_ClearFlag(DMAx_FLAG_TC_Rx);
    }
    else if(DMA_GetITStatus(DMAx_IT_TE_Rx) == SET)
    {   /* Transfer error! */

        /* Call transfer error callback function */
        if(pfnTransfError_l != NULL)
            pfnTransfError_l();

        DMA_ClearFlag(DMAx_FLAG_TE_Rx);
    }
}



/*----------------------------------------------------------------------------*/
/**
\brief  DMA transmit channel interrupt handler (memory -> USART)
*/
/*----------------------------------------------------------------------------*/
void DMAx_ChannelTx_IRQHandler(void)
{
    if(DMA_GetITStatus(DMAx_IT_TC_Tx) == SET)
    {
        /* Disable the DMA channel */
        DMA_Cmd(DMAx_Channel_Tx, DISABLE);

        fTxFinished_l = TRUE;

        /* Call transfer finished callback function */
        if(pfnTransfFin_l != NULL)
            pfnTransfFin_l();

        /* Clear the interrupt flag */
        DMA_ClearFlag(DMAx_FLAG_TC_Tx);
    }
    else if(DMA_GetITStatus(DMAx_IT_TE_Tx) == SET)
    {
        /* Call transfer error callback function */
        if(pfnTransfError_l != NULL)
            pfnTransfError_l();

        DMA_ClearFlag(DMAx_FLAG_TE_Tx);
    }
}

/*----------------------------------------------------------------------------*/
/**
\brief  Wait until the transfer is finished

\param[in] pTransFin_p      Pointer to the transfer finished flag
\param[in] timeoutMs_p      Time in ms until timeout
*/
/*----------------------------------------------------------------------------*/
static BOOLEAN waitForTransferFinished(volatile BOOLEAN * pTransFin_p, UINT32 timeoutMs_p)
{
    BOOLEAN fTransFin = FALSE;
    volatile UINT32 currTime = tickCnt_l;
    UINT32 timeoutTime = currTime + timeoutMs_p;

    /* Wait until timeout is reached or transfer is finished */
    while(*pTransFin_p == FALSE && currTime < timeoutTime)
    {
        currTime = tickCnt_l;
    }

    fTransFin = *pTransFin_p;

    return fTransFin;
}

/**
 * \}
 * \}
 */
