/*
 * user.c
 *
 *  Created on: 11 May 2018
 *      Author: 18321933
 */

#include "user.h"
#include "math.h"
#include "globals.h"
#include "functions.h"

//-------------------------UART-----------------------------//
#define cmdBufL 60   		// maximum length of a command string received on the UART
#define maxTxL  60  		// maximum length of transmit buffer (replies sent back to UART)

char cmdBuf[cmdBufL];  		// buffer in which to store commands that are received from the UART
char uartRxChar;			// temporary storage
char txBuf[maxTxL]; 		// buffer for replies that are to be sent out on UART

char* txStudentNo = "$A,18321933\r\n";

uint16_t cmdBufPos;  		// this is the position in the cmdB where we are currently writing to

//-------------------------Nested Flags---------------------//
//volatile bool ms3Flag;
volatile uint16_t s1Counter;
volatile bool ms5Flag;
volatile uint8_t ms5Counter;

uint32_t lasttick;

//-------------------------7-Segment-------------------------//
char tempF[3];

uint8_t numberMap[10];
uint8_t pinsValue[4];
uint8_t segmentsL = 0;



//-------------------------ADC Variables---------------------//
#define RMS_WINDOW 40

float vrmsSum;	//----------------------------------------floats or uints???
float irmsSum;

uint8_t adcCh;
uint8_t sampleCntr;

uint16_t isample[RMS_WINDOW];
uint16_t vsample[RMS_WINDOW];
uint32_t vrms_avg;
uint32_t irms_avg;
uint32_t vrms;
uint32_t irms;
uint32_t ambientT;
uint32_t waterT;

//-------------------------Flow Variables---------------------//
uint32_t flowCounter;
uint32_t totalFlow;
uint8_t flowPulse;

//-------------------------RTC Variables----------------------//
volatile HAL_StatusTypeDef halStatus;

RTC_DateTypeDef getDateLive;
RTC_TimeTypeDef getTimeLive;

int16_t heatingWindow;

RTC_TimeTypeDef onTime[3];
uint8_t HH_on;
uint8_t mm_on;
uint8_t ss_on;
RTC_TimeTypeDef offTime[3];
uint8_t HH_off;
uint8_t mm_off;
uint8_t ss_off;
//volatile uint8_t timeL;

RTC_DateTypeDef setDate;
uint8_t YYYY_set;
uint8_t MM_set;
uint8_t DD_set;
RTC_TimeTypeDef setTime;
uint8_t HH_set;
uint8_t mm_set;
uint8_t ss_set;
RTC_DateTypeDef getDate;
uint8_t YYYY_get;
uint8_t MM_get;
uint8_t DD_get;
RTC_TimeTypeDef getTime;
uint8_t HH_get;
uint8_t mm_get;
uint8_t ss_get;


void UserInitialise(void)
{
	uartRxFlag = false;
	tempSetpoint = 60;		// initial value

	//---------------------Prof code--------------------------//
	adcFlag = false;
	adcCh = 0;
	sampleCntr = 0;
	irmsSum = 0;
	vrmsSum = 0;

	//digit = 0;
	//---------------------Prof code--------------------------//

	valveState = 0;
	heaterState = 0;
	scheduleState = 0;

	numberMap[0] = 0b00111111;
	numberMap[1] = 0b00000110;
	numberMap[2] = 0b01011011;
	numberMap[3] = 0b01001111;
	numberMap[4] = 0b01100110;
	numberMap[5] = 0b01101101;
	numberMap[6] = 0b01111101;
	numberMap[7] = 0b00000111;
	numberMap[8] = 0b01111111;
	numberMap[9] = 0b01100111;

	segementsSet[0] = 0b0001;
	segementsSet[1] = 0b0010;
	segementsSet[2] = 0b0100;
	segementsSet[3] = 0b1000;

	pinsValue[0] = numberMap[8];
	pinsValue[1] = numberMap[8];
	pinsValue[2] = numberMap[8];
	pinsValue[3] = numberMap[8];

	segmentsL = 4;

	HAL_UART_Receive_IT(&huart1, (uint8_t*)&uartRxChar, 1);	// UART interrupt after 1 character was received


	// start timer 2 for ADC sampling
	__HAL_TIM_ENABLE(&htim2);
	__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);

	HAL_I2C_Init(&hi2c1);

	//HAL_I2C_Master_Transmit(&hi2c1, 0x45<<1, &buffer[0], 1, 100);

}

