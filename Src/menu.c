#include "ws2812.h"
#include "time.h"
#include "gdi.h"
#include "system.h"
#include "buttons.h"
#include "menu.h"

#define CONF_MODE_ON 0x01
#define CONF_MODE_OFF 0x00

#define CONF_GDI_YARK  0x01
#define CONF_TIME_VALUE 0x02
#define CONF_DATE_VALUE 0x03
#define CONF_NO_VALUE 0x00

#define MENU_COUNT_VAL 27

#define CONF_GD_Y   0x00
#define CONF_GD_EFF 0x01
#define CONF_TM_H 	0x02
#define CONF_TM_M 	0x03
#define CONF_TM_S 	0x04
#define CONF_DT_D 	0x05
#define CONF_DT_M 	0x06
#define CONF_DT_Y 	0x07
#define CONF_LC_ALL 0x08
#define CONF_LC_1 	0x09
#define CONF_LC_2 	0x0A
#define CONF_LC_3 	0x0B
#define CONF_LC_4 	0x0C
#define CONF_LC_5 	0x0D
#define CONF_LC_6 	0x0E
#define CONF_LB_ALL 0x0F
#define CONF_LB_1 	0x10
#define CONF_LB_2 	0x11
#define CONF_LB_3 	0x12
#define CONF_LB_4 	0x13
#define CONF_LB_5 	0x14
#define CONF_LB_6 	0x15
#define CONF_SEC_TYPE 0x16
#define CONF_GD_FE  0x17
#define CONF_GD_RUE 0x18
#define CONF_GD_COMMA 0x19
#define CONF_DATA_INTERV 0x1A
//#define CONF_GD_RVE 0x19



#define SEC_TYPE_MAX 0x01

#define CONF_LIFETIME 25000
#define CONF_DT_TIME_MAX 5
#define FLG_SHOW_NO 0x00
#define FLG_SHOW_TM 0x01
#define FLG_SHOW_DT 0x02
#define LC_RNB_MAX 0x03
#define RUE_EFF_MAX 0x05
#define COMMA_EFF_MAX 0x01
#define DATA_INTERV_MAX 0x03


extern unsigned short int encoderCounter;
extern unsigned short encoderMaxValue;
//Включает режим конфигурации
extern unsigned char effBrightCoef;
unsigned char prevEffBright;
extern volatile char flgLongButt;
extern char flgShow;
unsigned char flgSetTm, flgSetDt;//Флаги установки врем ени и даты(1 - если при настройке поменялось время(дата))
unsigned char indBl;
extern unsigned char yarkAll;
volatile unsigned char flagConfMode = CONF_MODE_OFF;
unsigned char flagTmDtConf = CONF_NO_VALUE;//Флаг настройки времени/даты()

unsigned char flagAutoYarkost;
extern struct timeNDate tmdt;
extern unsigned char currGdiEff[]; 
unsigned char cge;
extern unsigned char flgFrEff;
extern volatile unsigned char flgRueOn;
extern volatile unsigned short int flgC;
extern unsigned char flagSecType;
extern unsigned char ledMode;
extern volatile unsigned short int flgD;
extern void clearButtState(volatile struct Button *bt);
extern volatile struct Button bt1, bt2, bt0;
extern volatile struct Button* be;
unsigned short int prevConfVals[MENU_COUNT_VAL];

extern volatile unsigned int ctmr;
unsigned char ledEffType= 0;
extern volatile unsigned char gdiVal[6];	
extern unsigned short int colors[WS_ENCO_VAL];
	//Время ожидания отклика при настройке
volatile int confTm = 0;

