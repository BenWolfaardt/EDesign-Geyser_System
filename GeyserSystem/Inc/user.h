/*
 * user.h
 *
 *  Created on: 11 May 2018
 *      Author: 18321933
 */

#ifndef USER_H_
#define USER_H_

#include <stdbool.h>
#include "main.h"
#include "stm32f3xx_hal.h"

// extern variable declarations - tell the compiler that these variables exist, and are defined elsewhere
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim2;
extern ADC_HandleTypeDef hadc1;
extern RTC_HandleTypeDef hrtc;
extern I2C_HandleTypeDef hi2c1;

void UserInitialise(void);
void Flags(void);

void Logging(void);

#endif /* USER_H_ */
