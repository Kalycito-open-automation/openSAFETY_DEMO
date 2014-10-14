/**
********************************************************************************
\file   serial.c

\brief  Implements the driver for the serial device

Defines the platform specific functions for the serial for target
stm32f103rb.

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
#include <common/serial.h>

#include <stm32f10x_spi.h>
#include <stm32f10x_dma.h>
#include <stm32f10x_gpio.h>

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

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static void initGpio(void);
static void initSpi(void);
static void initDma(tHandlerParam * pTransParam_p);
static void initNvic(void);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief  Initialize the peripherals of the target

This function init's the peripherals of the AP like cache and the interrupt
controller.

\param pTransParam_p The transfer parameters (rx/tx base and size)

\retval TRUE    On success
\retval FALSE   On error

\ingroup module_serial
*/
//------------------------------------------------------------------------------
BOOL serial_init(tHandlerParam * pTransParam_p)
{
    UINT8 fReturn = FALSE;

    if(pTransParam_p != NULL)
    {
        /* Initialize all needed peripherals */
        initGpio();
        initSpi();
        initDma(pTransParam_p);
        initNvic();

        /* Set NSS high (not active!) */
        GPIOB->BSRR = GPIO_Pin_6;

        /* Init the SPI1 core */
        SPI_Cmd(SPI1, ENABLE);

        /* Disable both DMA channels for now */
        DMA_Cmd(DMA1_Channel2, DISABLE);
        DMA_Cmd(DMA1_Channel3, DISABLE);

        /* Enable SPI DMA interface */
        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);

        fReturn = TRUE;
    }

    return fReturn;
}

//------------------------------------------------------------------------------
/**
\brief  Close all peripherals of the target

\ingroup module_serial
*/
//------------------------------------------------------------------------------
void serial_exit(void)
{
    //TODO!
}

//------------------------------------------------------------------------------
/**
\brief  Start an SPI transfer

serial_transfer() sends starts an SPI DMA transfer to exchange the process
image with the PCP.

\param[in] pHandlParam_p       The parameters of the SPI transfer handler

\retval TRUE        On success
\retval FALSE       SPI send or receive failed

\ingroup module_serial
*/
//------------------------------------------------------------------------------
BOOL serial_transfer(tHandlerParam* pHandlParam_p)
{
    /* Reset the current data counter value */
    DMA_SetCurrDataCounter(DMA1_Channel2 , pHandlParam_p->consDesc_m.buffSize_m + 4);
    DMA_SetCurrDataCounter(DMA1_Channel3 , pHandlParam_p->prodDesc_m.buffSize_m + 4);

    /* Set NSS low (active) */
    GPIOB->BRR = GPIO_Pin_6;

    /* Enable both DMA channels */
    DMA_Cmd(DMA1_Channel2, ENABLE);
    DMA_Cmd(DMA1_Channel3, ENABLE);

    /* Enable the SPI1 DMA interface (This starts the transfer!) */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx, ENABLE);

    return TRUE;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief  Initialize the SPI GPIO pins

Enable SCK, MOSI and NSS as outputs and MISO as input.