//Количество значений
const unsigned short int confValMax[MENU_COUNT_VAL] = {
	L_MAX_YARK,
	GDI_COUNT_EFF,
	MAX_HOUR_VAL,
	MAX_MINUTE_VAL,
	MAX_SEC_VAL,
	MAX_DAY_VAL,
	MAX_MONTH_VAL,
	MAX_YEAR_VAL,
	WS_ENCO_VAL,
	WS_ENCOCOLOR_VAL,
	WS_ENCOCOLOR_VAL,
	WS_ENCOCOLOR_VAL,
	WS_ENCOCOLOR_VAL,
	WS_ENCOCOLOR_VAL,
	WS_ENCOCOLOR_VAL,
	WS_MAX_BRIGHT,
	WS_MAX_BRIGHT,
	WS_MAX_BRIGHT,
	WS_MAX_BRIGHT,
	WS_MAX_BRIGHT,
	WS_MAX_BRIGHT,
	WS_MAX_BRIGHT,
	SEC_TYPE_MAX,
	FIRE_EFF_MAX,
	RUE_EFF_MAX,
	COMMA_EFF_MAX,
  DATA_INTERV_MAX};



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~New Univers Functions																										
 void confSetYark(unsigned short int currValue)
{
	
	if(currValue == L_MAX_YARK){	flagAutoYarkost = 1;gdiVal[0] = GDI_CM1;}
	else
	{ 
		flagAutoYarkost = 0; setGdiBrightAll(currValue);
		gdiEffSet(0,gdiVal[0]); gdiEffSet(1,gdiVal[1]);
		gdiEffSet(2,gdiVal[2]); gdiEffSet(3,gdiVal[3]);
		gdiEffSet(4,gdiVal[4]); gdiEffSet(5,gdiVal[5]);
	}
}

 void confSetHour(unsigned short int currValue)
{
	//flagTmDtConf = CONF_TIME_VALUE;//Оотключаем отображение времени(даты),откл. запись тек. врем в пред. конфиг. параметры
	if(currValue!=tmdt.hour) flgSetTm = 1;
	tmdt.hour = currValue;
	prevConfVals[CONF_TM_H] = currValue;
	gdiVal[0] = tmdt.hour/10;
	gdiVal[1] = tmdt.hour%10;
	setIndCathFade(0,gdiVal[0]);
	setIndCathFade(1,gdiVal[1]);
	if(flgSetTm){ setTimeHour(tmdt.hour); flgSetTm = 0; }
	//setIndCathFade(2,tmdt.min/10);
	//setIndCathFade(3,tmdt.min%10);
	//setIndCathFade(4,tmdt.sec/10);
	//setIndCathFade(5,tmdt.sec%10);
	
}

 void confSetMinute(unsigned short int currValue)
{
	//flagTmDtConf = CONF_TIME_VALUE;
	if(currValue!=tmdt.min) flgSetTm = 1;

  tmdt.min = currValue;
	tmdt.sec = 0;
	prevConfVals[CONF_TM_M] = currValue;
	gdiVal[2] = tmdt.min/10;
	gdiVal[3] = tmdt.min%10;
	setIndCathFade(2,gdiVal[2]);
	setIndCathFade(3,gdiVal[3]);
	if(flgSetTm){setTimeMinSec(tmdt.min,tmdt.sec); flgSetTm = 0;}
}

 void confSetSec(unsigned short int currValue)
{
	//flagTmDtConf = CONF_TIME_VALUE;
	if(currValue!=tmdt.sec) flgSetTm = 1;

	tmdt.sec = currValue;
	prevConfVals[CONF_TM_S] = currValue;
	gdiVal[4] = tmdt.sec/10;
	gdiVal[5] = tmdt.sec%10;
	setIndCathFade(4,gdiVal[4]);
	setIndCathFade(5,gdiVal[5]);
}
 void confSetDay(unsigned short int currValue)
{		
	setIndCathFade(0,tmdt.day/10); setIndCathFade(1,tmdt.day%10);
	setIndCathFade(2,tmdt.month/10); setIndCathFade(3,tmdt.month%10);
	setIndCathFade(4,tmdt.year/10); setIndCathFade(5,tmdt.year%10);
	
	//flagTmDtConf = CONF_DATE_VALUE;
	if(currValue!=tmdt.day) flgSetDt = 1;
	tmdt.day = currValue;	
	prevConfVals[CONF_DT_D] = currValue;
	gdiVal[0] = tmdt.day/10;
	gdiVal[1] = tmdt.day%10;		
	setIndCathFade(0,gdiVal[0]);				
	setIndCathFade(1,gdiVal[1]);	
	gdiVal[2] = tmdt.month/10;
	gdiVal[3] = tmdt.month%10;
	gdiVal[4] = tmdt.year/10;
	gdiVal[5] = tmdt.year%10;
	if(flgSetDt){ setDate(tmdt.day,tmdt.month,tmdt.year); flgSetDt = 0; }
//	setIndCathFade(2,tmdt.month/10);
//	setIndCathFade(3,tmdt.month%10);
//	setIndCathFade(4,tmdt.year/10);
//	setIndCathFade(5,tmdt.year%10);
}

 void confSetMonth(unsigned short int currValue)
{	
	//flagTmDtConf = CONF_DATE_VALUE;	
	if(currValue!=tmdt.month) flgSetDt = 1;
	tmdt.month = currValue;			
	prevConfVals[CONF_DT_M] = currValue;
	gdiVal[2] = tmdt.month/10;
	gdiVal[3] = tmdt.month%10;		
	setIndCathFade(2,gdiVal[2]);				
	setIndCathFade(3,gdiVal[3]);		
	if(flgSetDt){ setDate(tmdt.day,tmdt.month,tmdt.year); flgSetDt = 0; }
}

 void confSetYear(unsigned short int currValue)
{			
	//flagTmDtConf = CONF_DATE_VALUE;	
	if(currValue!=tmdt.year) flgSetDt = 1;
	tmdt.year = currValue;	
	prevConfVals[CONF_DT_Y] = currValue;
	gdiVal[4] = tmdt.year/10;
	gdiVal[5] = tmdt.year%10;		
	setIndCathFade(4,gdiVal[4]);				
	setIndCathFade(5,gdiVal[5]);		
if(flgSetDt){ setDate(tmdt.day,tmdt.month,tmdt.year); flgSetDt = 0; }	
}

