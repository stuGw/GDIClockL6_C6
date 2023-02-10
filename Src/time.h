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
#define DELAY_TM SHOW_TIME_SEC*SYSTMR_FREQ
#define TIME_BUFF_SZ 7
#define TM_WORK 2
#define TM_READY 0
#define TM_WAIT 1

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
struct Temp
{
	char temph;
	char templ;
};
void timeTick(void);
void refreshTime(void);
void timeToHw(unsigned char* hwBuff);
void timeFromHw(unsigned char* hwBuff);
void dateToHw(unsigned char* hwBuff);
void dateFromHw(unsigned char* hwBuff);
void getTemp(struct Temp *t);
void refrTimeNDate(void);
void setTime(int h, int m, int s);
void setDate(int d, int m, int y);
#endif //TIME_H
