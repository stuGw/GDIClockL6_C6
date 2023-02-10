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
extern char effBrightCoef;
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
int confTm = 0;

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
	if(flgSetTm)setTime(tmdt.hour,tmdt.min,tmdt.sec); flgSetTm = 0;
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
	if(flgSetTm){setTime(tmdt.hour,tmdt.min,tmdt.sec); flgSetTm = 0;}
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
	if(ledMode == LED_ONCE)startWsTransfer();
}

 void confLedBright(unsigned short int currValue,unsigned char led)
{	
	setLedBright(led,currValue);
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
	prevConfVals[CONF_LC_ALL] = WS_GREEN;
	prevConfVals[CONF_LC_1] = WS_GREEN;
	prevConfVals[CONF_LC_2] = WS_GREEN;
	prevConfVals[CONF_LC_3] = WS_GREEN;
	prevConfVals[CONF_LC_4] = WS_GREEN;
	prevConfVals[CONF_LC_5] = WS_GREEN;
	prevConfVals[CONF_LC_6] = WS_GREEN;
	prevConfVals[CONF_LB_ALL] = 0;
	prevConfVals[CONF_LB_1] = 0;
	prevConfVals[CONF_LB_2] = 0;
	prevConfVals[CONF_LB_3] = 0;
	prevConfVals[CONF_LB_4] = 0;
	prevConfVals[CONF_LB_5] = 0;
	prevConfVals[CONF_LB_6] = 0;
	prevConfVals[CONF_SEC_TYPE] = flagSecType;
	prevConfVals[CONF_GD_RUE] = 1;
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
}	

 void confBButt(char bLeft, char bRight)
{
	static char menuFlag = 0;
	char flagAction = 0;

	//Обрабатываем нажатия не в режиме меню
	if(flagConfMode == CONF_MODE_OFF)
	{
		switch(bRight)
		{
			case BUTT_SHORTCLICK:{menuFlag = CONF_GD_Y;flagAction = 1;break;}
			case BUTT_DOUBLECLICK:{menuFlag = CONF_GD_EFF;flagAction = 1;break;}
			case BUTT_LONGCLICK_P:{menuFlag = CONF_LB_ALL;flagAction = 1;break;}
			case BUTT_LONGCLICK:{menuFlag = CONF_TM_H; flagConfMode = CONF_MODE_ON;break;}
			default:break;
		}
		
		switch(bLeft)
		{
			//Яркость  индикаторов
			//case BUTT_SHORTCLICK:{menuFlag = CONF_GD_RVE;flagAction = 1;break;}
			case BUTT_DOUBLECLICK:{break;}
			case BUTT_LONGCLICK:{break;}
			default:break;
		}	
	}
	
	//Нажатия в режиме меню
	if(flagConfMode == CONF_MODE_ON)
	{
		confTm = 0;
		//Нажатие на левую кнопку - переход к следующему пункту меню
		if(bLeft == BUTT_SHORTCLICK)
		{
			menuFlag++;
			if(menuFlag>=MENU_COUNT_VAL)
			{
				menuFlag = CONF_GD_Y;
				flagConfMode = CONF_MODE_OFF;
				return;
			}
			swMenu(menuFlag,prevConfVals[menuFlag]);
			return;
		}
		//Нажатие на правую кнопку - изменение текущего параметра
		if(bRight == BUTT_SHORTCLICK){flagAction = 1;}
	}
	
	if(flagAction)
	{
		//Увеличиваем значение для текущего пункта, 
		unsigned short int cVal = prevConfVals[menuFlag]+1;
		if(cVal>confValMax[menuFlag])cVal = 0;//но не более чем максимум
		//Изменяем пункт меню на соответствующее значение
		swMenu(menuFlag,cVal);
		prevConfVals[menuFlag] = cVal;//Сохраняем состояние
	}
}
void confByEnc(unsigned char bv, unsigned short int ev, signed char sign)
{
	unsigned short menuVal = 0;
	static unsigned short menuItem = 0;
	
	if(flagConfMode == CONF_MODE_OFF)
	{
		//Обработка вращения энкодера вне режима меню
		if(sign!=0)//Если было вращение энкодера
		{
			
			int value = 0;
			if(!(RG_BUTT_INPREG&be->buttBit))//Если зажата кнопка
			{
				clearButtState(be);
			
				menuItem = CONF_LB_ALL;
			}
			else 
				menuItem = CONF_GD_Y;
			if((ledEffType>LED_RNB_ALL)&&(menuItem == CONF_LB_ALL))
			{
				value = prevEffBright+sign; menuVal = value; 
				if(value>MAX_EFF_BRIGHT) menuVal = 0; if(value<0)menuVal = MAX_EFF_BRIGHT;
			}
			else
			{		
				value = prevConfVals[menuItem]+sign;
				menuVal = value;
				if(value>confValMax[menuItem])menuVal = 0;
				else if(value<0)menuVal = confValMax[menuItem];
			}
			swMenu(menuItem,menuVal);
			prevConfVals[menuItem] = menuVal;
			return;
		}
		
		//Обработка кнопок вне режима меню
		switch (bv)
		{
			case BUTT_SHORTCLICK:
			{
			
				prevConfVals[CONF_GD_EFF] = prevConfVals[CONF_GD_EFF]+1;
				if(prevConfVals[CONF_GD_EFF]>=confValMax[CONF_GD_EFF])
					prevConfVals[CONF_GD_EFF] = 0;
				swMenu(CONF_GD_EFF,prevConfVals[CONF_GD_EFF]);
				return;
			}
			case BUTT_DOUBLECLICK:
			{
				/*prevConfVals[CONF_GD_FE] = prevConfVals[CONF_GD_FE]+1;
				if(prevConfVals[CONF_GD_FE]>confValMax[CONF_GD_FE])
					prevConfVals[CONF_GD_FE] = 0;
				swMenu(CONF_GD_FE,prevConfVals[CONF_GD_FE]);*/
				gdiVal[0] = tmdt.day/10; gdiVal[1] = tmdt.day%10;
				gdiVal[2] = tmdt.month/10; gdiVal[3] = tmdt.month%10;
				gdiVal[4] = tmdt.year/10; gdiVal[5] = tmdt.year%10;
				flgShow = FLG_SHOW_DT;
				return;
			}
			case BUTT_LONGCLICK:
			{
				menuItem = CONF_TM_H; 
				flagConfMode = CONF_MODE_ON;
				setSecView(SEC_CATHODE);
				flgFrEff = FIRE_EFF_OFF;
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
			if((menuItem == CONF_TM_S)&&flgSetTm){setTime(tmdt.hour,tmdt.min,tmdt.sec); flgSetTm = 0;}//Установить время при выходе из режима установки времени
			if(menuItem == CONF_DT_Y)
			{
				if(flgSetDt){ setDate(tmdt.day,tmdt.month,tmdt.year); flgSetDt = 0; }//Установить дату при выходе из режима установки даты
				indBl = 0; flagTmDtConf = CONF_NO_VALUE; 
			}
			if((menuItem == CONF_LC_ALL)&&ledEffType)
			{
				menuItem+=7;
				if(ledEffType>LED_RNB_ALL)
				{
					setEncVal(ev = prevEffBright);
					setMaxEncVal(MAX_EFF_BRIGHT);
					swMenu(menuItem,ev);
					return;
				}
				
			}
			else if((menuItem == CONF_LB_ALL)&&ledEffType)
			{
				menuItem+=7;
			}
			else menuItem++;	
			if(menuItem>=MENU_COUNT_VAL)
			{
				menuItem = CONF_GD_Y;
				flagConfMode = CONF_MODE_OFF;
				flgFrEff = prevConfVals[CONF_GD_FE];
				setEncVal(ev = prevConfVals[menuItem]);	
				setMaxEncVal(confValMax[menuItem]);
				return;
			}
			setEncVal(ev = prevConfVals[menuItem]);	
		  setMaxEncVal(confValMax[menuItem]);
		}	
		if(bv||sign) swMenu(menuItem,ev);
	}
	
}
void confByEncM(unsigned char bv, unsigned short int ev, signed char sign)
{
		unsigned short menuVal = 0;
	static unsigned short menuItem = 0;
	static char flgMenuP = 0;
	if(flagConfMode == CONF_MODE_OFF)
	{
		//Обработка вращения энкодера вне режима меню
		if(sign!=0)//Если было вращение энкодера
		{
			int value = 0;
			if(!(RG_BUTT_INPREG&BUTT_ENTER_PB1))//Если зажата кнопка
			{
				clearButtState(&bt1);
				flgLongButt = 1;
				menuItem = CONF_LB_ALL;
			}
			else 
				menuItem = CONF_GD_Y;
					
			if(ledEffType>LED_RNB_ALL)
			{
				value = prevEffBright+sign; menuVal = value; 
				if(value>MAX_EFF_BRIGHT) menuVal = 0; if(value<0)menuVal = MAX_EFF_BRIGHT;
			}
			else
			{
				value = prevConfVals[menuItem]+sign;
				menuVal = value;
				if(value>confValMax[menuItem])menuVal = 0;
				else if(value<0)menuVal = confValMax[menuItem];
			}
			swMenu(menuItem,menuVal);
			prevConfVals[menuItem] = menuVal;
			return;
		}
		
		//Обработка кнопок вне режима меню
		switch (bv)
		{
			case BUTT_SHORTCLICK:
			{	
				prevConfVals[CONF_GD_EFF] = prevConfVals[CONF_GD_EFF]+1;
				if(prevConfVals[CONF_GD_EFF]>=confValMax[CONF_GD_EFF])
					prevConfVals[CONF_GD_EFF] = 0;
				swMenu(CONF_GD_EFF,prevConfVals[CONF_GD_EFF]);
				return;
			}
			case BUTT_DOUBLECLICK:
			{
				/*prevConfVals[CONF_GD_FE] = prevConfVals[CONF_GD_FE]+1;
				if(prevConfVals[CONF_GD_FE]>confValMax[CONF_GD_FE])
					prevConfVals[CONF_GD_FE] = 0;
				swMenu(CONF_GD_FE,prevConfVals[CONF_GD_FE]);*/
				flgShow = FLG_SHOW_DT;
				return;
			}
			case BUTT_LONGCLICK:
			{
				menuItem = CONF_TM_H; 
				flagConfMode = CONF_MODE_ON;
				flgFrEff = FIRE_EFF_OFF;
				setEncVal(ev = 0);	
				setMaxEncVal(23);
				flgMenuP = 1;
				break;
			}
			default:break;
		}
	}
	
	//Обработка режима меню
	if(flagConfMode == CONF_MODE_ON)
	{
		confTm = 0;
		if(bv == BUTT_LONGCLICK){ flgMenuP = 1; setEncVal(ev = menuItem); setMaxEncVal(23);}
		//Если в основном меню
		if(flgMenuP)
		{
			if(ev<CONF_TM_H) setEncVal(ev = CONF_TM_H);
			set2NumsToIndsEff(0, ev);
			if(bv == BUTT_SHORTCLICK)//Перейти к текущему пункту
			{
				menuItem = ev;
				setEncVal(ev = prevConfVals[menuItem]);	
				setMaxEncVal(confValMax[menuItem]);
				swMenu(menuItem,ev);
				flgMenuP = 0;	
			}	
			if(bv == BUTT_LONGCLICK){flgMenuP = 0; flagConfMode = CONF_MODE_OFF;}
			return;
		}
		
		//Нажатие на кнопку - переход к текущему пункту
		if(bv == BUTT_SHORTCLICK)
		{
			if((menuItem == CONF_TM_S)&&flgSetTm){setTime(tmdt.hour,tmdt.min,tmdt.sec); flgSetTm = 0;}//Установить время при выходе из режима установки времени
			if(menuItem == CONF_DT_Y)
			{
				if(flgSetDt){ setDate(tmdt.day,tmdt.month,tmdt.year); flgSetDt = 0; }//Установить дату при выходе из режима установки даты
				indBl = 0; flagTmDtConf = CONF_NO_VALUE; 
			}
			if((menuItem == CONF_LC_ALL)&&ledEffType) menuItem+=7;
			else menuItem++;	
			if(menuItem>=MENU_COUNT_VAL)
			{
				menuItem = CONF_GD_Y;
				flagConfMode = CONF_MODE_OFF;
				flgFrEff = prevConfVals[CONF_GD_FE];
				setEncVal(ev = prevConfVals[menuItem]);	
				setMaxEncVal(confValMax[menuItem]);
				return;
			}
			setEncVal(ev = prevConfVals[menuItem]);	
		  setMaxEncVal(confValMax[menuItem]);
		}
		
		if((bv||sign)&&(!flgMenuP))
			swMenu(menuItem,ev);
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
void handleUIButt(volatile struct Button* bt,  volatile struct Button* bt2 )
{

	char bs = 0, bs2 = 0;
	bs = handleButtInt(bt);
	bs2 = handleButtInt(bt2);
	if(bs||bs2)confBButt(bs,bs2);

}
