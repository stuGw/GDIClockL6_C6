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
#include "settings.h"

//v1.0.5
extern unsigned char flagAutoYarkost;
char statusb = 0;
extern volatile char flgLongButt;
volatile unsigned int sysCounter = 0;
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
volatile unsigned char secFlag = 0;
extern volatile unsigned char flgWSTransfer;
char fireYark = 2;



unsigned char flgFrEff = 0;
volatile unsigned char flgRueOn = 1;
unsigned char flgTmRefresh = 0;
unsigned char flgLedEff = 0;


extern unsigned char effBrightCoef;
extern volatile int confTm;
extern unsigned char flgSetTm;//menu.c
extern unsigned short int prevConfVals[];
/*~~~~~~~~~~~~~~~~~~INTERRUPTS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//~~~~~~~~~~~~~~~~~~~~~~~Прерывание ПДП WSDMA~~~~~~~~~~~~~~~~~~~~~~~~~~~
//При пересылке последнего значения для WS в таймер в НЕкольцевом режиме:
//весь буфер (данные + RES) передан - останавливаем таймер и DMA
void DMA1_Channel6_IRQHandler(void)
{
	DMA1->IFCR = DMA_IFCR_CGIF6;
	wsTransferComplete();
}

//Таймер эффектов
void TIM1_UP_IRQHandler(void)
{
	//Если меняется какой либо индикатор в режиме fade
	TIM1->SR = ~TIM_SR_UIF; //Clean UIF Flag сразу (rc_w0): событие во время обработки не потеряется
	gdiEffHandle();
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
	TIM2->SR = ~TIM_SR_UIF; //Clean UIF Flag сразу (rc_w0): событие во время обработки не потеряется
	fireIndCycle();
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
//PB0 (bt0): кнопка энкодера при ENC_BT0; в режиме CONF_BUTT - левая кнопка (меню)
void EXTI0_IRQHandler(void)
{
	EXTI->PR = EXTI_PR_PR0;//Запись 1 сбрасывает только свою линию
		if(!(bt0.buttBit&RG_BUTT_INPREG)){bt0.ipt = 1; bt0.st = 1;}
}
//PB1 (bt1): кнопка энкодера при ENC_BT1
void EXTI1_IRQHandler(void)
{
		EXTI->PR = EXTI_PR_PR1;
		if(!(bt1.buttBit&RG_BUTT_INPREG)){bt1.ipt = 1; bt1.st = 1;}


}
//PB2 (bt2): кнопка энкодера при ENC_BT2; в режиме CONF_BUTT - правая кнопка (значение)

void EXTI2_IRQHandler(void)
{
	EXTI->PR = EXTI_PR_PR2;
	//butt to down
	if(!(bt2.buttBit&RG_BUTT_INPREG)){bt2.ipt = 1; bt2.st = 1;}
//flgC=!flgC;
}


uint8_t counterEnco = 0;
uint8_t oldEncoState = 3;
uint8_t currEncoState = 0;

#if (ENC_QSTEPS != 2) && (ENC_QSTEPS != 4)
#error "ENC_QSTEPS must be 2 or 4 (see devconfdef.h)"
#endif
//Положение покоя энкодера, в котором засчитывается щелчок
#if ENC_QSTEPS == 2
#define ENC_REST(s) (((s) == 3) || ((s) == 0))
#else
#define ENC_REST(s) ((s) == 3)
#endif

//Декодер энкодера по таблице переходов кода Грея (вывод PB6 - бит 0, PB7 - бит 1).
//Индекс: (старое состояние << 2) | новое. Один допустимый переход = +-1 (четверть периода),
//пропущенные состояния и повторы = 0. Дребезг возвращает сумму в прежнее значение и не
//дает ложных щелчков (+1 при 3->2 и -1 при обратном 2->3 компенсируются).
//Прямой ход: 3->2->0->1->3, обратный: 3->1->0->2->3.
static const signed char encDelta[16] = {
	 0, +1, -1,  0,	//из 0
	-1,  0,  0, +1,	//из 1
	+1,  0,  0, -1,	//из 2
	 0, -1, +1,  0	//из 3
};
static signed char encAcc = 0;//Накопленные переходы с последнего положения покоя

//encoder exti
void EXTI9_5_IRQHandler(void)
{
	uint32_t pr = EXTI->PR & (EXTI_PR_PR6 | EXTI_PR_PR7);
	if(pr)
	{
		uint8_t newEncoState;
		//Сбрасываем только свои флаги (запись 1) ДО чтения выводов: фронт, пришедший во время
		//обработки, вызовет новое прерывание, а не потеряется
		EXTI->PR = pr;
		newEncoState = getEncoderPinsValue();
		currEncoState = newEncoState;
		encAcc += encDelta[(oldEncoState << 2) | newEncoState];
		oldEncoState = newEncoState;
		//В положении покоя засчитываем щелчок. Порог на 1 меньше полного числа переходов:
		//один пропущенный переход не теряет щелчок. Незавершенный поворот (|encAcc| меньше порога)
		//отбрасывается.
		if(ENC_REST(newEncoState))
		{
			if(encAcc >= ENC_QSTEPS-1)
			{
				if(encoderCounter >= encoderMaxValue)encoderCounter = 0; else encoderCounter++;
			}
			else if(encAcc <= 1-ENC_QSTEPS)
			{
				if(encoderCounter == 0) encoderCounter = encoderMaxValue; else encoderCounter--;
			}
			encAcc = 0;
		}
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~Прерывание от RTC~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Тактируется от ds3231. Раз в секунду только запрашивает чтение времени; на лампы оно
//выводится из главного цикла сразу после чтения (showTimeOnInds)
void RTC_IRQHandler(void)
{
	if(RTC->CRL&RTC_CRL_SECF){ RTC->CRL&=~RTC_CRL_SECF;}
	if(flagConfMode){return;}
	secFlag = TM_WAIT;
}

void I2C1_EV_IRQHandler(void)
{
	volatile uint16_t sr1;


	sr1 = I2C1->SR1;
	if(sr1& I2C_SR1_SB)//start ok/EV5
	{
		if(i2c.flg == I2C_NEED_TRM){ I2C1->DR = RTC_ADR|DS3231_WRITE;}
		if(i2c.flg == I2C_NEED_RCV){ I2C1->DR = RTC_ADR|DS3231_READ;}
	}
	if(sr1& I2C_SR1_ADDR){ (void)I2C1->SR2; if(i2c.flg == I2C_NEED_TRM) I2C1->DR = RTC_SEC_ADR;return;	}

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
//Аварийный обработчик (сюда же попадают MemManage/BusFault/UsageFault и
//необработанные прерывания через Default_Handler в startup).
//Первым делом гасим лампы: иначе текущая лампа осталась бы гореть постоянным
//током (без мультиплексирования), что вредно для катодов.
void HardFault_Handler(void)
{
	__disable_irq();
	GPIOB->BRR = 0xFC00;//Аноды PB10..PB15
	GPIOA->BRR = 0x9FFC;//Катоды PA2..PA12, PA15
#ifdef USE_IWDG
	while(1) {}//Сторожевой таймер перезагрузит контроллер (можно посмотреть отладчиком)
#else
	NVIC_SystemReset();
#endif
}

//fault HSE (прерывание системы контроля тактирования CSS)
void NMI_Handler(void)
{
	if (RCC->CIR & RCC_CIR_CSSF)
	{
		RCC->CIR |= RCC_CIR_CSSC;
		//Кварц отказал: аппаратно включился HSI 8 МГц без PLL, и все тайминги (ШИМ ламп, WS2812,
		//I2C) стали неверными в 5 раз. Гасим лампы и перезагружаемся: при старте initSystemClock()
		//не дождется HSE и запустит PLL от HSI на те же 40 МГц.
		GPIOB->BRR = 0xFC00;//Аноды PB10..PB15
		GPIOA->BRR = 0x9FFC;//Катоды PA2..PA12, PA15
		NVIC_SystemReset();
	}
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

//Эффект перебора цифр: перебор идет по лампам слева направо, каждая следующая включается через
//~333 мс. Этапы отслеживаются счетчиком stage и сравниваются через >=, поэтому, если главный
//цикл пропустит нужную миллисекунду, эффект не зависнет (раньше сравнивали stmr == 333/667/...).
void handleRue(char on)
{
	static const unsigned short rueTime[6] = {333,667,999,1332,1665,1999};
	static unsigned char stage = 0;//Номер лампы, на которой сейчас идет перебор
	if(!on) return;
	if(stmr < 10){currGdiEff[0] = 7; setIndCathRueS(0,gdiVal[0]);}
	while((stage < GDI_COUNT) && (stmr >= rueTime[stage]))
	{
		//Перебор переходит на следующую лампу, текущая возвращается к обычному эффекту
		if(stage < GDI_COUNT-1){currGdiEff[stage+1] = 7; setIndCathRueS(stage+1,gdiVal[stage+1]);}
		currGdiEff[stage] = cge;
		stage++;
	}
	if(stage >= GDI_COUNT){stage = 0; flgR = 0; stmr = 0;}
}
//Маятник запятых: положение (номер лампы) меняется каждые 200 мс по кругу
//0,1,2,3,4,5,4,3,2,1 (6 ламп) или 0,1,2,3,2,1 (4 лампы). Полный цикл всегда 2 с, поэтому проход
//слева направо занимает 1 с и на 6, и на 4 лампах (шаг 200 мс при 6 лампах, ~333 мс при 4).
//Этап вычисляется по ctmr, а не ловится на точное значение: если главный цикл пропустит нужную
//миллисекунду, запятая все равно переместится при следующем проходе.
void handleComma()
{
#if GDI_COUNT == 6
	static const unsigned char commaPos[10] = {0,1,2,3,4,5,4,3,2,1};
#else
	static const unsigned char commaPos[6] = {0,1,2,3,2,1};
#endif
#define COMMA_STEPS ((unsigned char)(sizeof(commaPos)/sizeof(commaPos[0])))
#define COMMA_PERIOD_MS 2000u
#define COMMA_STEP_MS (COMMA_PERIOD_MS/COMMA_STEPS)
	static unsigned char lastStep = 0xFF;
	unsigned int t = ctmr;
	unsigned char step;
	if(t > COMMA_PERIOD_MS-5){ ctmr = 0; t = 0; }
	step = t/COMMA_STEP_MS;//0..COMMA_STEPS-1
	//В первые 20 мс цикла положение подтверждается на каждом проходе (эффект перебора
	//цифр может погасить запятую, см. cathRue)
	if((step != lastStep) || (t < 20))
	{
		onIndComma(commaPos[step]);
		offIndComma(commaPos[(step+COMMA_STEPS-1)%COMMA_STEPS]);
		lastStep = step;
	}
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

	handleTmDtFlag();
	if(indBl)handleIndBlink(indBl-1);
	if(ledEffType)flgLedEff = 1;//ledEff(ledEffType);
	if(flgFrEff == FIRE_EFF1) handleIndFire();
	if(flgFrEff == FIRE_EFF2) handleIndFireAll();

}

//Выводит на лампы свежепрочитанное время. Вызывается из главного цикла сразу после чтения DS3231.
//Раньше это делало прерывание RTC со значениями, прочитанными на предыдущем тике, поэтому
//индикация отставала от DS3231 на лишнюю секунду.
//Запуск перебора цифр привязан к показываемому времени: он начинается, когда на лампах
//появляется :00 (или :59 в режиме "раз в час").
static void showTimeOnInds(void)
{
	unsigned char i;
	if(flagConfMode) return;//В режиме настройки лампами управляет меню
	if((flgRueOn == 5)&&(tmdt.sec == 59))flgR = 1;
	else if(flgRueOn&&(tmdt.sec == 0)&&!(tmdt.min%rueInterv[flgRueOn]))flgR = 1;
	for(i = 0; i < GDI_COUNT; i++) gdiEffSet(i, gdiVal[i]);
}

//Чтение времени завершено (или прервано по таймауту). Если за время чтения пришло новое
//прерывание RTC (secFlag == TM_WAIT), его нельзя затирать - иначе будет пропущена секунда.
static void secTickDone(void)
{
	IRQ_SAVE();
	if(secFlag == TM_WORK) secFlag = TM_READY;
	IRQ_RESTORE();
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
	oldEncoState = getEncoderPinsValue();//Энкодер может стоять в положении 00
	initConfig();//Заводские настройки (в т.ч. подсветка: цвет 362, яркость 0)
	settingsLoad();//Сохраненные во flash настройки поверх заводских (если есть)
	initButtons(&bt0,&bt1,&bt2);
	I2C1->CR1|=I2C_CR1_PE;
//	startLightSensDMA();
	//ds3231StartOsc();
	//	i2c1Write(RTC_ADR,0x0e,&value,1);
	NVIC_EnableIRQ(EXTI2_IRQn);
		NVIC_EnableIRQ(EXTI1_IRQn);
	initIWDG();
	while(1)
	{
		static unsigned int systm = 0;
		unsigned char tick = 0;
		feedIWDG();
		wsTransferPoll();//Отложенный кадр WS2812 (см. startWsTransfer)
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
			secTickDone();
			changeTmDt();
			showTimeOnInds();
		}
		//Фоновое чтение DS3231 не завершилось (NACK/помеха/зависшая шина):
		//I2C перезапущен, разрешаем новое чтение по следующему прерыванию RTC
		if(i2cCheckTimeout()) secTickDone();
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
#ifdef CONF_BUTT_01
    handleUIButt(&bt0, &bt1);
#elif defined(CONF_BUTT_02)
    handleUIButt(&bt0, &bt2);
#elif defined(CONF_BUTT_10)
    handleUIButt(&bt1, &bt0);
#elif defined(CONF_BUTT_12)
    handleUIButt(&bt1, &bt2);
#elif defined(CONF_BUTT_20)
    handleUIButt(&bt2, &bt0);
#elif defined(CONF_BUTT_21)
    handleUIButt(&bt2, &bt1);
#else
	handleUI(be);//Энкодер с кнопкой
#endif
		//Автояркость
		if(flagAutoYarkost){autoBright();}
		//Выход из режима настройки по таймауту
	  if(confTm>CONF_LIFETIME)//ENCODER CONF
		{
			confTm = 0;flagTmDtConf = CONF_NO_VALUE;
				flgSetTm = 0;//Неподтвержденные секунды отбрасываем: к моменту таймаута они устарели
			flagConfMode = CONF_MODE_OFF;indBl = 0;
			flgFrEff = prevConfVals[CONF_GD_FE];
		}
		//Обработка по системному тику
		if(tick)handleTickEvs();
		tick = 0;

		handleRue(flgR);
		if(flgC&&(!flagConfMode))handleComma();
		settingsPoll();//Сохранение изменившихся настроек во flash
	}
}
