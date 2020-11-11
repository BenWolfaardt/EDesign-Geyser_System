/*
 * functions.c
 *
 *  Created on: 11 May 2018
 *      Author: 18321933
 */

#include "functions.h"
#include "globals.h"

uint32_t TempConv(uint32_t tempVal)
{
	uint32_t temp = 0;
	uint32_t scale = 0;

	temp = ((100*tempVal)>>7)+((100*tempVal)>>12);
	scale = (temp-500)/10;

	return scale;
}

void switchHeater(void)
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,heaterState);
}

void switchValve(void)
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,valveState);
}

uint8_t StringTime2Int(char* inputTimeString, uint8_t* outputTimeInt)
{
	int returnValue = 0;
	timeL += 1;

	if (*inputTimeString == ',')
		return 0;

	while ((*inputTimeString >= '0') && (*inputTimeString <= '9'))
	{
		returnValue *= 10;
		returnValue += (*inputTimeString - 48);

		if (returnValue >= 128)
			return 0;

		inputTimeString++;
		timeL++;
	}
	*outputTimeInt = (int8_t)(returnValue);
	return timeL;
}

uint8_t String2Int(char* inputString, int16_t* outputInt)
{
	int returnValue = 0;
	int sign = 1;

	if (*inputString == '\0')
		return 0;

	if (*inputString == '-')
	{
		sign = -1;
		inputString++;
	}

	while ((*inputString >= '0') && (*inputString <= '9'))
	{
		returnValue *= 10;
		returnValue += (*inputString - 48);

		if (((sign == 1) && (returnValue >= 32768)) ||
				((sign == -1) && (returnValue >= 32769)))
			return 0;

		inputString++;
	}
	*outputInt = (int16_t)(sign * returnValue);
	return 1;
}

// convert integer var to ASCII string
uint32_t Int2String(char* outputString, int32_t value, uint8_t maxL)
{
	int numWritten = 0;
	int writePosition = 0;
	uint8_t digits = 0;

	if (maxL == 0)
		return 0;

	if (value < 0)
	{
		outputString[0] = '-';
		outputString++;
		maxL--;
		value = -value;
		numWritten = 1;
	}

	if (value < 10)
		digits = 1;
	else if (value < 100)
		digits = 2;
	else if (value < 1000)
		digits = 3;
	else if (value < 10000)
		digits = 4;
	else if (value < 100000)
		digits = 5;
	else if (value < 1000000)
		digits = 6;
	else
		digits = 7;

	if (digits > maxL)
		return 0; // error - not enough space in output string!

	writePosition = digits;
	while (writePosition > 0)
	{
		outputString[writePosition-1] = (char) ((value % 10) + 48);
		value /= 10;
		writePosition--;
		numWritten++;
	}

	return numWritten;
}

void writeToPins(uint8_t segments[], uint8_t pins[], int segmentsL)
{

	if(in == segmentsL)
	{
		in = 0;
	}
	else
	{
		in++;
	}

	switch(in)
	{
	case 1 :
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,(~(segments[0] >> 0) & 0b00000001)); //1
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,(~(segments[0] >> 1) & 0b00000001)); //2
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,(~(segments[0] >> 2) & 0b00000001)); //3
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,(~(segments[0] >> 3) & 0b00000001)); //4

	}
	break;
	case 2:
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,(~(segments[1] >> 0) & 0b00000001)); //1
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,(~(segments[1] >> 1) & 0b00000001)); //2
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,(~(segments[1] >> 2) & 0b00000001)); //3
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,(~(segments[1] >> 3) & 0b00000001)); //4
	}
	break;
	case 3:
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,(~(segments[2] >> 0) & 0b00000001)); //1
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,(~(segments[2] >> 1) & 0b00000001)); //2
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,(~(segments[2] >> 2) & 0b00000001)); //3
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,(~(segments[2] >> 3) & 0b00000001)); //4
	}
	break;
	case 4:
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,(~(segments[3] >> 0) & 0b00000001)); //1
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,(~(segments[3] >> 1) & 0b00000001)); //2
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,(~(segments[3] >> 2) & 0b00000001)); //3
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,(~(segments[3] >> 3) & 0b00000001)); //4
	}
	break;
	}

	i = (int)(in - 1);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5, (~(pins[i] >> 0) & 0b00000001)); //a
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6, (~(pins[i] >> 1) & 0b00000001)); //b
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7, (~(pins[i] >> 2) & 0b00000001)); //c
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6, (~(pins[i] >> 3) & 0b00000001)); //d
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7, (~(pins[i] >> 4) & 0b00000001)); //e
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, (~(pins[i] >> 5) & 0b00000001)); //f
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9, (~(pins[i] >> 6) & 0b00000001)); //g

}
