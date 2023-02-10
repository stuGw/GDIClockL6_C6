#include <string.h>
#include <stdlib.h>
#include "gdi.h"
//#include "system.h"
#include "time.h"
#include "ws2812.h"
//Используется для эффектов переключенияя
unsigned short int flgFTsk = 0;
const unsigned char tskFArr[6] = {FTSK_0,FTSK_1,FTSK_2,FTSK_3,FTSK_4,FTSK_5};
//Коды катодов и анодов
const unsigned short int LCat[11] = {CATHODE_0,CATHODE_1,CATHODE_2,CATHODE_3,CATHODE_4,CATHODE_5,CATHODE_6,CATHODE_7,CATHODE_8,CATHODE_9,ZPT_R};
const unsigned short int LAn[6] = {LAMP_1,LAMP_2,LAMP_3,LAMP_4,LAMP_5,LAMP_6};
//Последовательности расположения катодов в разных типах индикаторов
const unsigned char in14CathsPos[10] = {0,1,2,3,4,5,6,7,8,9};
const unsigned char in9CathsPos[10] = {1,0,2,9,3,8,4,7,5,6};
//Текущий эффект переключения
unsigned char currGdiEff[6];// = GDI_EFF_PL1;
//Описывает 6 газоразрядных индикаторов
struct lamps gdi[6];
//Время и дата
extern struct timeNDate tmdt;
//Общая я ркость индикаторов
unsigned char yarkAll;
//Значения для индикаторов(устанавливаются каждую сек. по прерыванию от ЧРВ)
extern char gdiVal[];
extern char flgFrEff;
extern char fireYark;
extern unsigned char flagSecType;
extern char flgShow;
extern volatile unsigned char flagConfMode;

//60,48,38,24,19,15,12,10,8,6,5,4,3,2,1

char fCfArr[FIRE_COEF_CNT] = {10,15,14,13,12,11,10,12,11,10,10,10,20,30,
															50,45,40,37,35,32,30,27,25,23,20,17,15,12,11,10,10,12,11,10,10,12,11,10,14,13,12,11,10,12,11,10,8,6,5,6,3,4,3,2,1,2,4,2,6,10,
															30,27,25,23,20,17,15,13,11,10,10,13,12,11,10,13,11,10,14,13,12,11,10,10,20,
															40,37,35,33,30,27,25,23,20,17,15,13,12,11,10,12,11,10,10,12,11,10,9,9,8,7,6,5,2,4,6,15,14,13,12,11,10,12,
															25,23,20,17,15,14,13,12,11,10,10,14,13,12,11,10,10,12,13,14,15,
															30,27,25,23,20,17,15,14,13,13,12,12,11,10,10,11,12,13,14,15,14,13,12,11,10,10,10,14,13,12,11,10,10,10,13,12,11,10};


	
void gdiEffHandle(void)
{
	if(tmdt.sec == 0) cathRueS(0); else if(flgFTsk&FTSK_0) catFade[currGdiEff[0]](0);
	if(tmdt.sec == 1) cathRueS(1); else if(flgFTsk&FTSK_1) catFade[currGdiEff[1]](1);
	if(tmdt.sec == 2) cathRueS(2);else if(flgFTsk&FTSK_2) catFade[currGdiEff[2]](2);
	if(tmdt.sec == 3) cathRueS(3);else if(flgFTsk&FTSK_3) catFade[currGdiEff[3]](3);
		if(flgFTsk&FTSK_5) catFade[currGdiEff[5]](5);
		if(flgFTsk&FTSK_4) catFade[currGdiEff[4]](4);
		
		
		
		
}

	//Установка и переключение для различных эффектов
void (*setIndCathEff[])(register unsigned char ind,register unsigned char cath) = {
	setIndCath,
	setIndCathRue,
	setIndCathTwoCaths,
	setIndCathFade,
	setIndCathFade,
	setIndCathFade,
	setIndCathFade,
	setIndCathRueS
};

void (*catFade[])(unsigned char ind) = 
{
	cathFade,
	cathRue,
	cathTwoCaths,
	cathFade,
	cathFadePrv,
	cathFadeTwo,
	cathFadeAll,
	cathRueS
};



 
void initInds()
{
	register unsigned char i = 0;
	while(i<6)
	{
		gdi[i].brAll = L_MAX_YARK-1;
		gdi[i].currC = 0;
		gdi[i].prevC = 9;
		gdi[i].currC = 0;
		gdi[i].zptL = 0;
		gdi[i].zptR = 0;
		memset(gdi[i].br,0,12);
		i++;
	}
}


