#include "ds3231.h"


void ds3231StartOsc(void)
{
//	unsigned char value;
//	i2cRd(RTC_ADR,RTC_CR_ADR,&value,1);
//	value&=RTC_OSC_START;
//	i2c1Write(RTC_ADR,RTC_CR_ADR,&value,1);
}
void ds3231StopOsc(void)
{
	//unsigned char value;
//	i2cRd(RTC_ADR,RTC_CR_ADR,&value,1);
//	value|=RTC_OSC_STOP;
//	i2c1Write(RTC_ADR,RTC_CR_ADR,&value,1);
}

void ds3231GetDate(unsigned char* dateBuffer)
{
//	if(!dateBuffer)
//		return;
//	i2cRd(RTC_ADR,RTC_WDAY_ADR,dateBuffer,4);
}

void ds3231GetTime(unsigned char* timeBuffer)
{
//	if(!timeBuffer)
//		return;
//	timeBuffer[1] = 1;
//	timeBuffer[2]= 2; 
//	i2cRd(RTC_ADR,RTC_SEC_ADR,timeBuffer,3);
}

void ds3231SetDate(unsigned char* dateBuffer)
{
	if(!dateBuffer)
		return;
	//I2C1->CR1|=I2C_CR1_PE;
	i2c1Write(RTC_ADR,RTC_WDAY_ADR,dateBuffer,4);
	//I2C1->CR1&=~I2C_CR1_PE;
}

void ds3231SetTime(unsigned char* timeBuffer)
{
	if(!timeBuffer)
		return;
	//I2C1->CR1|=I2C_CR1_PE;
	i2c1Write(RTC_ADR,RTC_SEC_ADR,timeBuffer,3);
//	I2C1->CR1&=~I2C_CR1_PE;
}

void ds3231GetTemp(char* tempBuffer)
{
//	if(!tempBuffer)
	//	return;
	
//	i2cRd(RTC_ADR,RTC_TEMPH_ADR,(unsigned char*)tempBuffer,2);
}