//Config all leds color
 void confAllLedsColor(unsigned short int currValue)
{	
	setAllLedsBright(7);

	if(currValue>WS_ENCOCOLOR_VAL)
	{
		ledEffType = currValue-WS_ENCOCOLOR_VAL;
	if(ledEffType>LED_RNB_ALL)effBrightCoef = 7;
		//setLedMode(LED_CIRC);
		prevConfVals[CONF_LC_ALL] = currValue;
		return;
	}
	else { ledEffType = 0; setLedMode(LED_ONCE);}
	prevConfVals[CONF_LC_ALL] = currValue;
	prevConfVals[CONF_LC_1] = currValue;
	prevConfVals[CONF_LC_2] = currValue;
	prevConfVals[CONF_LC_3] = currValue;
	prevConfVals[CONF_LC_4] = currValue;
	prevConfVals[CONF_LC_5] = currValue;
	prevConfVals[CONF_LC_6] = currValue;	
	setAllLedsColor(colors[currValue]);
	if(ledMode == LED_ONCE)startWsTransfer();

}

void confAllLedsBright(unsigned short int currValue)
{
	if(!currValue){setAllLedsBright(0); startWsTransfer();}
	if(ledEffType>LED_RNB_ALL)
	{ 
			effBrightCoef = currValue; 
			prevEffBright = currValue; 
		
			return;
	}
	//if(currValue<7)effBrightCoef = 0; else if(currValue>14)effBrightCoef = 2; else effBrightCoef = 1;
	prevConfVals[CONF_LB_ALL] = currValue;
	prevConfVals[CONF_LB_1] = currValue;
	prevConfVals[CONF_LB_2] = currValue;
	prevConfVals[CONF_LB_3] = currValue;
	prevConfVals[CONF_LB_4] = currValue;
	prevConfVals[CONF_LB_5] = currValue;
	prevConfVals[CONF_LB_6] = currValue;
	setAllLedsBright(currValue);
	if(ledMode == LED_ONCE)startWsTransfer();
}

 void confLedColor(unsigned short int currValue, unsigned char ledInd)
{
	setLedColor(ledInd,colors[currValue]);
	prevConfVals[CONF_LC_1+ledInd] = currValue;
	if(ledMode == LED_ONCE)startWsTransfer();
}

 void confLedBright(unsigned short int currValue,unsigned char led)
{	
	setLedBright(led,currValue);
	prevConfVals[CONF_LB_1+led] = currValue;
	if(ledMode == LED_ONCE)startWsTransfer();
}
 void confSecType(unsigned short int currValue){flagSecType = (char)currValue; prevConfVals[CONF_SEC_TYPE] = currValue; }

 void confSetGdiEff(unsigned short int currValue)
{
	cge = currValue; currGdiEff[0]=cge;currGdiEff[1]=cge;currGdiEff[2]=cge; 
	currGdiEff[3]=cge;currGdiEff[4]=cge;currGdiEff[5]=cge;
}

 void confSetGdiFire(unsigned short int currValue){flgFrEff = currValue; prevConfVals[CONF_GD_FE] = currValue;}
 void confRueEff(unsigned short int currValue){flgRueOn = currValue; prevConfVals[CONF_GD_RUE] = currValue;}
 void confCommaEff(unsigned short int currValue){flgC = currValue; prevConfVals[CONF_GD_COMMA] = currValue;if(!flgC){offCommas();}ctmr = 0;}
 void confDataInterv(unsigned short int currValue){flgD = currValue;  prevConfVals[CONF_DATA_INTERV] = currValue;}
 void confGdiRve(unsigned short int currValue){};
	


 void swMenu(char punkt, unsigned short int val)
{
	switch(punkt)
	{
		case CONF_GD_Y:{confSetYark(val);break;}//   00 яркость индикаторов
		case CONF_GD_EFF:{confSetGdiEff(val);break;}//1 эффект переключения
		case CONF_TM_H:{confSetHour(val); indBl = 1; break;}//2 час
		case CONF_TM_M:{confSetMinute(val); indBl = 3; break;}//3 минута
		case CONF_TM_S:{confSetSec(val); indBl = 5; break;}//4 секунда
		case CONF_DT_D:{confSetDay(val); indBl = 1; break;}//5 день
		case CONF_DT_M:{confSetMonth(val); indBl = 3; break;}//6 месяц
		case CONF_DT_Y:{confSetYear(val);indBl = 5; break;}//7 год
		case CONF_LC_ALL:{confAllLedsColor(val);break;}//8 цвет подсветки всех индикаторов
		case CONF_LC_1:{confLedColor(val,0);break;}//9 цвет подсветки 1
		case CONF_LC_2:{confLedColor(val,1);break;}//10 --2
		case CONF_LC_3:{confLedColor(val,2);break;}//11 --3
		case CONF_LC_4:{confLedColor(val,3);break;}//12 --4
		case CONF_LC_5:{confLedColor(val,4);break;}//13 --5
		case CONF_LC_6:{confLedColor(val,5);break;}//14 --6
		case CONF_LB_ALL:{confAllLedsBright(val);break;}//15 яркость подсветки всех индикаторов
		case CONF_LB_1:{confLedBright(val,0);break;}//16 яркость подсветки 1
		case CONF_LB_2:{confLedBright(val,1);break;}//17 --2
		case CONF_LB_3:{confLedBright(val,2);break;}//18 --3
		case CONF_LB_4:{confLedBright(val,3);break;}//19 --4
		case CONF_LB_5:{confLedBright(val,4);break;}//20 --5
		case CONF_LB_6:{confLedBright(val,5);break;}//21 --6
		case CONF_SEC_TYPE:{confSecType(val);break;}//22
		case CONF_GD_FE:{confSetGdiFire(val);break;}//23 вкл/выкл эффект горения
		case CONF_GD_RUE:{confRueEff(val);break;}//24 выбор интервала перебора 0-выкл, 1-  каждая минута 2-5 минут 3- 15минут 4-30минут 5- час
		case CONF_GD_COMMA:{confCommaEff(val);break;}// 25 маятник - вкл\выкл
		case CONF_DATA_INTERV:{confDataInterv(val);break;} // 26 выбор интервала даты 0-20мин 1-1час 2-3часа 4-5часов
		default:break;
	}
	if((punkt>CONF_DT_Y)&&(flagConfMode == CONF_MODE_ON)) set2NumsToIndsEff(punkt,val);
}

