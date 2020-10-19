/*
 * user.c
 *
 * Parts of code taken from Lecturers code on SunLearn from first demo
 *
 *  Created on: 05 Mar 2018
 *      Author: 18321933
 */

#include "user.h"

#define cmdBufL 50   		// maximum length of a command string received on the UART
#define maxTxL  50  		// maximum length of transmit buffer (replies sent back to UART)

volatile bool uartRxFlag;	// use 'volatile' keyword because the variable is changed from interrupt handler

char uartRxChar;			// temporary storage
char cmdBuf[cmdBufL];  		// buffer in which to store commands that are received from the UART
char txBuf[maxTxL]; 		// buffer for replies that are to be sent out on UART

char* txStudentNo = "$A,18321933\r\n";

int16_t tempSetpoint;		// the current temperature set point

uint16_t cmdBufPos;  		// this is the position in the cmdB where we are currently writing to

void UserInitialise(void)
{
	uartRxFlag = false;
	tempSetpoint = 60;		// initial value

	HAL_UART_Receive_IT(&huart1, (uint8_t*)&uartRxChar, 1);	// UART interrupt after 1 character was received
}

void User(void)
{
	if (uartRxFlag)
	{
		if (uartRxChar == '$')
			cmdBufPos = 0;

		// add character to command buffer, but only if there is more space in the command buffer
		if (cmdBufPos < cmdBufL)
			cmdBuf[cmdBufPos++] = uartRxChar;

		if ((cmdBufPos >= 4) && (cmdBuf[0] == '$') && (cmdBuf[cmdBufPos-2] == '\r') && (cmdBuf[cmdBufPos-1] == '\n'))
		{
			DecodeCmd();
			cmdBufPos = 0;	// clear buffer
		}
		uartRxFlag = false;  // clear the flag - the 'receive character' event has been handled.
		HAL_UART_Receive_IT(&huart1, (uint8_t*)&uartRxChar, 1);	// UART interrupt after 1 character was received
	}
}

void DecodeCmd()
{
	uint8_t charsL;

	switch (cmdBuf[1])
	{
	case 'A' :
		HAL_UART_Transmit(&huart1, (uint8_t*)txStudentNo, 13, 1000);
		break;

	case 'F':
		String2Int(cmdBuf+3, &tempSetpoint);

		txBuf[0] = '$';
		txBuf[1] = 'F';
		txBuf[2] = '\r';
		txBuf[3] = '\n';

		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);
		break;

	case 'G':
		txBuf[0] = '$';
		txBuf[1] = 'G';
		txBuf[2] = ',';
		charsL = Int2String(txBuf+3, tempSetpoint, 4);
		txBuf[3 + charsL] = '\r';
		txBuf[4 + charsL] = '\n';

		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, charsL+5, 1000);
		break;

	case 'T':
	  	//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET); //1
	  	//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET); //2
	  	//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET); //3
		//HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET); //4




//		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5, 1); //a
//		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6, 1); //b
//		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7, 1); //c
//		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6, 1); //d
//		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7, 1); //e
//		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, 1); //f
//		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9, 1); //g

		break;
	}
}

// HAL_UART_RxCpltCallback - callback function that will be called from the UART interrupt handler.
// This function will execute whenever a character is received from the UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
	// the interrupt handler will automatically put the received character in the uartRXChar variable (no need to write any code for that).
	// so all we do it set flag to indicate character was received, and then process the received character further in the main loop
	uartRxFlag = true;
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
uint8_t Int2String(char* outputString, int16_t value, uint8_t maxL)
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
	else
		digits = 5;

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

void resetAll(void)
{
	  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET); //1
	  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET); //2
	  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET); //3
	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_RESET); //4
	  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_RESET); //5
	  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET); //6
	  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_RESET); //7
}
