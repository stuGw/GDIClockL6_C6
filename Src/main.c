/*
 * main.c
 *
 *  Created on: Dec 20, 2021
 *      Author: vit
 */


#include "main.h"
#include "stm32f1xx.h"

#include "system.h"
#include "gdi.h"
#include "time.h"
#include "ws2812.h"
#include "ds3231.h"
#include <stdlib.h>
#include "menu.h"

//v1.0.5
extern unsigned char flagAutoYarkost;
char statusb = 0;
extern volatile char flgLongButt;
unsigned int sysCounter = 0;
volatile unsigned short int flgR = 0;
volatile unsigned short int flgD = 0;
unsigned short int rueInterv[6] = {1,1,5,15,30,59};
unsigned  short int dataInterv[4] = {20,1,3,5};
unsigned short int rueTmr =0;
volatile unsigned short int flgC =1;
volatile unsigned int stmr = 0;
volatile unsigned int ctmr = 0;
volatile char up, down;
volatile struct Button bt0,bt2,bt1;//bt0 bt2 - buttons, bt1 - encoder button
volatile struct Button *be;
char flgShow = FLG_SHOW_TM;
volatile unsigned char gdiVal[6];
char gdiFlgChn[6] = {0,0,0,0,0,0};
extern unsigned char indBl;
extern unsigned char ledEffType;
extern volatile unsigned char flagConfMode;//Флаг режима настройки
extern unsigned char flagTmDtConf;//Флаг настройки времени/даты()
unsigned char flagSecType = SEC_CATHODE;
extern  struct I2CDev i2c;
extern unsigned char bf[];
//extern unsigned char i2cAM;//Adress&Mode for i2c dev
//extern unsigned char i2cIter;
//extern unsigned char i2cData[10];
//extern unsigned char i2cMA;//adress mem in dev
//extern unsigned char i2cRcvCnt;

extern unsigned short encoderCounter;
extern unsigned short encoderMaxValue;

struct timeNDate tmdt;
unsigned char secFlag = 0;
extern volatile unsigned char flgWSTransfer;
char fireYark = 2;



unsigned char flgFrEff = 0;
volatile unsigned char flgRueOn = 1;
unsigned char flagFireEff = 0;
unsigned char flgTmRefresh = 0;
unsigned char flgLedEff = 0;


extern unsigned char effBrightCoef;
extern int confTm;
extern unsigned short int prevConfVals[];
/*~~~~~~~~~~~~~~~~~~INTERRUPTS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//~~~~~~~~~~~~~~~~~~~~~~~Прерывание ПДП WSDMA~~~~~~~~~~~~~~~~~~~~~~~~~~~
//При пересылке последнего значения для WS в таймер в НЕкольцевом режиме
void DMA1_Channel6_IRQHandler(void)
{
	 stopWSDMA();
	 DMA1->IFCR|=DMA_IFCR_CGIF6;
}

//Таймер для реализации протокола ws2812
//После передачи всех данных - остановить таймер
void TIM3_IRQHandler(void)
{
	static int counter = 0;
	if(counter>=LED_HW_LENGTH)
	{
		stopTimer3();
	startLightSensDMA();//Запускаем канал LightSens
		flgWSTransfer = WS_TRANSFER_READY;//Передача закончена - можно передавать заново
		counter = 0;
	}
	else
		counter++;
	TIM3->SR &= ~TIM_SR_UIF; //Clean UIF Flag
}

//Таймер эффектов
void TIM1_UP_IRQHandler(void)
{
	//Если меняется какой либо индикатор в режиме fade
	gdiEffHandle();
	TIM1->SR &= ~TIM_SR_UIF; //Clean UIF Flag
}

//Основной цикл обновления индикаторов, за одно прерывание зажигается один индикатор
//Один цикл обновления всех 6 индикаторов - 360 прерываний
//Регулировка яркости ШИМом
//Обновление всех индикаторов происходит за 360 прерываний
//6 индикаторов, на каждый по 60
//60-размерность ШИМ
/*
			1         2						3					4					5						5
|      _____|          |          |          |          |          |
______|
*/
void TIM2_IRQHandler(void)
{
	//End configure
	if(flagFireEff)
		fireIndFireEffCycle();
	else
		fireIndCycle();
	TIM2->SR &= ~TIM_SR_UIF; //Clean UIF Flag
}


