#include "time.h"
#include "ds3231.h"

#define HOUR_READ_DS 23
#define READ_DS_ALWAYS
extern struct timeNDate tmdt;

void timeTick()
{
//		unsigned char tbuff[4];
#ifdef READ_DS_ALWAYS
		//ds3231GetTime(tbuff);
	//	timeFromHw(tbuff);
	//	ds3231GetDate(tbuff);
	//	dateFromHw(tbuff);
	sendI2C();	

	return;
#else
	if((tmdt.min == 0)||(tmdt.sec == 10))
	{
		//ds3231GetTime(tbuff);
		//timeFromHw(tbuff);
		//ds3231GetDate(tbuff);
		//dateFromHw(tbuff);
	//	sendI2C();
		return;
	}
	
	tmdt.sec++;
	if(tmdt.sec>59){tmdt.min++; tmdt.sec = 0;}else return;
	if(tmdt.min>59){tmdt.hour++; tmdt.min = 0;}else return;
	if(tmdt.hour>23){tmdt.day++;tmdt.hour = 0;}
#endif
}

//(dta/16)*10+dta%16;//bp 16 d 10
void timeToHw(unsigned char* hwBuff)
{
	hwBuff[0] = (tmdt.sec/10)*16+tmdt.sec%10;
	hwBuff[1] = (tmdt.min/10)*16+tmdt.min%10;
	hwBuff[2] = 0x00;
	hwBuff[2] = tmdt.hour%10;
	if((tmdt.hour/10) == 1) hwBuff[2]|=RTC_HOUR_10;
	if((tmdt.hour/10) == 2) hwBuff[2]|=RTC_HOUR_20;
	
}

void timeFromHw(unsigned char* hwBuff)
{
	unsigned char s,m,h;
	s = (hwBuff[0]/16)*10+hwBuff[0]%16;
	m = (hwBuff[1]/16)*10+hwBuff[1]%16;
	h = (hwBuff[2]&0x0f)%10;
	if(hwBuff[2]&RTC_HOUR_10) h+=10;
	if(hwBuff[2]&RTC_HOUR_20) h+=20;
	hwBuff[0] =0; hwBuff[1] =0; hwBuff[2] =0;
	//tmdt читают прерывания RTC и TIM1 - обновляем время одним куском, без промежуточных значений
	{
		IRQ_SAVE();
		tmdt.sec = s; tmdt.min = m; tmdt.hour = h;
		IRQ_RESTORE();
	}
}

void dateToHw(unsigned char* hwBuff)
{
	hwBuff[0] = tmdt.wDay;
	hwBuff[1] = (tmdt.day/10)*16+tmdt.day%10;
	hwBuff[2] = (tmdt.month/10)*16+tmdt.month%10;
	hwBuff[3] = (tmdt.year/10)*16+tmdt.year%10;
}

void dateFromHw(unsigned char* hwBuff)
{
	tmdt.wDay = hwBuff[0];
	tmdt.day = (hwBuff[1]/16)*10+hwBuff[1]%16;
	tmdt.month = ((hwBuff[2]&0x7f)/16)*10+(hwBuff[2]&0x7f)%16;
	tmdt.year = (hwBuff[3]/16)*10+hwBuff[3]%16;
}


void setTime(int h, int m, int s)
{
	unsigned char tbuff[4];	
	tmdt.hour = h;
	tmdt.min = m;
	tmdt.sec = s;
  timeToHw(tbuff);
  ds3231SetTime(tbuff);
}
void setDate(int d, int m, int y)
{
	unsigned char tbuff[4];			
	//Неверную дату (день или месяц 0, 31 февраля) в DS3231 не записываем
	if(m < 1) m = 1;
	if(m > 12) m = 12;
	if(d < 1) d = 1;
	if(d > daysInMonth(m,y)) d = daysInMonth(m,y);
	tmdt.day = d;
	tmdt.month = m;
	tmdt.year = y;
	dateToHw(tbuff);
	ds3231SetDate(tbuff);
}

//Запись в DS3231 только тех регистров, которые меняет пользователь.
//В режиме настройки tmdt "заморожен" на моменте входа в меню, а DS3231 продолжает идти:
//setTime() записал бы в часы устаревшие минуты/секунды и отмотал бы время назад.
void setTimeHour(int h)
{
	unsigned char tbuff[3];
	tmdt.hour = h;
	timeToHw(tbuff);
	i2c1Write(RTC_ADR,RTC_HR_ADR,&tbuff[2],1);
}

//Минуты вместе с секундами (при установке минут секунды обнуляются)
void setTimeMinSec(int m, int s)
{
	unsigned char tbuff[3];
	tmdt.min = m;
	tmdt.sec = s;
	timeToHw(tbuff);
	i2c1Write(RTC_ADR,RTC_SEC_ADR,tbuff,2);
}

void setTimeSec(int s)
{
	unsigned char tbuff[3];
	tmdt.sec = s;
	timeToHw(tbuff);
	i2c1Write(RTC_ADR,RTC_SEC_ADR,tbuff,1);
}
