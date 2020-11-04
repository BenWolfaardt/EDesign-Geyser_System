/*
 * user.c
 *
 * Parts of code taken from Lecturers code on SunLearn from first demo
 *
 *  Created on: 05 Mar 2018
 *      Author: 18321933
 */

#include "user.h"
#include "math.h"
#include "globals.h"

#define cmdBufL 50   		// maximum length of a command string received on the UART
#define maxTxL  50  		// maximum length of transmit buffer (replies sent back to UART)

//bool displayDelay2ms = 0;

char cmdBuf[cmdBufL];  		// buffer in which to store commands that are received from the UART
char uartRxChar;			// temporary storage
char txBuf[maxTxL]; 		// buffer for replies that are to be sent out on UART
char* txStudentNo = "$A,18321933\r\n";

char tempF[3];

//extern ADC_ChannelConfTypeDef adcChannel12;
//extern ADC_ChannelConfTypeDef adcChannel13;

extern TIM_HandleTypeDef htim2;

extern ADC_HandleTypeDef hadc1;

extern RTC_HandleTypeDef hrtc;

int i = 0;
int j = 0;

uint8_t my_counter;
int16_t tempSetpoint;		// the current temperature set point

uint8_t numberMap[10];
uint8_t pinsValue[4];
uint8_t segements[4];
//uint8_t pinsTemp[3];

uint16_t cmdBufPos;  		// this is the position in the cmdB where we are currently writing to

/*uint32_t adc12;
uint32_t adc13;
uint32_t adcBuf12;
uint32_t adcBuf13;
uint32_t measuredRMS12;
uint32_t vRMS12;
//uint32_t vRMS12;
uint32_t measuredRMS13;
//uint32_t iRMS13;
uint32_t iRMS13;*/

volatile bool uartRxFlag;	// use 'volatile' keyword because the variable is changed from interrupt handler


//volatile bool adctick;

//---------------------Prof code--------------------------//
void LedSet(uint16_t val);

volatile bool adcFlag;
uint8_t adcchan;
uint8_t samplectr;
#define RMS_WINDOW 40
float vrms_accum;
float irms_accum;
uint16_t isample[RMS_WINDOW];
uint16_t vsample[RMS_WINDOW];
uint32_t vrms_avg;
uint32_t irms_avg;
uint32_t vrms;
uint32_t irms;

uint32_t lasttick;

uint8_t digit;

uint32_t ambientT;
uint32_t waterT;
//---------------------Ben code--------------------------//
//volatile bool flowFlag;
volatile bool ms5Flag;
//bool lastFlowFlag;
volatile uint8_t ms5Counter;
uint32_t flowCounter;
uint32_t totalFlow;
uint8_t flowPulse;

int16_t heaterState;
int16_t valveState;

//bool flowValues[10000];

volatile bool flowHighFlag;
volatile bool firstHighFlag;

//--------------------Demo 4------------------------------------//

int16_t scheduleState;
int16_t heatingWindow;
RTC_TimeTypeDef onTime[3];
uint8_t HH_on;
uint8_t mm_on;
uint8_t ss_on;
RTC_TimeTypeDef offTime[3];
uint8_t HH_off;
uint8_t mm_off;
uint8_t ss_off;
volatile uint8_t timeL;

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

RTC_DateTypeDef getDateLive;
RTC_TimeTypeDef getTimeLive;

volatile HAL_StatusTypeDef halStatus;
//--------------------Demo 4------------------------------------//



uint8_t g_length = 0;

uint8_t in = 0;