//~~~~~~~~~~~~~~~~Системный таймер за одну секунду SYSTMR_FREQ тиков~~~~~~~~~~~~~
void SysTick_Handler(void)
{

	sysCounter++;
	if(flgR)stmr++;
	if(flgC)ctmr++;
	if(bt0.ipt) bt0.tmd++;
	if(bt0.st) bt0.tm++;
	if(bt2.ipt) bt2.tmd++;
	if(bt2.st) bt2.tm++;
	if(bt1.ipt) bt1.tmd++;
	if(bt1.st) bt1.tm++;

	if(flagConfMode == CONF_MODE_ON)
		confTm++;

}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~Прерывания от кнопок~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Правая для ин-17
void EXTI0_IRQHandler(void)
{
	EXTI->PR|=0x01;
		if(!(bt0.buttBit&RG_BUTT_INPREG)){bt0.ipt = 1; bt0.st = 1;}
}
//Кнопка энкодера
void EXTI1_IRQHandler(void)
{
		EXTI->PR|=0x02;
		if(!(bt1.buttBit&RG_BUTT_INPREG)){bt1.ipt = 1; bt1.st = 1;}


}
//Левая кнопка ин-17

void EXTI2_IRQHandler(void)
{
	EXTI->PR|=0x04;
	//butt to down
	if(!(bt2.buttBit&RG_BUTT_INPREG)){bt2.ipt = 1; bt2.st = 1;}
//flgC=!flgC;
}