//Задание яркости для всех индикаторов
void setGdiBrightAll(unsigned char value)
{  
  gdi[0].brAll = value; gdi[1].brAll = value;
	gdi[2].brAll = value; gdi[3].brAll = value;
	if((flagSecType == SEC_CATHODE)||(flgShow == 2)||(flagConfMode == 1)){gdi[4].brAll = value; gdi[5].brAll = value;}
	else{ gdi[4].brAll = 0; gdi[5].brAll = 0;}
}

void perebor(unsigned char ind, unsigned char num)
{
	
}
void offCommas(void)
{
	gdi[0].br[10] = 0; gdi[1].br[10] = 0; gdi[2].br[10] = 0;
	gdi[3].br[10] = 0; gdi[4].br[10] = 0; gdi[5].br[10] = 0;
}

void onIndComma(unsigned char ind){ gdi[ind].br[10] = gdi[ind].brAll;}
 void offIndComma(unsigned char ind){ gdi[ind].br[10] = 0; }

void setHwNumToInds(unsigned int number)
{
	unsigned int tmp;
	register unsigned char i = 0;
	tmp = number/100000;
	
	while(i<11)
	{
		gdi[0].br[i] = 0;
		gdi[1].br[i] = 0;
		gdi[2].br[i] = 0;
		gdi[3].br[i] = 0;
		gdi[4].br[i] = 0;
		gdi[5].br[i] = 0;
		i++;
	}
	
	gdi[0].br[tmp] = gdi[0].brAll;
	gdi[1].br[(number%100000)/10000]=gdi[1].brAll;
	gdi[2].br[(number%10000)/1000]=gdi[2].brAll;
	gdi[3].br[(number%1000)/100]=gdi[3].brAll;
	gdi[4].br[(number%100)/10]=gdi[4].brAll;
	gdi[5].br[number%10]=gdi[5].brAll;
}

void set2NumsToIndsEff(unsigned int n1, unsigned int n2)
{
	unsigned int tmp1,tmp2;
	tmp1 = n1/100;
	tmp2 = n2/100;
	gdiEffSet(0,tmp1);
	gdiEffSet(1,(n1%100)/10);
	gdiEffSet(2,(n1%10));
	gdiEffSet(3,tmp2);
	gdiEffSet(4,(n2%100)/10);
	gdiEffSet(5,(n2%10));
}

void setHw2NumsToInds(unsigned int n1, unsigned int n2)
{
	unsigned int tmp1,tmp2;
	register unsigned char i = 0;
	tmp1 = n1/100;
	tmp2 = n2/100;
	
	while(i<11)
	{
		gdi[0].br[i] = 0;
		gdi[1].br[i] = 0;
		gdi[2].br[i] = 0;
		gdi[3].br[i] = 0;
		gdi[4].br[i] = 0;
		gdi[5].br[i] = 0;
		i++;
	}
	
	gdi[0].br[tmp1] = gdi[0].brAll;
	gdi[1].br[(n1%100)/10]=gdi[1].brAll;
	gdi[2].br[(n1%10)]=gdi[2].brAll;
	
	gdi[3].br[tmp2] = gdi[3].brAll;
	gdi[4].br[(n2%100)/10]=gdi[4].brAll;
	gdi[5].br[(n2%10)]=gdi[5].brAll;

}

