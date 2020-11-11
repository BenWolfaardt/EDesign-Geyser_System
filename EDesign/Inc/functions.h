/*
 * functions.h
 *
 *  Created on: 11 May 2018
 *      Author: 18321933
 */

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include "stm32f3xx_hal.h"

uint32_t TempConv(uint32_t tempVal);
uint32_t Int2String(char* outputString, int32_t value, uint8_t maxL);

uint8_t String2Int(char* inputString, int16_t* outputInt);
uint8_t StringTime2Int(char* inputTimeString, uint8_t* outputTimeInt);

void writeToPins(uint8_t segments[], uint8_t pins[], int segmentsL);
void switchHeater(void);
void switchValve(void);

#endif /* FUNCTIONS_H_ */