uint8_t counterEnco = 0;
uint8_t oldEncoState = 3;
uint8_t currEncoState = 0;
//encoder exti
void EXTI9_5_IRQHandler(void)
{
	//counterEnco++;
	uint8_t newEncoState = 3;
	if((EXTI->PR & 0x40) || (EXTI->PR & 0x80))
	{

		newEncoState = getEncoderPinsValue();
		currEncoState = newEncoState;
		// Check transition
		if(newEncoState == 3)
		{
			if(oldEncoState == 1){  if(encoderCounter >= encoderMaxValue)encoderCounter = 0; else encoderCounter++; }
			else if(oldEncoState == 2){  if(encoderCounter == 0) encoderCounter = encoderMaxValue; else encoderCounter--; }
		}

		//if(encoderCounter>encoderMaxValue) encoderCounter = 0;
		oldEncoState = newEncoState;

		if(EXTI->PR & 0x40) EXTI->PR |= 0x40;
		if(EXTI->PR & 0x80) EXTI->PR |= 0x80;

		/*if (oldEncoState == 3 && newEncoState == 2) {        // 3 -> 2 transition
				if(encoderCounter >= encoderMaxValue)encoderCounter = 0; else encoderCounter++;
			} else if (oldEncoState == 2 && newEncoState == 0) { // 2 -> 0 transition
				if(encoderCounter >= encoderMaxValue)encoderCounter = 0; else encoderCounter++;
			} else if (oldEncoState == 0 && newEncoState == 1) { // 0 -> 1 transition
				if(encoderCounter >= encoderMaxValue)encoderCounter = 0; else encoderCounter++;
			} else if (oldEncoState == 1 && newEncoState == 3) { // 1 -> 3 transition
				if(encoderCounter >= encoderMaxValue)encoderCounter = 0; else encoderCounter++;
			} else if (oldEncoState == 3 && newEncoState == 1) { // 3 -> 1 transition
				if(encoderCounter == 0) encoderCounter = encoderMaxValue; else encoderCounter--;
			} else if (oldEncoState == 1 && newEncoState == 0) { // 1 -> 0 transition
				if(encoderCounter == 0) encoderCounter = encoderMaxValue; else encoderCounter--;
			} else if (oldEncoState == 0 && newEncoState == 2) { // 0 -> 2 transition
				if(encoderCounter == 0) encoderCounter = encoderMaxValue; else encoderCounter--;
			} else if (oldEncoState == 2 && newEncoState == 3) { // 2 -> 3 transition
				if(encoderCounter == 0) encoderCounter = encoderMaxValue; else encoderCounter--;
			}*/
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~Прерывание от RTC~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Тактируется от ds3231
void RTC_IRQHandler(void)
{
	static int tmpCounter = 0;
	tmpCounter++;
	//ledDbgTuggle();
	//GPIOB->BRR|=0x00000002;
	//NVIC_ClearPendingIRQ(RTC_IRQn);
	if(RTC->CRL&RTC_CRL_SECF){ RTC->CRL&=~RTC_CRL_SECF;}
//	if(RTC->CRL&RTC_CRL_OWF) RTC->CRL&=~RTC_CRL_OWF;
//set2NumsToIndsEff(down,up);

	if(flagConfMode){return;}

	secFlag = 1;
	if((flgRueOn == 5)&&(tmdt.sec == 59))flgR = 1;
	else if(flgRueOn&&(tmdt.sec == 0)&&!(tmdt.min%rueInterv[flgRueOn]))flgR = 1;
	//if(flgRueOn){rueTmr++;if(rueTmr == rueInterv[1])flgR = 1;rueTmr = 0;}

	gdiEffSet(0,gdiVal[0]);
	gdiEffSet(1,gdiVal[1]);
	gdiEffSet(2,gdiVal[2]);
	gdiEffSet(3,gdiVal[3]);
  gdiEffSet(4,gdiVal[4]);
	gdiEffSet(5,gdiVal[5]);




	//GPIOB->BSRR|=0x00000002;
}

void I2C1_EV_IRQHandler(void)
{
	volatile uint16_t sr1,sr2;


	sr1 = I2C1->SR1;
	if(sr1& I2C_SR1_SB)//start ok/EV5
	{
		if(i2c.flg == I2C_NEED_TRM){ I2C1->DR = RTC_ADR|DS3231_WRITE;}
		if(i2c.flg == I2C_NEED_RCV){ I2C1->DR = RTC_ADR|DS3231_READ;}
	}
	if(sr1& I2C_SR1_ADDR){ sr2 = I2C1->SR2; if(i2c.flg == I2C_NEED_TRM) I2C1->DR = RTC_SEC_ADR;return;	}

	if(sr1 & I2C_SR1_BTF)
	{
		if(i2c.flg == I2C_NEED_TRM){ i2c.flg = I2C_NEED_RCV; 	I2C1->CR1|=I2C_CR1_START; }//repeated start}
	}
	//if dma not used read in interrupts
	#ifndef I2C_IPTDMA
		if(sr1& I2C_SR1_RXNE)
		{
			if(i2c.flg == I2C_NEED_RCV)
				{
					i2c.devData[i2c.devIter] = I2C1->DR; i2c.devIter++;
				__disable_irq();
				if(i2c.devIter == i2c.devRcvCnt-2){ I2C1->CR1&=~I2C_CR1_ACK; I2C1->CR1|=I2C_CR1_STOP; }
			  if(i2c.devIter == i2c.devRcvCnt-1){i2c.devIter = 0; i2c.rcvRdy = 1;}
			  __enable_irq();
		}
	}
	#endif
	//__enable_irq();
}

//with DMA
/*void I2C1_EV_IRQHandler(void)
{
	volatile uint16_t sr1,sr2;
	//__disable_irq();
	sr1 = I2C1->SR1;
	if(sr1& I2C_SR1_SB)//start ok/EV5
	{
		if(i2c.flg == I2C_NEED_TRM){ I2C1->DR = RTC_ADR|DS3231_WRITE;}
		if(i2c.flg == I2C_NEED_RCV){ I2C1->DR = RTC_ADR|DS3231_READ;}
	}
	if(sr1& I2C_SR1_ADDR){ sr2 = I2C1->SR2; if(i2c.flg == I2C_NEED_TRM) I2C1->DR = RTC_SEC_ADR;__enable_irq();return;	}

	if(sr1 & I2C_SR1_BTF)
	{
		if(i2c.flg == I2C_NEED_TRM){ i2c.flg = I2C_NEED_RCV; 	I2C1->CR1|=I2C_CR1_START;}//repeated start}
	}
	//__enable_irq();
}
*/
//Get data from ds3231, find this in i2c.devData arr
void DMA1_Channel7_IRQHandler(void)
{
	//NACK generated automically
	if(DMA1->ISR&DMA_ISR_TCIF7)
	{
		DMA1_Channel7->CCR = 0;//disable dma ch7
		DMA1->IFCR|=DMA_IFCR_CGIF7; I2C1->CR1|=I2C_CR1_STOP; i2c.rcvRdy = 1;}
	  i2c.rcvRdy = 1;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//fault HSE
void NMI_Handler(void)
{
    /* ???????? ???? ??????? ???????? CSS */
    if (RCC->CIR & RCC_CIR_CSSF) RCC->CIR|=RCC_CIR_CSSC;
}
//Переключение флага показа времени/даты
void handleTmDtFlag()
{
	static unsigned short int dtPer = 0;
	if(flgShow == FLG_SHOW_TM)
	{

		if((tmdt.hour == SHOW_DT_H)&&(tmdt.min == SHOW_DT_M )&&(tmdt.sec == SHOW_DT_S)){ flgShow = FLG_SHOW_DT;}
		if((tmdt.hour == SHOW_DT_H2)&&(tmdt.min == SHOW_DT_M2)&&(tmdt.sec == SHOW_DT_S2)) flgShow = FLG_SHOW_DT;

		if((flgD == 0)&&!(tmdt.min%dataInterv[flgD])&&tmdt.sec == 0)flgShow = FLG_SHOW_DT;//every 20 mins
		if(flgD&&!(tmdt.hour%dataInterv[flgD])&&(tmdt.sec == 0)&&(tmdt.min == 0))flgShow = FLG_SHOW_DT;//every 1/3/5 hours
	}
	else
	{
		dtPer++;

		if(dtPer>DELAY_DT){dtPer = 0;flgShow = FLG_SHOW_TM;}
	}
}
void handleIndBlink(short int ind)
{
	static unsigned short blinkTimer = 0;
	static signed char sign = 1;
	blinkTimer++;
	if(blinkTimer >= 17)
	{
		if(gdi[ind].br[gdi[ind].currC]>=59)sign = -1;
		if(gdi[ind].br[gdi[ind].currC]<=1) sign = 1;
		gdi[ind].br[gdi[ind].currC]+=sign;
		if(gdi[ind+1].br[gdi[ind+1].currC]>=59)sign = -1;
		if(gdi[ind+1].br[gdi[ind+1].currC]<=1) sign = 1;
		gdi[ind+1].br[gdi[ind+1].currC]+=sign;
		blinkTimer = 0;
	}
}

void handleRue(char on)
{
		if(on){
			if(stmr <10){currGdiEff[0] = 7; setIndCathRueS(0,gdiVal[0]);}
			if(stmr == 333){currGdiEff[1] = 7; setIndCathRueS(1,gdiVal[1]);currGdiEff[0] = cge;}
			if(stmr == 667){currGdiEff[2] = 7; setIndCathRueS(2,gdiVal[2]);currGdiEff[1] = cge;}
			if(stmr == 999){currGdiEff[3] = 7; setIndCathRueS(3,gdiVal[3]); currGdiEff[2] = cge;}
			if(stmr == 1332){currGdiEff[4] = 7; setIndCathRueS(4,gdiVal[4]); currGdiEff[3] = cge;}
			if(stmr == 1665){currGdiEff[5] = 7; setIndCathRueS(5,gdiVal[5]);currGdiEff[4] = cge;}
			if(stmr == 1999){ currGdiEff[5] = cge;flgR = 0; stmr = 0;}
		}
}
void handleComma()
{
	if(ctmr <20){gdi[0].br[10] = gdi[0].brAll;gdi[1].br[10] = 0;}
	if(ctmr == 200){gdi[1].br[10] = gdi[1].brAll;gdi[0].br[10] = 0;}
	if(ctmr == 400){gdi[2].br[10] = gdi[2].brAll;gdi[1].br[10] = 0;}
	if(ctmr == 600){gdi[3].br[10] = gdi[3].brAll;gdi[2].br[10] = 0;}
	if(ctmr == 800){gdi[4].br[10] = gdi[4].brAll;;gdi[3].br[10] = 0;}
	if(ctmr == 1000){gdi[5].br[10] = gdi[5].brAll;;gdi[4].br[10] = 0;}
	if(ctmr == 1200){gdi[4].br[10] = gdi[4].brAll;;gdi[5].br[10] = 0;}
	if(ctmr == 1400){gdi[3].br[10] = gdi[3].brAll;;gdi[4].br[10] = 0;}
	if(ctmr == 1600){gdi[2].br[10] = gdi[2].brAll;;gdi[3].br[10] = 0;}
	if(ctmr ==1800){gdi[1].br[10] = gdi[1].brAll;;gdi[2].br[10] = 0;}
	if(ctmr > 1995){ctmr = 0;}
	//if(ctmr == 990){gdi[0].zptR = 0; gdi[1].zptR = 50;}
}
void changeTmDt()
{
	static char w1 = 1,w2 = 0;
	if(flagTmDtConf)return;

	if(flgShow == FLG_SHOW_TM)
	{
		w1 = 1;
		if(w2){flgR = 1;w2 = 0;}
		gdiVal[0] = tmdt.hour/10; gdiVal[1] = tmdt.hour%10;
		gdiVal[2] = tmdt.min/10; gdiVal[3] = tmdt.min%10;
		 gdiVal[4] = tmdt.sec/10; gdiVal[5] = tmdt.sec%10;
		setSecView(flagSecType);
	}
	if(flgShow == FLG_SHOW_DT)
	{
		if(w1){flgR = 1;w1 = 0;w2 = 1;}
		gdiVal[0] = tmdt.day/10; gdiVal[1] = tmdt.day%10;
		gdiVal[2] = tmdt.month/10; gdiVal[3] = tmdt.month%10;
		gdiVal[4] = tmdt.year/10; gdiVal[5] = tmdt.year%10;
		setSecView(SEC_CATHODE);

	}

	//Запись тек. времени в значения параметров для того, чтобы при входе
	//в режим настройки отображались последние значения времени и даты

		prevConfVals[CONF_TM_H] = tmdt.hour;
	  prevConfVals[CONF_TM_M] = tmdt.min;
		prevConfVals[CONF_TM_S] = tmdt.sec;
		prevConfVals[CONF_DT_D] = tmdt.day;
		prevConfVals[CONF_DT_M] = tmdt.month;
		prevConfVals[CONF_DT_Y] = tmdt.year;
}

void handleTickEvs()
{
/*static int cntr = 0;
	cntr++;
	if(cntr>1000){secFlag = 1;

 gdiEffSet(0,gdiVal[0]);
	 gdiEffSet(1,gdiVal[1]);
	 gdiEffSet(2,gdiVal[2]);
	 gdiEffSet(3,gdiVal[3]);
   gdiEffSet(4,gdiVal[4]);
	 gdiEffSet(5,gdiVal[5]);
	cntr = 0;}*/
//handleRue();

	handleTmDtFlag();
	if(indBl)handleIndBlink(indBl-1);
	if(ledEffType)flgLedEff = 1;//ledEff(ledEffType);
	if(flgFrEff == FIRE_EFF1) handleIndFire();
	if(flgFrEff == FIRE_EFF2) handleIndFireAll();

}

int main(void)
{
	up = 0;
	ctmr = 0;
	down = 0;
	flgR = 1;
	flgC = 1;
	cge= GDI_EFF_PL1;
	currGdiEff[0] = cge;
	currGdiEff[1] = cge;
	currGdiEff[2] = cge;
	currGdiEff[3] = cge;
	currGdiEff[4] = cge;
	currGdiEff[5] = cge;
	sysInit();//Инициализация переферии
	initConfig();//Инициализация стартовых параметров
	initButtons(&bt0,&bt1,&bt2);
	//confAllLedsColor(362);
	confAllLedsBright(0);
	setAllLedsBright(0);
	setLedMode(LED_ONCE);
	I2C1->CR1|=I2C_CR1_PE;
//	startLightSensDMA();
	//ds3231StartOsc();
	//	i2c1Write(RTC_ADR,0x0e,&value,1);
	NVIC_EnableIRQ(EXTI2_IRQn);
		NVIC_EnableIRQ(EXTI1_IRQn);
	flagAutoYarkost = 1;
	while(1)
	{
		static unsigned int systm = 0;
		unsigned char tick = 0;
		//flagAutoYarkost = 0;
		//set2NumsToIndsEff(bt2.dn,bt2.up);
		 ////setHw2NumsToInds(oldEncoState,counterEnco);
		//setHw2NumsToInds(encoderCounter,currEncoState);

		//Перезаписываем время
		if(i2c.rcvRdy)
		{
			ledDbgTuggle();
			i2c.rcvRdy = 0; i2c.flg = I2C_FREE;
			timeFromHw(i2c.devData); dateFromHw(i2c.devData+3);
			secFlag = TM_READY;
			changeTmDt();
		}
		//ловим системный тик
		if(systm!=sysCounter){systm = sysCounter;tick = 1;}
		//Запуск приема времени
		if((flgWSTransfer == WS_TRANSFER_READY) && (secFlag == TM_WAIT) && (flagTmDtConf == CONF_NO_VALUE))
		{
			secFlag = TM_WORK;
			timeTick();
		}
		//Изменение значений диодов в зависимости от эффкта
		if((flgWSTransfer == WS_TRANSFER_READY)&&(secFlag == TM_READY)&&(flgLedEff == 1))
		{
			ledEff(ledEffType);
			flgLedEff = 0;
		}
		//Польз. интерфейс
		handleUI(be);//ENCODER CON
		//handleUIButt(&bt0,&bt2);
		//Автояркость
		if(flagAutoYarkost){autoBright();}
		//Выход из режима настройки по таймауту
	  if(confTm>CONF_LIFETIME)//ENCODER CONF
		{
			confTm = 0;flagTmDtConf = CONF_NO_VALUE;
			flagConfMode = CONF_MODE_OFF;indBl = 0;
			flgFrEff = prevConfVals[CONF_GD_FE];
		}
		//Обработка по системному тику
		if(tick)handleTickEvs();
		tick = 0;

		handleRue(flgR);
		if(flgC&&(!flagConfMode))handleComma();
	}
}