void DecodeCmd()
{
	//uint8_t charsL;

	switch (cmdBuf[1])
	{
	case 'A' : //Student number
		//flowCounter = 0;	//-----------------------------------------------------------------------------------------------------------------flow counter remove
		HAL_UART_Transmit(&huart1, (uint8_t*)txStudentNo, 13, 1000);
		break;

	case 'B' : //Switch valve
		String2Int(cmdBuf+3, (int16_t*) &valveState);

		switchValve();//----------------------------------------------------------default values

		txBuf[0] = '$';	txBuf[1] = 'B';
		txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);
		break;

	case 'C' : //Enable/ disable automatic schedule
		String2Int(cmdBuf+3, (int16_t*) &scheduleState);//----------------------------------------------------------default values OFF

		txBuf[0] = '$';	txBuf[1] = 'C';
		txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);
		break;

	case 'D' : //Switch heater
		String2Int(cmdBuf+3, (int16_t*) &heaterState);

		switchHeater();

		txBuf[0] = '$';	txBuf[1] = 'D';
		txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);

		break;

	case 'E' : //Enable/disable logging to flash memory
		String2Int(cmdBuf+3, (int16_t*) &valveState);
		//-----------------------------------------------------------------------------------------------------------------must i actualy code something?
		txBuf[0] = '$';	txBuf[1] = 'E';
		txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);
		break;

	case 'F': //Set Temperature
		String2Int(cmdBuf+3, (int16_t*) &tempSetpoint);

		txBuf[0] = '$'; txBuf[1] = 'F';	txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);

		i = 0;

		charsL = Int2String(tempF, tempSetpoint, 4);

		while (i < charsL)
		{
			for (j=0; j <10; j++)
			{
				if (tempF[i] == (j+0x30))
				{
					pinsValue[i] = numberMap[j];
					j = 10;
				}
			}
			i++;
		}
		i = 0;

		segmentsL = charsL;

		break;

	case 'G': //Get temperature

		txBuf[0] = '$';	txBuf[1] = 'G';	txBuf[2] = ',';
		charsL = Int2String(txBuf+3, tempSetpoint, 4);
		txBuf[3 + charsL] = '\r'; txBuf[4 + charsL] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, charsL+5, 1000);
		break;

	case 'H' : //Set time

		timeL = 0;

//		timeL = StringTime2Int(cmdBuf+5, &YYYY_set);
//		timeL = StringTime2Int(cmdBuf+5+timeL, &MM_set);
//		timeL = StringTime2Int(cmdBuf+5+timeL, &DD_set);
		timeL = StringTime2Int(cmdBuf+3+timeL, &HH_set);
		timeL = StringTime2Int(cmdBuf+3+timeL, &mm_set);
		timeL = StringTime2Int(cmdBuf+3+timeL, &ss_set);

		setDate.Year = YYYY_set;
		setDate.Month = MM_set;
		setDate.Date = DD_set;
		setTime.Hours = HH_set;
		setTime.Minutes = mm_set;
		setTime.Seconds = ss_set;

		//Update the Calendar (cancel write protection and enter init mode)
		__HAL_RTC_WRITEPROTECTION_DISABLE(&hrtc); // Disable write protection
		halStatus = RTC_EnterInitMode(&hrtc); // Enter init mode
		halStatus = HAL_RTC_SetTime(&hrtc, &setTime, RTC_FORMAT_BCD);
		halStatus = HAL_RTC_SetDate(&hrtc, &setDate, RTC_FORMAT_BCD);
		__HAL_RTC_WRITEPROTECTION_ENABLE(&hrtc);

		txBuf[0] = '$';	txBuf[1] = 'H';
		txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);
		break;

	case 'I' : //Get time

		//		halStatus = HAL_RTC_GetTime(&hrtc, &getTime, RTC_FORMAT_BCD);
		//		halStatus = HAL_RTC_GetDate(&hrtc, &getDate, RTC_FORMAT_BCD);

		getTime = getTimeLive;
		getDate = getDateLive;

		txBuf[0] = '$';	txBuf[1] = 'I';
		txBuf[2] = ',';
		charsL = 3;
