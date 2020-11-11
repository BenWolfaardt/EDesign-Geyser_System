/*
 * globals.h
 *
 *  Created on: 11 May 2018
 *      Author: 18321933
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "stdbool.h"

//Volatile keyword because the variable is changed from interrupt handler
//Flags
volatile bool uartRxFlag;
volatile bool adcFlag;
volatile bool flowHighFlag;
volatile bool firstHighFlag;
volatile bool rtcSecFlag;volatile bool i2cTxFlag;
//volatile bool i2cErFlag;

//Variables
volatile int16_t tempSetpoint;

//-------------------------Control States--------------------//
volatile int16_t valveState;
volatile int16_t heaterState;
volatile int16_t scheduleState;
//-------------------------

uint8_t in; //7-segments
uint8_t segementsSet[4]; //7 segments

volatile uint8_t timeL; //time RTC

int i, j;


#endif /* GLOBALS_H_ */