//Заводские значения подсветки
#define COLOR_IDX_GREEN 120	//Индекс зеленого в таблице colors[] (оттенок 120 градусов)
#define FACTORY_COLOR 362	//Эффект "радуга справа налево" (WS_ENCOCOLOR_VAL + LED_RNB_RL)

void initConfVals(void)
{
	prevConfVals[CONF_GD_Y] = yarkAll;
	prevConfVals[CONF_GD_EFF] = cge;
	prevConfVals[CONF_GD_FE] = flgFrEff;
	prevConfVals[CONF_TM_H] = 4;
	prevConfVals[CONF_TM_M] = 5;
	prevConfVals[CONF_TM_S] = 6;
	prevConfVals[CONF_DT_D] = 7;
	prevConfVals[CONF_DT_M] = 8;
	prevConfVals[CONF_DT_Y] = 9;
	prevConfVals[CONF_LC_ALL] = FACTORY_COLOR;
	prevConfVals[CONF_LC_1] = COLOR_IDX_GREEN;
	prevConfVals[CONF_LC_2] = COLOR_IDX_GREEN;
	prevConfVals[CONF_LC_3] = COLOR_IDX_GREEN;
	prevConfVals[CONF_LC_4] = COLOR_IDX_GREEN;
	prevConfVals[CONF_LC_5] = COLOR_IDX_GREEN;
	prevConfVals[CONF_LC_6] = COLOR_IDX_GREEN;
	prevConfVals[CONF_LB_ALL] = 0;
	prevConfVals[CONF_LB_1] = 0;
	prevConfVals[CONF_LB_2] = 0;
	prevConfVals[CONF_LB_3] = 0;
	prevConfVals[CONF_LB_4] = 0;
	prevConfVals[CONF_LB_5] = 0;
	prevConfVals[CONF_LB_6] = 0;
	prevConfVals[CONF_SEC_TYPE] = flagSecType;
	prevConfVals[CONF_GD_RUE] = 1;
	prevConfVals[CONF_GD_COMMA] = flgC;//Маятник запятых включен с запуска (main)
	encoderMaxValue = 121;
	encoderCounter = 30;
	prevEffBright = 0;
}
																										