//		charsL += Int2String(txBuf+charsL, (uint32_t) getDate.Year, 2);
//		txBuf[charsL] = ','; charsL++;
//		charsL += Int2String(txBuf+charsL, (uint32_t) getDate.Month, 2);
//		txBuf[charsL] = ','; charsL++;
//		charsL += Int2String(txBuf+charsL, (uint32_t) getDate.Date, 2);
//		txBuf[charsL] = ','; charsL++;
		charsL += Int2String(txBuf+charsL, (uint32_t) getTime.Hours, 2);
		txBuf[charsL] = ','; charsL++;
		charsL += Int2String(txBuf+charsL, (uint32_t) getTime.Minutes, 2);
		txBuf[charsL] = ','; charsL++;
		charsL += Int2String(txBuf+charsL, (uint32_t) getTime.Seconds, 2);
		txBuf[charsL] = '\r'; charsL++; txBuf[charsL] = '\n'; charsL++;
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, charsL, 1000);
		break;

	case 'J' : //Set heating schedule
		//In order to set a heating schedule for the 2nd schedule window, to start at 8:30 AM and end at 10AM, the
		//test station will issue a command
		//$J,2,8,30,0,10,0,0<CR><LF>

		String2Int(cmdBuf+3, &heatingWindow);

		timeL = 0;

		timeL = StringTime2Int(cmdBuf+5, &HH_on);
		timeL = StringTime2Int(cmdBuf+5+timeL, &mm_on);
		timeL = StringTime2Int(cmdBuf+5+timeL, &ss_on);
		timeL = StringTime2Int(cmdBuf+5+timeL, &HH_off);
		timeL = StringTime2Int(cmdBuf+5+timeL, &mm_off);
		timeL = StringTime2Int(cmdBuf+5+timeL, &ss_off);

		onTime[heatingWindow-1].Hours = HH_on;
		onTime[heatingWindow-1].Minutes = mm_on;
		onTime[heatingWindow-1].Seconds = ss_on;
		//HAL_RTC_SetTime(&hrtc, &onTime[heatingWindow-1], RTC_FORMAT_BCD);

		offTime[heatingWindow-1].Hours = HH_off;
		offTime[heatingWindow-1].Minutes = mm_off;
		offTime[heatingWindow-1].Seconds = ss_off;
		//HAL_RTC_SetTime(&hrtc, &offTime[heatingWindow-1], RTC_FORMAT_BCD);

		txBuf[0] = '$';	txBuf[1] = 'J';
		txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);
		break;

	case 'K': //Request telemetry
		// return string with format $K,1234,220000,25,66,567800,OFF,OPEN<CR><LF>
		txBuf[0] = '$'; txBuf[1] = 'K'; txBuf[2] = ',';
		charsL = 3;
		charsL += Int2String(txBuf+charsL, irms, 10);
		txBuf[charsL] = ','; charsL++;
		charsL += Int2String(txBuf+charsL, vrms, 10);
		txBuf[charsL] = ','; charsL++;
		charsL += Int2String(txBuf+charsL, TempConv(ambientT), 10);    // temp ambient
		txBuf[charsL] = ','; charsL++;
		charsL += Int2String(txBuf+charsL, TempConv(waterT), 10);    // temp water
		txBuf[charsL] = ','; charsL++;
		charsL += Int2String(txBuf+charsL, totalFlow, 10);    // flow totalFlow
		txBuf[charsL] = ','; charsL++;

		if (heaterState == 0U)
		{
			txBuf[charsL] = 'O'; charsL++;
			txBuf[charsL] = 'F'; charsL++;
			txBuf[charsL] = 'F'; charsL++;
			txBuf[charsL] = ','; charsL++;
		}
		else if(heaterState==1U)
		{
			txBuf[charsL] = 'O'; charsL++;
			txBuf[charsL] = 'N'; charsL++;
			txBuf[charsL] = ','; charsL++;
		}
		if(valveState==0U)
		{
			txBuf[charsL] = 'C'; charsL++;
			txBuf[charsL] = 'L'; charsL++;
			txBuf[charsL] = 'O'; charsL++;
			txBuf[charsL] = 'S'; charsL++;
			txBuf[charsL] = 'E'; charsL++;
			txBuf[charsL] = 'D'; charsL++;
		}
		else if(valveState==1U)
		{
			txBuf[charsL] = 'O'; charsL++;
			txBuf[charsL] = 'P'; charsL++;
			txBuf[charsL] = 'E'; charsL++;
			txBuf[charsL] = 'N'; charsL++;
		}

		txBuf[charsL] = '\r'; charsL++; txBuf[charsL] = '\n'; charsL++;
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, charsL, 1000);

		break;

	case 'L' : //Request log entry
		String2Int(cmdBuf+3, (int16_t*) &valveState);

		switchValve();

		txBuf[0] = '$';	txBuf[1] = 'B';
		txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);
		break;
	}
}

