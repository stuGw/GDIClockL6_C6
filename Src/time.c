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
	tmdt.sec = (hwBuff[0]/16)*10+hwBuff[0]%16;	
	tmdt.min = (hwBuff[1]/16)*10+hwBuff[1]%16;
	tmdt.hour = (hwBuff[2]&0x0f)%10;
	if(hwBuff[2]&RTC_HOUR_10) tmdt.hour+=10;
	if(hwBuff[2]&RTC_HOUR_20) tmdt.hour+=20;
	hwBuff[0] =0; hwBuff[1] =0; hwBuff[2] =0; 
	
	//I2C1->CR1&=~I2C_CR1_PE;
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

void getTemp(struct Temp *t)
{
	ds3231GetTemp((char*)t);
}
void refrTimeNDate()
{
	unsigned char tbuff[4];
	ds3231GetTime(tbuff);
	timeFromHw(tbuff);
	ds3231GetDate(tbuff);
	dateFromHw(tbuff);
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
	tmdt.day = d;
	tmdt.month = m;
	tmdt.year = y;
	dateToHw(tbuff);
	ds3231SetDate(tbuff);
}