void initConfig(void)
{
	indBl = 0;
	yarkAll = 60;
	setGdiBrightAll(yarkAll);
	flagAutoYarkost = 1;
	flagTmDtConf = CONF_NO_VALUE; 
	
	flagSecType = SEC_CATHODE;
	encoderMaxValue = L_MAX_YARK;
	flagSecType = SEC_CATHODE;
	
	initWs2812Buff();
	setAllLedsBright(0);
	setLedMode(LED_ONCE);
	setAllLedsColor(WS_GREEN);
	setAllLedsSat(100);
	startWsTransfer();
	initConfVals();
	//Заводская подсветка: эффект "радуга справа налево", яркость 0 (выключена)
	confAllLedsColor(FACTORY_COLOR);
	confAllLedsBright(0);
}

//~~~~~~~~~~~~~~~~~ Сохранение настроек: снимок и восстановление ~~~~~~~~~~~~~~~~~
//Состав и порядок значений задают формат записи во flash (settings.c). При изменении состава
//или порядка нужно увеличить SET_VERSION в settings.c. Время и дату не сохраняем: их хранит DS3231.
enum {
	SV_GD_Y, SV_GD_EFF,
	SV_LC_ALL, SV_LC_1, SV_LC_2, SV_LC_3, SV_LC_4, SV_LC_5, SV_LC_6,
	SV_LB_ALL, SV_LB_1, SV_LB_2, SV_LB_3, SV_LB_4, SV_LB_5, SV_LB_6,
	SV_SEC_TYPE, SV_GD_FE, SV_GD_RUE, SV_GD_COMMA, SV_DATA_INTERV,
	SV_EFF_BRIGHT,//Яркость эффектов огня и угля (prevEffBright)
	SV_COUNT
};
_Static_assert(SV_COUNT == CONF_SAVED_COUNT, "CONF_SAVED_COUNT (menu.h) не совпадает с числом SV_*");