void setHwStringToInds(const char* str)
{
	int size = sizeof(str);
	int i = 0;

	while(i<11)
	{
		gdi[0].br[i] = 0;
		gdi[1].br[i] = 0;
		gdi[2].br[i] = 0;
		gdi[3].br[i] = 0;
		gdi[4].br[i] = 0;
		gdi[5].br[i] = 0;
		i++;
	}
	i = 0;
	while((i<size)&&(i<6))
	{
		if((str[i]>48)&&(str[i]<58))gdi[i].br[str[i]-48] = gdi[i].brAll;
		i++;
	}

}
//Обновляем катоды в соответствии с их яркостью
 void refrIndCaths(unsigned char index,unsigned char tmrBr)
{
	if(gdi[index].br[0]>tmrBr) gdi[index].codeC|=LCat[0]; else gdi[index].codeC&=~LCat[0];
	if(gdi[index].br[1]>tmrBr) gdi[index].codeC|=LCat[1]; else gdi[index].codeC&=~LCat[1];
	if(gdi[index].br[2]>tmrBr) gdi[index].codeC|=LCat[2]; else gdi[index].codeC&=~LCat[2];
	if(gdi[index].br[3]>tmrBr) gdi[index].codeC|=LCat[3]; else gdi[index].codeC&=~LCat[3];
	if(gdi[index].br[4]>tmrBr) gdi[index].codeC|=LCat[4]; else gdi[index].codeC&=~LCat[4];
	if(gdi[index].br[5]>tmrBr) gdi[index].codeC|=LCat[5]; else gdi[index].codeC&=~LCat[5];
	if(gdi[index].br[6]>tmrBr) gdi[index].codeC|=LCat[6]; else gdi[index].codeC&=~LCat[6];
	if(gdi[index].br[7]>tmrBr) gdi[index].codeC|=LCat[7]; else gdi[index].codeC&=~LCat[7];
	if(gdi[index].br[8]>tmrBr) gdi[index].codeC|=LCat[8]; else gdi[index].codeC&=~LCat[8];
	if(gdi[index].br[9]>tmrBr) gdi[index].codeC|=LCat[9]; else gdi[index].codeC&=~LCat[9];
	if(gdi[index].br[10]>tmrBr) gdi[index].codeC|=LCat[10]; else gdi[index].codeC&=~LCat[10];
}
//Зажигаем индикатор
//Яркость регулируется параметром brAll и 
//параметром br[] для катодов по отдельности
void fireIndCycle()
{
	static unsigned char timerBright = 0;//
	static unsigned char currInd = 0;

	refrIndCaths(currInd,timerBright);
	//Зажигаем текущий индикатор
	if(gdi[currInd].codeC)fireCurrInd(currInd);else setCath(0x0000);
	
	timerBright++;
	
	//Переходим к следующему индикатору
	if(timerBright>L_MAX_YARK)
	{
		timerBright = 0;
		currInd++;
		if(currInd>5)currInd = 0;
		setAnode(0x0000);
	}
}

//Зажигание индикаторов в режиме "пламени"
void fireIndFireEffCycle()
{
}
//Автояркость для gdi
void autoBright(void)
{
	unsigned short int adcBrVal = 0;
	adcBrVal =  getLightSensValue();

	if(adcBrVal<=3)adcBrVal = 3;
	if(adcBrVal>59)adcBrVal = 59;
	yarkAll = adcBrVal;
	if(flgFrEff) fireYark = (adcBrVal/20);
	else
	setGdiBrightAll(yarkAll);
	setLedAutoBright(adcBrVal);
}


/*~~~~~~~~~~~~~~~~~~~~~Стандартный эффект плавного переключения~~~~~~~~~~~~~~~~~~~~~~~~~~
* GDI_EFF_PL1 3
* Установка значений*/
void setIndCathFade(register unsigned char ind,register unsigned char cath)
{
	unsigned char tmp;
	tmp = gdi[ind].prevC;
	if(gdi[ind].currC!=cath) gdi[ind].prevC = gdi[ind].currC;
	else{ gdi[ind].br[gdi[ind].currC] = gdi[ind].brAll; return; }

	gdi[ind].br[tmp] = 0;
	gdi[ind].currC = cath;
	gdi[ind].cathTm = 0;
	flgFTsk|= tskFArr[ind]; //Устанавливаем флаг для текущего индикатора
}
/*Плавное изменение катодов*/
 void cathFade(unsigned char ind)
{
	unsigned char flgTskRdy = 0;
	gdi[ind].cathTm++;
	if((gdi[ind].cathTm%15)==0)
	{
		if(gdi[ind].br[gdi[ind].prevC]>0) gdi[ind].br[gdi[ind].prevC]--;
		else flgTskRdy|=0xf0;
		if(gdi[ind].br[gdi[ind].currC]<gdi[ind].brAll) gdi[ind].br[gdi[ind].currC]++;
		else flgTskRdy|=0x0f;
		if(flgTskRdy == 0xff)flgFTsk&=~tskFArr[ind];
	}
}
/*Плавное гашение предыдущего катода резкое зажигание текущего*/
void cathFadePrv(unsigned char ind)
{
	unsigned char flgTskRdy = 0;
	gdi[ind].cathTm++;
	if((gdi[ind].cathTm%15)==0)
	{
		if(gdi[ind].br[gdi[ind].prevC]>0) gdi[ind].br[gdi[ind].prevC]--;
		else flgTskRdy|=0xf0;

		gdi[ind].br[gdi[ind].currC] = gdi[ind].br[gdi[ind].currC] = gdi[ind].brAll; flgTskRdy|=0x0f;

		if(flgTskRdy == 0xff)flgFTsk&=~tskFArr[ind];
	}
}
 void cathFadeTwo(unsigned char ind)
{
	unsigned char flgTskRdy = 0;
	gdi[ind].cathTm++;
	if((gdi[ind].cathTm%7)==0)
	{
		if(gdi[ind].br[gdi[ind].prevC]>0) gdi[ind].br[gdi[ind].prevC]--;
		else
		{
			flgTskRdy|=0xf0;
			if(gdi[ind].br[gdi[ind].currC]<gdi[ind].brAll) gdi[ind].br[gdi[ind].currC]++;
			else
			flgTskRdy|=0x0f;
		}
		if(flgTskRdy == 0xff)flgFTsk&=~tskFArr[ind];
	}
}


