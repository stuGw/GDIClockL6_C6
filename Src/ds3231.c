#include "ds3231.h"

//Чтение времени и даты выполняется в фоне через прерывания и DMA (sendI2C, I2C1_EV_IRQHandler).
//Здесь только блокирующая запись при установке времени и даты (i2c1Write).
void ds3231SetDate(unsigned char* dateBuffer)
{
	if(!dateBuffer)
		return;
	i2c1Write(RTC_ADR,RTC_WDAY_ADR,dateBuffer,4);
}

void ds3231SetTime(unsigned char* timeBuffer)
{
	if(!timeBuffer)
		return;
	i2c1Write(RTC_ADR,RTC_SEC_ADR,timeBuffer,3);
}