//Пункт меню для каждого значения (кроме SV_EFF_BRIGHT)
static const unsigned char savedItem[SV_EFF_BRIGHT] = {
	CONF_GD_Y, CONF_GD_EFF,
	CONF_LC_ALL, CONF_LC_1, CONF_LC_2, CONF_LC_3, CONF_LC_4, CONF_LC_5, CONF_LC_6,
	CONF_LB_ALL, CONF_LB_1, CONF_LB_2, CONF_LB_3, CONF_LB_4, CONF_LB_5, CONF_LB_6,
	CONF_SEC_TYPE, CONF_GD_FE, CONF_GD_RUE, CONF_GD_COMMA, CONF_DATA_INTERV
};

//Максимально допустимое значение (включительно)
static unsigned short savedMax(unsigned char i)
{
	if(i == SV_EFF_BRIGHT) return MAX_EFF_BRIGHT;
	if(i == SV_GD_EFF) return confValMax[CONF_GD_EFF]-1;//confValMax - число эффектов
	return confValMax[savedItem[i]];
}

//Текущие сохраняемые значения
void confSnapshot(unsigned short* out)
{
	unsigned char i;
	for(i = 0; i < SV_EFF_BRIGHT; i++) out[i] = prevConfVals[savedItem[i]];
	out[SV_EFF_BRIGHT] = prevEffBright;
}

//Применение сохраненных значений поверх заводских. Возвращает 0 и ничего не меняет, если
//значения недопустимы (повреждена запись, другая версия прошивки).
unsigned char confRestore(const unsigned short* v)
{
	unsigned char i;
	for(i = 0; i < SV_COUNT; i++)
		if(v[i] > savedMax(i)) return 0;

	//Яркость ламп: L_MAX_YARK - автоматическая (по датчику освещенности), иначе постоянная
	if(v[SV_GD_Y] >= L_MAX_YARK) flagAutoYarkost = 1;
	else { flagAutoYarkost = 0; setGdiBrightAll(v[SV_GD_Y]); }
	prevConfVals[CONF_GD_Y] = v[SV_GD_Y];
	//Эффект смены цифр
	confSetGdiEff(v[SV_GD_EFF]);
	prevConfVals[CONF_GD_EFF] = v[SV_GD_EFF];
	//Подсветка: общий цвет (он же выбирает эффект; заодно выставляет яркость 7) и общая яркость...
	confAllLedsColor(v[SV_LC_ALL]);
	confAllLedsBright((ledEffType > LED_RNB_ALL) ? v[SV_EFF_BRIGHT] : v[SV_LB_ALL]);
	//...затем индивидуальные цвета и яркости (при эффекте они не используются). Общие функции
	//перезаписывают prevConfVals общими значениями, поэтому сохраненные записываются после них.
	for(i = 0; i < 6; i++)
	{
		if(ledEffType == 0)
		{
			confLedColor(v[SV_LC_1+i],i);
			confLedBright(v[SV_LB_1+i],i);
		}
		prevConfVals[CONF_LC_1+i] = v[SV_LC_1+i];
		prevConfVals[CONF_LB_1+i] = v[SV_LB_1+i];
	}
	prevConfVals[CONF_LC_ALL] = v[SV_LC_ALL];
	prevConfVals[CONF_LB_ALL] = v[SV_LB_ALL];
	prevEffBright = v[SV_EFF_BRIGHT];
	//Остальное
	confSecType(v[SV_SEC_TYPE]);
	confSetGdiFire(v[SV_GD_FE]);
	confRueEff(v[SV_GD_RUE]);
	confCommaEff(v[SV_GD_COMMA]);
	confDataInterv(v[SV_DATA_INTERV]);
	return 1;
}	

//~~~~~~~~~~~~~~~~~ Общая логика меню для энкодера и для двух кнопок ~~~~~~~~~~~~~~~~~
//Яркость подсветки при эффектах огня и угля (ledEffType > LED_RNB_ALL) - это "яркость эффекта":
//значение хранится в prevEffBright, диапазон 0..MAX_EFF_BRIGHT
static unsigned char itemIsEffBright(unsigned char item)
{
	return (item == CONF_LB_ALL) && (ledEffType > LED_RNB_ALL);
}

