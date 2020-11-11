/*
 * interrupts.c
 *
 *  Created on: 12 May 2018
 *      Author: 18321933
 */

#include "interrupts.h"
#include "globals.h"

//----------------------------------------------------------------------------------------------//
//											Interupts											//
//----------------------------------------------------------------------------------------------//

// HAL_UART_RxCpltCallback - callback function that will be called from the UART interrupt handler.
// This function will execute whenever a character is received from the UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
	// the interrupt handler will automatically put the received character in the uartRXChar variable (no need to write any code for that).
	// so all we do it set flag to indicate character was received, and then process the received character further in the main loop
	uartRxFlag = true;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) //every 250 ns
{
	if (htim == &htim2)
		adcFlag = true;

}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	flowHighFlag = 1;
	firstHighFlag = 1;
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
	rtcSecFlag = 1;
}