void UserInitialise(void)
{
	uartRxFlag = false;
	tempSetpoint = 60;		// initial value

	//---------------------Prof code--------------------------//
	adcFlag = false;
	adcchan = 0;
	samplectr = 0;
	irms_accum = 0;
	vrms_accum = 0;

	digit = 0;
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

	segements[0] = 0b0001;
	segements[1] = 0b0010;
	segements[2] = 0b0100;
	segements[3] = 0b1000;

	pinsValue[0] = numberMap[1];
	pinsValue[1] = numberMap[8];
	pinsValue[2] = numberMap[5];
	pinsValue[3] = numberMap[2];

	HAL_UART_Receive_IT(&huart1, (uint8_t*)&uartRxChar, 1);	// UART interrupt after 1 character was received


	// start timer 2 for ADC sampling
	__HAL_TIM_ENABLE(&htim2);
	__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
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

		if (adcchan == 0)
		{
			vsample[samplectr] = HAL_ADC_GetValue(&hadc1);
		}
		else if (adcchan == 1)
		{
			isample[samplectr] = HAL_ADC_GetValue(&hadc1);
		}
		else if (adcchan == 2)
		{
			ambientT = HAL_ADC_GetValue(&hadc1);
		}
		else if (adcchan == 3)
		{
			waterT = HAL_ADC_GetValue(&hadc1);
		}

		adcchan++;
		if (adcchan >= 4)
		{
			adcchan = 0;
			samplectr++;

			if (samplectr >= RMS_WINDOW)
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
				samplectr = 0;
				/*				//float vrms_accum;
				//float irms_accum;
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
		switch (adcchan)
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

		//flowFlag = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10);

		ms5Counter++;
		if (ms5Counter >= 5)
		{
			ms5Counter = 0;
			ms5Flag = 1;
		}

		//LedUpdate();
		writeToPins(segements, pinsValue, g_length);
	}

	if (rtcSecFlag == 1) //------------1 second period
	{
		rtcSecFlag = 0;
		tick = 0;

		halStatus = HAL_RTC_GetTime(&hrtc, &getTimeLive, RTC_FORMAT_BCD);
		halStatus = HAL_RTC_GetDate(&hrtc, &getDateLive, RTC_FORMAT_BCD);
	}
}

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

void DecodeCmd()
{
	//---------------------Prof code--------------------------//
	//uint8_t numcharswritten;
	//---------------------Prof code--------------------------//

	uint8_t charsL;

	switch (cmdBuf[1])
	{
	case 'A' : //Student number
		//flowCounter = 0;	//-----------------------------------------------------------------------------------------------------------------flow counter remove
		HAL_UART_Transmit(&huart1, (uint8_t*)txStudentNo, 13, 1000);
		break;

	case 'B' : //Switch valve
		String2Int(cmdBuf+3, &valveState);

		switchValve();//----------------------------------------------------------default values

		txBuf[0] = '$';	txBuf[1] = 'B';
		txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);
		break;

	case 'C' : //Enable/ disable automatic schedule
		String2Int(cmdBuf+3, &scheduleState);//----------------------------------------------------------default values OFF
		//-----------------------------------------------------------------------------------------------------------------must i actualy code something?
		txBuf[0] = '$';	txBuf[1] = 'C';
		txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);
		break;

	case 'D' : //Switch heater
		String2Int(cmdBuf+3, &heaterState);

		switchHeater();

		txBuf[0] = '$';	txBuf[1] = 'D';
		txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);

		break;

	case 'E' : //Enable/disable logging to flash memory
		String2Int(cmdBuf+3, &valveState);
		//-----------------------------------------------------------------------------------------------------------------must i actualy code something?
		txBuf[0] = '$';	txBuf[1] = 'E';
		txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);
		break;

	case 'F': //Set Temperature
		String2Int(cmdBuf+3, &tempSetpoint);

		txBuf[0] = '$'; txBuf[1] = 'F';	txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);

		//LedSet(tempSetpoint);

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

		g_length = charsL;

		break;

	case 'G': //Get temperature

		txBuf[0] = '$';	txBuf[1] = 'G';	txBuf[2] = ',';
		charsL = Int2String(txBuf+3, tempSetpoint, 4);
		txBuf[3 + charsL] = '\r'; txBuf[4 + charsL] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, charsL+5, 1000);
		break;

	case 'H' : //Set time

		timeL = 0;

		timeL = StringTime2Int(cmdBuf+5, &YYYY_set);
		timeL = StringTime2Int(cmdBuf+5+timeL, &MM_set);
		timeL = StringTime2Int(cmdBuf+5+timeL, &DD_set);
		timeL = StringTime2Int(cmdBuf+5+timeL, &HH_set);
		timeL = StringTime2Int(cmdBuf+5+timeL, &mm_set);
		timeL = StringTime2Int(cmdBuf+5+timeL, &ss_set);

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
		txBuf[3] = '2';
		txBuf[4] = '0';
		charsL = 5;
		charsL += Int2String(txBuf+charsL, (uint32_t) getDate.Year, 2);
		txBuf[charsL] = ','; charsL++;
		charsL += Int2String(txBuf+charsL, (uint32_t) getDate.Month, 2);
		txBuf[charsL] = ','; charsL++;
		charsL += Int2String(txBuf+charsL, (uint32_t) getDate.Date, 2);
		txBuf[charsL] = ','; charsL++;
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
		HAL_RTC_SetTime(&hrtc, &onTime[heatingWindow-1], RTC_FORMAT_BCD);

		offTime[heatingWindow-1].Hours = HH_off;
		offTime[heatingWindow-1].Minutes = mm_off;
		offTime[heatingWindow-1].Seconds = ss_off;
		HAL_RTC_SetTime(&hrtc, &offTime[heatingWindow-1], RTC_FORMAT_BCD);

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
		String2Int(cmdBuf+3, &valveState);

		switchValve();

		txBuf[0] = '$';	txBuf[1] = 'B';
		txBuf[2] = '\r'; txBuf[3] = '\n';
		HAL_UART_Transmit(&huart1, (uint8_t*)txBuf, 4, 1000);
		break;
	}
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

void resetAll(void)
{
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET); //a
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET); //b
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET); //c
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET); //d
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET); //e
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET); //f
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_SET); //g
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
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,(~(segements[0] >> 0) & 0b00000001)); //1
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,(~(segements[0] >> 1) & 0b00000001)); //2
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,(~(segements[0] >> 2) & 0b00000001)); //3
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,(~(segements[0] >> 3) & 0b00000001)); //4

	}
	break;
	case 2:
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,(~(segements[1] >> 0) & 0b00000001)); //1
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,(~(segements[1] >> 1) & 0b00000001)); //2
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,(~(segements[1] >> 2) & 0b00000001)); //3
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,(~(segements[1] >> 3) & 0b00000001)); //4
	}
	break;
	case 3:
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,(~(segements[2] >> 0) & 0b00000001)); //1
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,(~(segements[2] >> 1) & 0b00000001)); //2
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,(~(segements[2] >> 2) & 0b00000001)); //3
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,(~(segements[2] >> 3) & 0b00000001)); //4
	}
	break;
	case 4:
	{
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,(~(segements[3] >> 0) & 0b00000001)); //1
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,(~(segements[3] >> 1) & 0b00000001)); //2
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,(~(segements[3] >> 2) & 0b00000001)); //3
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,(~(segements[3] >> 3) & 0b00000001)); //4
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

/*void LedSet(uint16_t val)
{
	//uint8_t charsL;


}*/