//Текущее значение пункта меню
static unsigned short itemValue(unsigned char item)
{
	return itemIsEffBright(item) ? prevEffBright : prevConfVals[item];
}

//Изменение значения пункта на +-1 по кругу с немедленным применением. Вне меню так меняются
//яркость ламп (CONF_GD_Y), яркость подсветки (CONF_LB_ALL) и эффект смены цифр (CONF_GD_EFF)
static void stepItem(unsigned char item, signed char delta)
{
	int max, value;
	if(itemIsEffBright(item)) max = MAX_EFF_BRIGHT;
	else if(item == CONF_GD_EFF) max = confValMax[item]-1;//confValMax - число эффектов
	else max = confValMax[item];
	value = itemValue(item)+delta;
	if(value>max) value = 0;
	else if(value<0) value = max;
	swMenu(item,value);
	prevConfVals[item] = value;//Сохраняем состояние
}

//Показать дату вместо времени (на DELAY_DT, затем handleTmDtFlag вернет время)
static void showDate(void)
{
	gdiVal[0] = tmdt.day/10; gdiVal[1] = tmdt.day%10;
	gdiVal[2] = tmdt.month/10; gdiVal[3] = tmdt.month%10;
	gdiVal[4] = tmdt.year/10; gdiVal[5] = tmdt.year%10;
	flgShow = FLG_SHOW_DT;
}

//Вход в режим настройки. Возвращает первый пункт - часы
static unsigned char menuEnter(void)
{
	flagConfMode = CONF_MODE_ON;
	setSecView(SEC_CATHODE);
	flgFrEff = FIRE_EFF_OFF;
	return CONF_TM_H;
}

//Переход к следующему пункту меню. Возвращает его номер, MENU_COUNT_VAL - если меню закончилось.
static unsigned char menuNextItem(unsigned char item)
{
	//При уходе с секунд записываем их в DS3231 (часы и минуты записываются сразу при изменении)
	if((item == CONF_TM_S)&&flgSetTm){setTimeSec(tmdt.sec); flgSetTm = 0;}
	//При уходе с года сбрасываем мигание индикаторов
	if(item == CONF_DT_Y)
	{
		if(flgSetDt){ setDate(tmdt.day,tmdt.month,tmdt.year); flgSetDt = 0; }
		indBl = 0; flagTmDtConf = CONF_NO_VALUE;
	}
	//Если включен эффект подсветки, индивидуальные цвета и яркости ламп пропускаем
	if(ledEffType && ((item == CONF_LC_ALL)||(item == CONF_LB_ALL))) return item+7;
	return item+1;
}

//Выход из режима настройки после последнего пункта (по таймауту выходит main)
static void menuExit(void)
{
	flagConfMode = CONF_MODE_OFF;
	flgFrEff = prevConfVals[CONF_GD_FE];
}

//Управление двумя кнопками (CONF_BUTT). bLeft - кнопка на PB0 (bt0), bRight - на PB2 (bt2).
//Вне меню:
//  правая: короткое - яркость ламп +1, двойное - эффект смены цифр +1, долгое - вход в меню
//  левая:  короткое - яркость подсветки +1, двойное - показать дату
//В меню:
//  левая:  короткое - следующий пункт
//  правая: короткое - значение +1, двойное - значение -1 (по кругу)
//Выход из меню - после последнего пункта или по таймауту (main.c)
void confBButt(char bLeft, char bRight)
{
	static unsigned char menuItem = CONF_TM_H;

	if(flagConfMode == CONF_MODE_OFF)
	{
		switch(bRight)
		{
			case BUTT_SHORTCLICK:{stepItem(CONF_GD_Y,1);break;}
			case BUTT_DOUBLECLICK:{stepItem(CONF_GD_EFF,1);break;}
			case BUTT_LONGCLICK:
			{
				menuItem = menuEnter();
				swMenu(menuItem,prevConfVals[menuItem]);//Показываем первый пункт (часы)
				return;
			}
			default:break;
		}
		switch(bLeft)
		{
			case BUTT_SHORTCLICK:{stepItem(CONF_LB_ALL,1);break;}
			case BUTT_DOUBLECLICK:{showDate();break;}
			default:break;
		}
		return;
	}

	//Режим меню: любое нажатие продлевает время ожидания
	confTm = 0;
	if(bLeft == BUTT_SHORTCLICK)
	{
		menuItem = menuNextItem(menuItem);
		if(menuItem>=MENU_COUNT_VAL)
		{
			menuExit();
			return;
		}
		swMenu(menuItem,itemValue(menuItem));
		return;
	}
	if(bRight == BUTT_SHORTCLICK) stepItem(menuItem,1);
	else if(bRight == BUTT_DOUBLECLICK) stepItem(menuItem,-1);
}

