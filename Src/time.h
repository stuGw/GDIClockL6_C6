#ifndef TIME_H
#define TIME_H
#define TMDT_LENGTH 7

#define MAX_HOUR_VAL 23
#define MAX_MINUTE_VAL 59
#define MAX_SEC_VAL 59
#define MAX_YEAR_VAL 99
#define MAX_MONTH_VAL 12
#define MAX_DAY_VAL 31
//Задержка показа даты и времени
#define DELAY_DT SHOW_DATE_SEC*SYSTMR_FREQ
#define TIME_BUFF_SZ 7
#define TM_WORK 2
#define TM_READY 0
#define TM_WAIT 1

//Число дней в месяце (год 0..99 = 2000..2099: високосный каждый четвертый). Для неверного месяца - 31.
static inline unsigned char daysInMonth(unsigned char month, unsigned char year)
{
	static const unsigned char days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
	if((month < 1)||(month > 12)) return 31;
	if((month == 2)&&((year % 4) == 0)) return 29;
	return days[month-1];
}

struct timeNDate
{
	int millSecs;
	char sec;
	char min;
	char hour;
	char wDay;
	char day;
	char month;
	short int year;
};	
void timeTick(void);
void timeToHw(unsigned char* hwBuff);
void timeFromHw(unsigned char* hwBuff);
void dateToHw(unsigned char* hwBuff);
void dateFromHw(unsigned char* hwBuff);
void setTime(int h, int m, int s);
void setDate(int d, int m, int y);
void setTimeHour(int h);
void setTimeMinSec(int m, int s);
void setTimeSec(int s);
#endif //TIME_H
