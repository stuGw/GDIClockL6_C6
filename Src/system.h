#ifndef SYSTEM_H
#define SYSTEM_H

#include "stm32f1xx.h"
#include "stm32f103x6.h"
#include "devconfdef.h"

//Критическая секция: сохраняет и восстанавливает PRIMASK, поэтому безопасна и внутри обработчиков
//прерываний. Применяется там, где главный цикл меняет данные, которые читают/пишут прерывания.
#define IRQ_SAVE()    uint32_t irqPrimask_ = __get_PRIMASK(); __disable_irq()
#define IRQ_RESTORE() __set_PRIMASK(irqPrimask_)

#define I2C_IPTDMA

//QUARZ_FREQ задается в devconfdef.h
#define MUL_CLOCK_COEF 5
#define SYS_CORE_CLOCK QUARZ_FREQ*MUL_CLOCK_COEF
//Под 40 МГц жестко рассчитаны: PLLMULL5 (initSystemClock), TIM3 (WS2812: ARR = 49, WS_H0/WS_H1),
//I2C (FREQ = 20 МГц на APB1, CCR, TRISE), TIM1/TIM2. Другая частота потребует пересчета всего этого.
#if (QUARZ_FREQ != 8000000) || (MUL_CLOCK_COEF != 5)
#error "system.h: поддерживается только кварц 8 МГц с PLL x5 (ядро 40 МГц), см. комментарий выше"
#endif
#define CORE_CLOCK_MHZ SYS_CORE_CLOCK/1000000
#define GDI_FREQ 70
#define SYSTMR_FREQ 1000
#define SYS_TIMER_FREQ SYS_CORE_CLOCK/SYSTMR_FREQ-1			
#define GDI_DISCRETE 66				
//GDI_COUNT (число ламп) задается модулем в gdi.h; здесь используется только в GDI_TIM_PSC, который раскрывается в system.c после gdi.h
#define GDI_TIM_ARR 20
#define GDI_TIM_PSC (SYS_CORE_CLOCK/(GDI_COUNT*L_MAX_YARK))/GDI_FREQ/GDI_TIM_ARR	




#define RG_BUTT_INPREG GPIOB->IDR
#define BUTT_NEXT_PB0 0x00000001
#define BUTT_ENTER_PB1 0x00000002
#define BUTT_PREV_PB2 0x00000004




#define DS3231_ADR 0xD0
#define DS3231_WRITE 0x00
#define DS3231_READ 0x01

#define SENS_BUFF_SZ 8
//Сглаживание датчика освещенности: фильтр первого порядка, один шаг в миллисекунду.
//Постоянная времени ~LIGHT_FILT_DIV мс; значение хранится с запасом разрядов (LIGHT_FILT_SHIFT)
#define LIGHT_FILT_DIV 128
#define LIGHT_FILT_SHIFT 6
#define LIGHT_ADC_MIN 20
#define LIGHT_ADC_MAX 1700
#define LIGHT_COEF (LIGHT_ADC_MAX-LIGHT_ADC_MIN)/60
#define I2C_RCV_SIZE 10

#define I2C_NEED_RCV 0x02
#define I2C_NEED_TRM 0x01
#define I2C_FREE 0x00

//Защита I2C от зависаний
#define I2C_TIMEOUT_MS 20		//Макс. время фонового чтения DS3231 (обычно < 1 мс)
#define I2C_WAIT_LOOPS 20000UL	//Ограничение циклов ожидания флага в i2c1Write (~5 мс)
#define WS_TRANSFER_TIMEOUT_MS 10	//Кадр WS2812 занимает ~0.3 мс; дольше - считаем, что прерывание DMA потеряно
#define I2C_RECOVERY_DELAY 30	//Полупериод SCL при освобождении шины (~5 мкс)
void initEncoderGPIOEXTI();
uint8_t getEncoderPinsValue();
void sysInit(void);
void initLightSensBuff(void);
void initLightSensDMA(void);
void startLightSensDMA(void);
void stopLightSensDMA(void);
unsigned short int getLightSensValue(void);
void initSystemClock(void);
void ledDbgOn(void);
void ledDbgOff(void);
void ledDbgTuggle(void);
void initTimer2(void); 
void startTimer2(void);
void initTimer3(void);
void startTimer1(void);
void initTimer1(void);
void startTimer3(void);
void stopTimer3(void);
void initGPIO(void);
void initExtIntGPIO(void);
void initAnodesGPIO(void);
void initCathsGPIO(void);
void initWsGPIO(void);
void initADC(void);
void initI2C1i(void);
void initI2cParams(void);
void enableI2C1(void);
unsigned char i2c1Write(char devAdress, char memAdress,unsigned char* data, char length);
void i2c1Recover(void);
unsigned char i2cCheckTimeout(void);
void initIWDG(void);
void feedIWDG(void);
void wsTransferComplete(void);
void wsTransferPoll(void);
void enableInterrupts(void);
void initRTC(void);
void startWSDMA(void);
void startWSDMACircle(void);
void initWSDMA(void);
void stopWSDMA(void);
void wsDmaTmCirc(void);
void wsDmaTmOnce(void);
void startWsTransfer(void);
void delay(int val);
void initTimeDMA(void);
void startTimeDma(void);
void sendI2C(void);
struct I2CDev 
{
	unsigned char devAM;//Adress&Mode for i2c dev
	unsigned char devIter;
  unsigned char devData[10];
  unsigned char devMA;//adress mem in dev
  unsigned char devRcvCnt;
	unsigned char trmRdy;
	volatile unsigned char rcvRdy;//Меняется в прерывании DMA
	volatile unsigned char flg;
	unsigned char devIterRev;
};


 unsigned short int getEncVal();
 void setEncVal(unsigned short int val);
 void setMaxEncVal(unsigned short int mval);


void setCath(register unsigned short int cath);
 void setAnode(register unsigned short int cath);


#endif //SYSTEM_H