/*~~~~~~~~~~~~~~~~~~~~~Эффект с перебором всех цифр~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* GDI_EFF_RUE 1
* Установка значений*/
void setIndCathRue(register unsigned char ind,register unsigned char cath)
{
	unsigned char tmp;

	tmp = gdi[ind].prevC;
	if(gdi[ind].currC!=cath)
		gdi[ind].prevC = gdi[ind].currC;
	else
	{
		gdi[ind].br[gdi[ind].currC] = gdi[ind].brAll;
		return;
	}
	gdi[ind].br[tmp] = 0;
	gdi[ind].currC = cath;
	gdi[ind].cathTm = 0;
	flgFTsk|= tskFArr[ind]; //Устанавливаем флаг для текущего индикатора
	gdi[ind].nRueCat = in14CathsPos[cath]+1;
}
/*Изменение очередью*/
 void cathRue(unsigned char ind)
{
	unsigned char tmp;
	register unsigned char i = 0;

	if(gdi[ind].nRueCat>9)
		gdi[ind].nRueCat = 0;

	tmp = in14CathsPos[gdi[ind].nRueCat];

	if(gdi[ind].cathTm<MAX_FADE_TIMER_VALUE)
	{
		gdi[ind].cathTm++;
		if((gdi[ind].cathTm%15)==0)
		{
			while(i<11)
			{
				gdi[ind].br[i]= 0;
				i++;
			}
			gdi[ind].br[in14CathsPos[gdi[ind].nRueCat]] = gdi[ind].brAll;
			if(tmp == gdi[ind].currC)
			{
				return;
			}
				gdi[ind].nRueCat++;
		}
	}
}

 void setIndCathRueS(register unsigned char ind,register unsigned char cath)
{
	unsigned char tmp;

	tmp = gdi[ind].prevC;
	if(gdi[ind].currC!=cath)
		gdi[ind].prevC = gdi[ind].currC;
	else
	{
//gdi[ind].br[gdi[ind].currC] = gdi[ind].brAll;
	//	return;
	}
	gdi[ind].br[tmp] = 0;
	gdi[ind].currC = cath;
	gdi[ind].cathTm = 0;
	flgFTsk|= tskFArr[ind]; //Устанавливаем флаг для текущего индикатора
	gdi[ind].nRueCat = in14CathsPos[cath]+1;
}
/*Изменение очередью*/
 void cathRueS(unsigned char ind)
{
	unsigned char tmp;
	register unsigned char i = 0;

	if(gdi[ind].nRueCat>9)
		gdi[ind].nRueCat = 0;

	tmp = in14CathsPos[gdi[ind].nRueCat];

	if(gdi[ind].cathTm<MAX_RUES_VALUE)
	{
		gdi[ind].cathTm++;
		if((gdi[ind].cathTm%30)==0)
		{
			while(i<11)
			{
				gdi[ind].br[i]= 0;
				i++;
			}
			gdi[ind].br[in14CathsPos[gdi[ind].nRueCat]] = gdi[ind].brAll;
			if(tmp == gdi[ind].currC)
			{
				return;
			}
				gdi[ind].nRueCat++;
		}
	}
}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Простое переключение~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*GDI_EFF_STD 0
*Установка значений*/
 void setIndCath(register unsigned char ind,register unsigned char cath)
{

	if(gdi[ind].currC!=cath)
		gdi[ind].prevC = gdi[ind].currC;

	gdi[ind].currC = cath;
	memset(gdi[ind].br,0,10);
	gdi[ind].br[cath] = gdi[ind].brAll;
	gdi[ind].br[gdi[ind].prevC] = 0;//gdi[ind].brightness;
}
/* Переключение в cathFade()*/