void Flags(void)
{
	//halStatus = HAL_I2C_Master_Transmit_IT(&hi2c1, 0x44<<1, &pData, 1);	// I2C write call

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
	if(adcFlag == 1U)
	{
		// this code branch will execute every 250ns...
		// the adcFlag variable will get set in the 250ns timer interrupt handler.
		//
		// use this branch to sample one ADC value.
		// 1. use HAL_ADC_GetValue() to retrieve the last sampled ADC value.
		// 2. process the value as needed
		// 3. change the ADC channel
		// 4. start a new ADC sampling iteration
		//
		// the result is that all ADC 4 channels are sampled every 1ms

		if (adcCh == 0)
		{
			vsample[sampleCntr] = HAL_ADC_GetValue(&hadc1);
		}
		else if (adcCh == 1)
		{
			isample[sampleCntr] = HAL_ADC_GetValue(&hadc1);
		}
		else if (adcCh == 2)
		{
			ambientT = HAL_ADC_GetValue(&hadc1);
		}
		else if (adcCh == 3)
		{
			waterT = HAL_ADC_GetValue(&hadc1);
		}

		adcCh++;
		if (adcCh >= 4)
		{
			adcCh = 0;
			sampleCntr++;

			if (sampleCntr >= RMS_WINDOW)
			{
				/*				vsample *= vsample;
				temp12 += vsample;
				temp12/=RMS_WINDOW;
				temp12 = sqrt(temp12);
				temp12*=3350;
				temp12/=4095;
				measuredRMS12 = temp12;
				//vRMS12 = measuredRMS12*4.679287305;
				vrms = measuredRMS12*4.679287305;
				adcBuf12 = 0;

				//iRMS13 = measuredRMS12*84.97807018;
				irms = measuredRMS12*84.97807018;
				adcBuf13 = 0;*/
				sampleCntr = 0;
				/*				//float vrmsSum;
				//float irmsSum;
				//uint16_t isample[RMS_WINDOW];
				//uint16_t vsample[RMS_WINDOW];
				//uint32_t vrms_avg;
				//uint32_t irms_avg;
				//uint32_t vrms;
				//uint32_t irms;
			}
			else
			{
				vsample *= vsample;
				temp12 += adc12;*/
			}
		}

		ADC_ChannelConfTypeDef chdef;
		switch (adcCh)
		{
		case 0: chdef.Channel = ADC_CHANNEL_12; break;  //V				//PB1
		case 1: chdef.Channel = ADC_CHANNEL_13; break;  //I				//PB13
		case 2: chdef.Channel = ADC_CHANNEL_8; break; //temp ambient	//PC2
		case 3: chdef.Channel = ADC_CHANNEL_9; break; //temp water		//PC3
		}

		chdef.Rank = 1;
		chdef.SingleDiff = ADC_SINGLE_ENDED;
		chdef.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
		chdef.OffsetNumber = ADC_OFFSET_NONE;
		chdef.Offset = 0;
		HAL_ADC_ConfigChannel(&hadc1, &chdef);

		HAL_ADC_Start(&hadc1);

		adcFlag = 0;
	}

	if (flowHighFlag == 1)
	{
		if (firstHighFlag == 1)
		{
			firstHighFlag = 0;
			ms5Counter = 0;
			ms5Flag = 0;
			flowPulse = 0;
		}
		if (ms5Flag == 1)
		{
			ms5Flag = 0;

			flowPulse++;
			if (flowPulse == 1)
			{
				flowCounter++;
				totalFlow = 100*flowCounter;
			}
		}
	}

	// 1ms timer
	uint32_t tick = HAL_GetTick();
	if (tick != lasttick)
	{
		lasttick = tick;

		ms5Counter++;
		if (ms5Counter >= 5)
		{
			ms5Counter = 0;
			ms5Flag = 1;
		}

		s1Counter++;
		if (s1Counter >= 1000)
		{
			s1Counter = 0;

			halStatus = HAL_RTC_GetTime(&hrtc, &getTimeLive, RTC_FORMAT_BCD);
			halStatus = HAL_RTC_GetDate(&hrtc, &getDateLive, RTC_FORMAT_BCD);

			if (scheduleState == 1)
			{
				i = 0;
				while (i < 3)
				{
					if (getTimeLive.Hours >=  onTime[i].Hours && getTimeLive.Hours <=  offTime[i].Hours)
					{
						if (getTimeLive.Minutes >=  onTime[i].Minutes && getTimeLive.Minutes <=  offTime[i].Minutes)
						{
							if (getTimeLive.Seconds >=  onTime[i].Seconds && getTimeLive.Seconds <=  offTime[i].Seconds)
							{
								heaterState = 1;
								i = 4;
							}
						}
					}
					else
						heaterState = 0;
					i++;
				}
				switchHeater();
			}
		}

		writeToPins(segementsSet, pinsValue, segmentsL, i);
		i++;

		if (i == segmentsL)
			i = 0;
	}

	//	if (rtcSecFlag == 1) //------------1 second period
	//	{
	//		rtcSecFlag = 0;
	//		tick = 0;
	//
	//		halStatus = HAL_RTC_GetTime(&hrtc, &getTimeLive, RTC_FORMAT_BCD);
	//		halStatus = HAL_RTC_GetDate(&hrtc, &getDateLive, RTC_FORMAT_BCD);
	//	}

	//		buffer[0] = 0x00;


	//halStatus = HAL_I2C_Master_Transmit_IT(&hi2c1, 0x45<<1, &buffer[0], 1);	// I2C write call


	//		HAL_I2C_Mem_Read(&hi2c1, 0x44>>1, 0x00, 2, &buffer[0], 4, 100);


	//HAL_I2C_Master_Transmit(&hi2c1, 0x45<<1, &buffer[0], 1, 100); //45 rotary slider
	//HAL_I2C_Master_Receive(&hi2c1, 0x45<<1, &buffer[1], 3, 100);
	//HAL_I2C_Master_Transmit(&hi2c1, 0x45<<1, buffer, 4, 100); //45 rotary slider
	//HAL_I2C_Master_Receive(&hi2c1, 0x45<<1, &buffer[0], 1, 100);


	if (i2cTxFlag)	//Now process the interrupt call-back
	{
		i2cTxFlag = 0;


		//buffer[0] = 0x45>>1 | 0;//control byte
		//ACK
		//buffer[1] = 0; //MSB
		//ACK
		//buffer[2] = 0; //LSB

		//HAL_I2C_Master_Receive(&hi2c1, 0x45<<1, &buffer[3], 1, 100);

		//		//S - start
		//		buffer[0] = ; //Control byte + WRITE bit --- Control byte 7 bit (slave) address then 1 bit read/ write
		//		//ACK
		//		buffer[1] = 0x28;//address command
		//		//ACK
		//		buffer[2] = 0; //MSB
		//		//ACK
		//		buffer[3] = 0; //LSB
		//		//ACK
		//		//S - stop
		//
		//		//RDY set LOW
		//
		//		HAL_I2C_Master_Transmit(&hi2c1, 0x45<<1, buffer, 4, 100); //45 rotary slider

		//HAL_Delay(20);

		//S - start
		/*buffer[0] = 0x45>>1 | 0; //Control byte + WRITE bit------------------------ or 0 ----------------------
		//ACK
		buffer[1] = 0x03;//address command
		//ACK
		//S - Start
		buffer[2] =  0x45>>1 | 1; //Control byte + READ bit
		//ACK
		buffer[3] = 0; //MSB
		//NACK
		//S - stop

		//RDY set LOW

		HAL_I2C_Master_Transmit(&hi2c1, 0x45<<1, &buffer[0], 1, 100);
		HAL_Delay(30);
		HAL_I2C_Master_Transmit(&hi2c1, 0x45<<1, &buffer[1], 1, 100);
		HAL_Delay(30);
		HAL_I2C_Master_Transmit(&hi2c1, 0x45<<1, &buffer[2], 1, 100);
		HAL_Delay(30);
		HAL_I2C_Master_Receive(&hi2c1, 0x45<<1, &buffer[3], 1, 100);
		HAL_Delay(30);*/

		//float value = buffer[0]<<8 | buffer[1]; //combine 2 8-bit into 1 16-bit

		//HAL_Delay(100);
		//HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);
	}
}



