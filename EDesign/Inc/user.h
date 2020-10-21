/*
 * user.h
 *
 * Parts of code taken from Lecturers code on SunLearn from first demo
 *
 *  Created on: 05 Mar 2018
 *      Author: 18321933
 */

#ifndef USER_H_
#define USER_H_

#include <stdbool.h>
#include "main.h"
#include "stm32f3xx_hal.h"

// extern variable declarations - tell the compiler that these variables exist, and are defined elsewhere
extern UART_HandleTypeDef huart1;

// function prototypes

void DecodeCmd(void);

void UserInitialise(void);
void User(void);
void resetAll(void);
void writeToPins(uint8_t segments[], uint8_t pins[], int segmentsL);

uint8_t Int2String(char* outputString, int16_t value, uint8_t maxL);
uint8_t String2Int(char* inputString, int16_t* outputInt);


#endif /* USER_H_ */