\ingroup module_serial
*/
//------------------------------------------------------------------------------
static void initGpio(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    memset(&GPIO_InitStructure, 0, sizeof(GPIO_InitTypeDef));

    /* Enable GPIOA, GPIOB and AFIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO  , ENABLE);

    /*  SPI1 : SCK, MISO and MOSI */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;  /* SCK = Pin5 || MOSI = Pin7 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;   /* MISO = Pin6 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //NSS   SPI_NSS_Soft
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;   /* NSS = Pin6 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//------------------------------------------------------------------------------
/**
\brief  Initialize the SPI core as master

Setup SPI core as full duplex 8bit data with CPOL, CPHA = (0,0)

\ingroup module_serial
*/
//------------------------------------------------------------------------------
static void initSpi(void)
{
    SPI_InitTypeDef SPI_InitStructure;

    memset(&SPI_InitStructure, 0, sizeof(SPI_InitTypeDef));

    /* Enable the SPI1 periph */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    /* Reset SPI Interface */
    SPI_I2S_DeInit(SPI1);

    /* SPI1 configuration */
    SPI_StructInit(&SPI_InitStructure);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; //SPI_BaudRatePrescaler_2;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
}

//------------------------------------------------------------------------------
/**
\brief  Initialize DMA channel 2 and 3

Setup the DMA for receive from SPI to RAM and back!

\param pTransParam_p The transfer parameters

\ingroup module_serial
*/
//------------------------------------------------------------------------------
static void initDma(tHandlerParam * pTransParam_p)
{
    DMA_InitTypeDef DMA_InitStructure;

    memset(&DMA_InitStructure, 0, sizeof(DMA_InitTypeDef));

    /* Enable DMA1 clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* Configure DMA1 - Channel2 (SPI -> memory) */
    DMA_DeInit(DMA1_Channel2);    /* Reset DMA registers to default values */

    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;     /* Address of peripheral the DMA must map to */
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pTransParam_p->consDesc_m.pBuffBase_m;    /* Variable to which received data will be stored */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = pTransParam_p->consDesc_m.buffSize_m;    /* Buffer size */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Channel2, DMA_IT_TC | DMA_IT_TE, ENABLE);

    /* Configure DMA1 - Channel3 (memory -> SPI) */
    DMA_DeInit(DMA1_Channel3);    /* Reset DMA registers to default values */

    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;     /* Address of peripheral the DMA must map to */
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pTransParam_p->prodDesc_m.pBuffBase_m;    /* Variable from which data will be transmitted */
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = pTransParam_p->prodDesc_m.buffSize_m;    /* Buffer size */
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Channel3, DMA_IT_TC | DMA_IT_TE, ENABLE);
}

//------------------------------------------------------------------------------
/**
\brief  Initialize the DMA interrupts in NVIC

Setupt transfer finished and transfer error interrupts for both DMA
channels.

\ingroup module_serial
*/
//------------------------------------------------------------------------------
static void initNvic(void)
{
    NVIC_InitTypeDef   NVIC_InitStructure;

    memset(&NVIC_InitStructure, 0, sizeof(NVIC_InitTypeDef));

    /* Enable DMA1 Channel2 interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0E;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable DMA1 Channel3 interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0E;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

//------------------------------------------------------------------------------
/**
\brief  DMA channel2 interrupt handler (SPI -> memory)

\ingroup module_serial
*/
//------------------------------------------------------------------------------
void DMA1_Channel2_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC2) == SET)
    {
        /* Disable the DMA channel */
        DMA_Cmd(DMA1_Channel2, DISABLE);

        /* Wait until SPI is finished -> Then disable the SPI -> DMA interface */
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);

        /* Set NSS high (not active!) */
        GPIOB->BSRR = GPIO_Pin_6;

        /* Clear the interrupt flag */
        DMA_ClearFlag(DMA1_FLAG_TC2);
    }
    else if(DMA_GetITStatus(DMA1_IT_TE2) == SET)
    {
        /* Transfer error! */
        //TODO! handle this error?
        DMA_ClearFlag(DMA1_FLAG_TE2);
    }
}

//------------------------------------------------------------------------------
/**
\brief  DMA channel3 interrupt handler (memory -> SPI)

\ingroup module_serial
*/
//------------------------------------------------------------------------------
void DMA1_Channel3_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC3) == SET)
    {
        /* Disable the DMA channel */
        DMA_Cmd(DMA1_Channel3, DISABLE);

        /* Wait for the SPI core to finish transfer */
        while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET);
        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);

        /* Clear the interrupt flag */
        DMA_ClearFlag(DMA1_FLAG_TC3);
    }
    else if(DMA_GetITStatus(DMA1_IT_TE3) == SET)
    {
        /* Transfer error! */
        //TODO! handle this error?
        DMA_ClearFlag(DMA1_FLAG_TE3);
    }
}

/// \}