/*~~~~~~~~~~~~~~~~~~~~~~Режим переключения когда зажигаются все катоды~~~~~~~~~~~~~~~~~~~~~~~
*GDI_EFF_ALL 4
*Установка значений в setIndCathFade()
*/
/*Переключение*/
 void cathFadeAll(unsigned char ind)
{
	register unsigned char i = 0;
	if(gdi[ind].cathTm<MAX_FADE_TIMER_VALUE)
	{
		gdi[ind].cathTm++;
		if((gdi[ind].cathTm%7)==0)
		{
			if(gdi[ind].cathTm<450)
			{
				while(i<10)
				{
					if(i!=gdi[ind].prevC)
					{
						if(gdi[ind].br[i]<gdi[ind].brAll)
							gdi[ind].br[i]++;
					}
							i++;
				}

			gdi[ind].br[gdi[ind].prevC]--;
			if(gdi[ind].br[gdi[ind].prevC]>=gdi[ind].brAll)
					gdi[ind].br[gdi[ind].prevC] = 0;
			}
			else
			{
				while(i<10)
				{
					if(i!=gdi[ind].currC)
					{
						if(gdi[ind].br[i]!=0)
							gdi[ind].br[i]--;

					}
					i++;
				}

				gdi[ind].br[gdi[ind].currC]++;

				if(gdi[ind].br[gdi[ind].currC]>gdi[ind].brAll)
					gdi[ind].br[gdi[ind].currC] = gdi[ind].brAll;
			}
		}
		if(gdi[ind].cathTm>900)
			flgFTsk&=~tskFArr[ind];
	}
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Эффект с двумя катодами одновременно~~~~~~~~~~~~~~~~~~~~~
*GDI_EFF_2CF 2
*Установка значений*/
 void setIndCathTwoCaths(unsigned char ind,register unsigned char cath)
{

	unsigned char tmp;
	tmp = gdi[ind].prevC;
	if(gdi[ind].currC!=cath)
		gdi[ind].prevC = gdi[ind].currC;
	else
	{
		gdi[ind].br[gdi[ind].currC] = gdi[ind].brAll;
		return;
	}

	gdi[ind].br[tmp] = 0;
	gdi[ind].currC = cath;
	gdi[ind].br[gdi[ind].currC] = gdi[ind].brAll;
	gdi[ind].br[gdi[ind].prevC] = gdi[ind].brAll;
	flgFTsk|= tskFArr[ind]; //Устанавливаем флаг для текущего индикатора
	gdi[ind].cathTm = 0;
}
/*Переключение*/
 void cathTwoCaths(unsigned char ind)
{
	gdi[ind].cathTm++;
	if(gdi[ind].cathTm>=MAX_FADE_TIMER_VALUE)
			{gdi[ind].br[gdi[ind].prevC] = 0; flgFTsk&=~tskFArr[ind];}
}


//Otpravka znachenija neposredstvenno na gdi
 void fireCurrInd(unsigned char currIndex)
{
	setAnode(0x0000);
	setCath(gdi[currIndex].codeC);//pravka
	setAnode(LAn[currIndex]);
}

//Dobavlenie tekushego catoda k tekushemu indicatoru
 void fireCurrIndCath(unsigned char ind, unsigned char cath){ gdi[ind].codeC|=LCat[cath]; }



//Vikluchenie vseh indicatorov
 void offInds(){ setAnode(0x0000); setCath(0x0000);	}

 void offCurrIndCath(unsigned char ind, unsigned char cath){ gdi[ind].br[cath] = 0; }
void handleIndFire()
{
	static unsigned short blinkTimer = 0;
	static unsigned short int i = 0,j = FIRE_COEF_CNT-27, k = FIRE_COEF_CNT-12, l =FIRE_COEF_CNT-66, m = FIRE_COEF_CNT-150, n = FIRE_COEF_CNT-100;
	blinkTimer++;
	if(blinkTimer >= 50)
	{	
gdi[0].br[gdi[0].currC] = fCfArr[i];gdi[0].brAll = fCfArr[i]; //fireEffP[fireYark][i]; gdi[0].brAll = fireEffP[fireYark][i];
		gdi[1].br[gdi[1].currC] = fCfArr[j];gdi[1].brAll = fCfArr[j];//fireEffP[fireYark][i]; gdi[1].brAll = fireEffP[fireYark][i];
		gdi[2].br[gdi[2].currC] = fCfArr[k];gdi[2].brAll = fCfArr[k];//fireEffP[fireYark][i];gdi[2].brAll = fireEffP[fireYark][i];
		gdi[3].br[gdi[3].currC] = fCfArr[l];gdi[3].brAll = fCfArr[l];//fireEffP[fireYark][i];gdi[3].brAll = fireEffP[fireYark][i];
		gdi[4].br[gdi[4].currC] = fCfArr[m];gdi[4].brAll = fCfArr[m];//fireEffP[fireYark][i];gdi[4].brAll = fireEffP[fireYark][i];
		gdi[5].br[gdi[5].currC] = fCfArr[n];gdi[5].brAll = fCfArr[n];//fireEffP[fireYark][i];gdi[5].brAll = fireEffP[fireYark][i];
		i++; j++; k++; l++; m++; n++;
		if(i>=FIRE_COEF_CNT)i=0;if(j>=FIRE_COEF_CNT)j=0;if(k>=FIRE_COEF_CNT)k=0;
		if(l>=FIRE_COEF_CNT)l=0;if(m>=FIRE_COEF_CNT)m=0;if(n>=FIRE_COEF_CNT)n=0;
		blinkTimer = 0;
	}
}

void handleIndFireAll()
{
		static unsigned short blinkTimer = 0;
	static unsigned short int i = 0;
	blinkTimer++;
	if(blinkTimer >= 50)
	{	
		gdi[0].br[gdi[0].currC] = fCfArr[i];gdi[0].brAll = fCfArr[i]; //fireEffP[fireYark][i]; gdi[0].brAll = fireEffP[fireYark][i];
		gdi[1].br[gdi[1].currC] = fCfArr[i];gdi[1].brAll = fCfArr[i];//fireEffP[fireYark][i]; gdi[1].brAll = fireEffP[fireYark][i];
		gdi[2].br[gdi[2].currC] = fCfArr[i];gdi[2].brAll = fCfArr[i];//fireEffP[fireYark][i];gdi[2].brAll = fireEffP[fireYark][i];
		gdi[3].br[gdi[3].currC] = fCfArr[i];gdi[3].brAll = fCfArr[i];//fireEffP[fireYark][i];gdi[3].brAll = fireEffP[fireYark][i];
		gdi[4].br[gdi[4].currC] = fCfArr[i];gdi[4].brAll = fCfArr[i];//fireEffP[fireYark][i];gdi[4].brAll = fireEffP[fireYark][i];
		gdi[5].br[gdi[5].currC] = fCfArr[i];gdi[5].brAll = fCfArr[i];//fireEffP[fireYark][i];gdi[5].brAll = fireEffP[fireYark][i];
		i++; 
		if(i>=FIRE_COEF_CNT)i=0;
		blinkTimer = 0;
	}
}

void gdiEffSet(unsigned char ind, unsigned char val)
	{

		if(val == GDI_OFF){ gdi[ind].brAll = 0; setIndCathEff[currGdiEff[ind]](ind,gdi[ind].currC);}
		else
		setIndCathEff[currGdiEff[ind]](ind,val);
	}

void setSecView(unsigned char flg)
{
	switch (flg)
	{
		case SEC_CATHODE:{gdi[4].brAll = gdi[0].brAll; gdi[5].brAll = gdi[0].brAll; break;}
		case SEC_OFF:{gdi[4].brAll = 0; gdi[5].brAll = 0;break;}//gdiVal[4] = GDI_OFF; gdiVal[5] = GDI_OFF; break;}
		case SEC_COMMA1:
		{
				if(tmdt.sec%2){offCurrIndCath(4, gdi[4].currC); onIndComma(4);offIndComma(5); offCurrIndCath(5, gdi[5].currC);}
				else{offCurrIndCath(5, gdi[5].currC); onIndComma(5);offIndComma(4);offCurrIndCath(4, gdi[4].currC);}
				break;
		}
		default: break;
	}
}