//Управление энкодером с кнопкой (CONF_B_ENC).
//bv - событие кнопки, ev - значение энкодера, sign - направление вращения (0 - не вращался).
//Вне меню: вращение - яркость ламп, вращение с зажатой кнопкой - яркость подсветки,
//короткое нажатие - эффект смены цифр, двойное - показать дату, долгое - вход в меню.
//В меню: вращение меняет значение (энкодер привязан к текущему пункту), нажатие - следующий пункт.
void confByEnc(unsigned char bv, unsigned short int ev, signed char sign)
{
	static unsigned char menuItem = 0;

	if(flagConfMode == CONF_MODE_OFF)
	{
		//Обработка вращения энкодера вне режима меню
		if(sign!=0)
		{
			if(!(RG_BUTT_INPREG&be->buttBit))//Если зажата кнопка - подсветка, иначе - яркость ламп
			{
				clearButtState(be);
				menuItem = CONF_LB_ALL;
			}
			else
				menuItem = CONF_GD_Y;
			stepItem(menuItem,sign);
			return;
		}

		//Обработка кнопки вне режима меню
		switch (bv)
		{
			case BUTT_SHORTCLICK:{stepItem(CONF_GD_EFF,1);return;}
			case BUTT_DOUBLECLICK:{showDate();return;}
			case BUTT_LONGCLICK:
			{
				menuItem = menuEnter();
				setEncVal(ev = prevConfVals[CONF_TM_H]);
				setMaxEncVal(confValMax[CONF_TM_H]);
				break;
			}
			default:break;
		}
	}

	//Обработка режима меню
	if(flagConfMode == CONF_MODE_ON)
	{
		confTm = 0;
		//Нажатие на кнопку - переход к следующему пункту меню
		if(bv == BUTT_SHORTCLICK)
		{
			menuItem = menuNextItem(menuItem);
			if(menuItem>=MENU_COUNT_VAL)
			{
				menuItem = CONF_GD_Y;
				menuExit();
				setEncVal(ev = prevConfVals[menuItem]);
				setMaxEncVal(confValMax[menuItem]);
				return;
			}
			if(itemIsEffBright(menuItem))
			{
				setEncVal(ev = prevEffBright);
				setMaxEncVal(MAX_EFF_BRIGHT);
				swMenu(menuItem,ev);
				return;
			}
			setEncVal(ev = prevConfVals[menuItem]);
			setMaxEncVal(confValMax[menuItem]);
		}
		if(bv||sign) swMenu(menuItem,ev);
	}
}
//Обработка пользовательского интерфейса ввода
void handleUI(volatile struct Button* bt)
{
	signed char es = 0;
	unsigned short int ev = 0;
	char bs = 0;
	bs = handleButtInt(bt);
	es = handleEncoder(&ev);
	if(es||bs)confByEnc(bs,ev,es);

}
//Обработка двух кнопок (CONF_BUTT): btLeft - "меню" (PB0), btRight - "значение" (PB2)
void handleUIButt(volatile struct Button* btLeft, volatile struct Button* btRight)
{
	char bsLeft = handleButtInt(btLeft);
	char bsRight = handleButtInt(btRight);
	if(bsLeft||bsRight)confBButt(bsLeft,bsRight);
}
