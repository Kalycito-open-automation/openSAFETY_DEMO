/**
********************************************************************************
\file   target/stm32f401re/app-gpio.c

\defgroup module_targ_stm32f401_app GPIO application module
\{

\brief  Implements a GPIO application for target stm32f401 (Cortex-M4)

This application simply reads inputs and outputs from common GPIO pins and
forwards it to the user application.

\ingroup group_app_targ_stm32f401
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
#include <common/app-gpio.h>
#include <stm32f4xx_hal_gpio.h>
#include <stm32f4xx_hal_rcc.h>
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
//#define STM32CUBE_F4_V1_4_0 	1
#define STM32CUBE_F4_V1_3_0		1
/*----------------------------------------------------------------------------*/
/* local types                                                                */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/* local vars                                                                 */
/*----------------------------------------------------------------------------*/
static UINT32 usedInputPins_l = 0;
static UINT32 usedOutputPins_l = 0;
static BOOL gpioClockWasActivated_l = 0;
/*----------------------------------------------------------------------------*/
/* local function prototypes                                                  */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*            P U B L I C   F U N C T I O N S                                 */
/*============================================================================*/

/*----------------------------------------------------------------------------*/
/**
\brief  Initialize the GPIO application

\retval 0       Init successful
\retval 1       Error on init
*/
/*----------------------------------------------------------------------------*/
UINT8 appgpio_init(void)
{
	GPIO_InitTypeDef inputInitSettings;
	GPIO_InitTypeDef outputInitSettings;

	/* enable clock for GPIO port C, if it isn't activated */
	if (!(RCC->AHB1ENR & RCC_AHB1ENR_GPIOCEN))
	{
#ifdef STM32CUBE_F4_V1_4_0
		__HAL_RCC_GPIOC_CLK_ENABLE();
#endif
#ifdef STM32CUBE_F4_V1_3_0
		__GPIOC_CLK_ENABLE();
#endif
	}
	else
	{
		gpioClockWasActivated_l = 1;
	}

	usedInputPins_l = 0;
	usedOutputPins_l = 0;

	inputInitSettings.Pin = GPIO_PIN_10 | GPIO_PIN_12;	/* Port C, MorphoConnector CN7: Pin 1 (PC10), Pin 3 (PC12) */
	inputInitSettings.Mode = GPIO_MODE_INPUT;
	inputInitSettings.Pull = GPIO_PULLDOWN;
	inputInitSettings.Speed = GPIO_SPEED_LOW; 			/* Ignored when pin is configured as input */
	inputInitSettings.Alternate = 0;

	usedInputPins_l |= inputInitSettings.Pin;

	outputInitSettings.Pin = GPIO_PIN_2 | GPIO_PIN_3;	/* Port C, MorphoConnector CN7: Pin 35 (PC2), Pin 37 (PC3) */
	outputInitSettings.Mode = GPIO_MODE_OUTPUT_PP;
	outputInitSettings.Pull = GPIO_NOPULL;
	outputInitSettings.Speed = GPIO_SPEED_LOW;
	outputInitSettings.Alternate = 0;

	usedOutputPins_l |= outputInitSettings.Pin;

	HAL_GPIO_Init (GPIOC, & inputInitSettings);
	HAL_GPIO_Init (GPIOC, & outputInitSettings);

    return 0;
}

/*----------------------------------------------------------------------------*/
/**
\brief  Cleanup the GPIO application
*/
/*----------------------------------------------------------------------------*/
void appgpio_exit(void)
{
	HAL_GPIO_DeInit(GPIOC, usedInputPins_l | usedOutputPins_l);

	/* disable clock for port, if it got activated in appgpio_init() */
	if (gpioClockWasActivated_l == 0)
	{
#ifdef STM32CUBE_F4_V1_4_0
		__HAL_RCC_GPIOC_CLK_DISABLE();
#endif

#ifdef STM32CUBE_F4_V1_3_0
		__GPIOC_CLK_DISABLE();
#endif
	}

	usedInputPins_l = 0;
	usedOutputPins_l = 0;
}


/*----------------------------------------------------------------------------*/
/**
\brief  Write a value to the output port

This function writes a value to the output port of the AP

\param[in] value_p       the value to write
*/
/*----------------------------------------------------------------------------*/
void appgpio_writeOutputPort(UINT32 value_p)
{
	UINT16 outputPin = 0x01;
	GPIO_PinState pinState = GPIO_PIN_RESET;

	/* process value for output ports */
	if (usedOutputPins_l != 0)
	{
		while (outputPin)
		{
			if (outputPin & usedOutputPins_l)
			{
				pinState = (value_p & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET;
				HAL_GPIO_WritePin(GPIOC, outputPin, pinState);
				value_p = value_p >> 1;
			}

			outputPin = (outputPin << 1)  & GPIO_PIN_MASK;
		}
	}

}

/*----------------------------------------------------------------------------*/
/**
\brief  Read a value from the input port

This function reads a value from the input port of the AP

\return  UINT32
\retval  value              the value of the input port
*/
/*----------------------------------------------------------------------------*/
UINT8 appgpio_readInputPort(void)
{
    UINT8 val = 0;
    UINT8 valPosition = 0x01;
	UINT16 inputPin = 0x01;
	GPIO_PinState pinState = GPIO_PIN_RESET;

	if (usedInputPins_l != 0)
	{
		while (inputPin)
		{
			if (inputPin & usedInputPins_l)
			{
				pinState = HAL_GPIO_ReadPin(GPIOC, inputPin);

				if (pinState == GPIO_PIN_SET)
				{
					val = val | valPosition;
				}

				valPosition = valPosition << 1;
			}
			inputPin = (inputPin << 1)  & GPIO_PIN_MASK;
		}
	}

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

