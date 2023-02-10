#ifndef SYSTEM_H
#define SYSTEM_H

#include "stm32f1xx.h"
#include "stm32f103x6.h"
#include "devconfdef.h"

#define I2C_IPTDMA

#define QUARZ_FREQ 8000000
#define MUL_CLOCK_COEF 5
#define SYS_CORE_CLOCK QUARZ_FREQ*MUL_CLOCK_COEF
#define CORE_CLOCK_MHZ SYS_CORE_CLOCK/1000000
#define GDI_FREQ 70
#define SYSTMR_FREQ 1000
#define SYS_TIMER_FREQ SYS_CORE_CLOCK/SYSTMR_FREQ-1			
#define GDI_DISCRETE 66				
#define GDI_COUNT 6						  	
#define GDI_TIM_ARR 20
#define GDI_TIM_PSC (SYS_CORE_CLOCK/(GDI_COUNT*L_MAX_YARK))/GDI_FREQ/GDI_TIM_ARR	

#define LED_HW_LENGTH 6*24+WS_RES_VAL


//#define RG_ENCODER_VAL encoderCounter
//#define encoderCounter (9*2+1)
//#define RG_ENCODER_VAL TIM4->CNT
//#define RG_ENCODER_MAX_VAL TIM4->ARR

#define RG_BUTT_INPREG GPIOB->IDR
#define BUTT_NEXT_PB0 0x00000001
#define BUTT_ENTER_PB1 0x00000002
#define BUTT_PREV_PB2 0x00000004




#define DS3231_ADR 0xD0
#define DS3231_WRITE 0x00
#define DS3231_READ 0x01

#define SENS_BUFF_SZ 1
#define LIGHT_ADC_MIN 20
#define LIGHT_ADC_MAX 1700
#define LIGHT_COEF (LIGHT_ADC_MAX-LIGHT_ADC_MIN)/60
#define I2C_RCV_SIZE 10

#define I2C_NEED_RCV 0x02
#define I2C_NEED_TRM 0x01
#define I2C_FREE 0x00
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
void initTimer4(void);
void initTimer3(void);
void init_DMA(void);
void startTimer1(void);
void initTimer1(void);
void startTimer3(void);
void stopTimer3(void);
void initGPIO(void);
void initExtIntGPIO(void);
void initAnodesGPIO(void);
void initCathsGPIO(void);
//void initEncoderGPIO(void);//ENCODER CONF
void initWsGPIO(void);
void initADC(void);
void initI2C1(void);
void initI2C1i(void);
void initI2cParams(void);
void enableI2C1(void);
void disableI2C1(void);
void i2c1Write(char devAdress, char memAdress,unsigned char* data, char length);
void enableInterrupts(void);
void initRTC(void);
void initWs2812ProtPin(void);
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
void i2cGetTmDt(void);
void uSendData(char* data, int size);
struct I2CDev 
{
	unsigned char devAM;//Adress&Mode for i2c dev
	unsigned char devIter;
  unsigned char devData[10];
  unsigned char devMA;//adress mem in dev
  unsigned char devRcvCnt;
	unsigned char trmRdy;
	unsigned char rcvRdy;
	unsigned char flg;
	unsigned char devIterRev;
};


 unsigned short int getEncVal();
 void setEncVal(unsigned short int val);
 void setMaxEncVal(unsigned short int mval);


void setCath(register unsigned short int cath);
 void setAnode(register unsigned short int cath);


//new version in 1_0_2
/*__inline void i2cRd(char devAdr, char memAdr, register unsigned char* buff, register char count)
{
	
	int tmp;
	char crd = count;
	if(!buff)return;
	
	I2C1->CR1|=I2C_CR1_START; while(!(I2C1->SR1&I2C_SR1_SB)); tmp = I2C1->SR1;//start
	I2C1->DR = (devAdr|DS3231_WRITE); while(!(I2C1->SR1 & I2C_SR1_ADDR)); tmp = I2C1->SR1; tmp = I2C1->SR2;//send devadr
	I2C1->DR = memAdr;  //send memadr

	I2C1->CR1|= I2C_CR1_START; while(!(I2C1->SR1&I2C_SR1_SB));tmp = I2C1->SR1;//povstart	
	I2C1->DR = (devAdr|DS3231_READ); while(!(I2C1->SR1&I2C_SR1_ADDR));tmp = I2C1->SR1; tmp = I2C1->SR2;//send devadr
	
	if(crd>3){ while(!(I2C1->SR1 & I2C_SR1_BTF)); *buff = I2C1->DR; buff++; crd--; }
	
	if(crd == 3)
	{
		while(!(I2C1->SR1 & I2C_SR1_BTF));
		I2C1->CR1&= ~I2C_CR1_ACK; 
		__disable_irq();
		*buff = I2C1->DR; buff++;
		I2C1->CR1|= I2C_CR1_STOP;
		*buff = I2C1->DR;
		__enable_irq();
		buff++;
		while((I2C1->SR1 & I2C_SR1_RXNE)!= I2C_SR1_RXNE);
		*buff = I2C1->DR;
		while((I2C1->CR1 & I2C_CR1_STOP) == I2C_CR1_STOP);
		I2C1->CR1|=I2C_CR1_ACK;
		crd = 0;
		
	}
	
}
*/


//old version in v1_0_1
__inline void i2c1Read(char devAdress, char memAdress,register unsigned char* buffer, register char count)
{
	register int i = 0;
	if(!buffer)
		return;
	//__disable_irq();
	//set START signal
	I2C1->CR1&= ~I2C_CR1_START;
	I2C1->CR1|= I2C_CR1_START;
	while(!(I2C1->SR1&I2C_SR1_SB));//wait while START ok
	(void)I2C1->SR1;
	I2C1->DR = (devAdress|DS3231_WRITE);//send dev adress& write flag
	while(!(I2C1->SR1 & I2C_SR1_ADDR));//wait dev while adress sended
	(void)I2C1->SR1;
	(void)I2C1->SR2;
	I2C1->DR = memAdress;//set adress for write
	while(!(I2C1->SR1 & I2C_SR1_BTF));//wait while adress sended
	I2C1->CR1|= I2C_CR1_START;//set POVSTART signal
	while(!(I2C1->SR1&I2C_SR1_SB));//wait while START ok
	(void)I2C1->SR1;
	I2C1->DR = (devAdress|DS3231_READ);//send dev adress& read flag
	while(!(I2C1->SR1 & I2C_SR1_ADDR));//wait dev while adress sended
	(void)I2C1->SR1;
	(void)I2C1->SR2;
	while(i<count)	//get data
	{
	
		I2C1->CR1|=I2C_CR1_ACK;
		while(!(I2C1->SR1 & I2C_SR1_RXNE));
		buffer[i] = I2C1->DR;
		i++;		
	}
	I2C1->CR1 &= ~I2C_CR1_ACK;
	I2C1->CR1 |= I2C_CR1_STOP;	//stop
	
}


 

/*
__inline void i2cRead(char devAdress, char memAdress,register unsigned char* buffer, register char count)
{
		register int i = 0;
	int tmp;
	int crd = count;
	if(!buffer)
		return;

	//set START signal
	I2C1->CR1&= ~I2C_CR1_START;
	I2C1->CR1|= I2C_CR1_START;
	while(!(I2C1->SR1&I2C_SR1_SB));//wait while START ok
	//(void)I2C1->SR1; //??
	I2C1->DR = (devAdress|DS3231_WRITE);//send dev adress& write flag
	while(!(I2C1->SR1 & I2C_SR1_ADDR));//wait dev while adress sended
	//(void)I2C1->SR1;//??
	tmp = I2C1->SR2;
	
	while(crd)
	{
		if(crd!=3)
		{
			
			while((I2C1->SR1 & I2C_SR1_BTF)!= I2C_SR1_BTF);
		//	I2C1->CR1|=I2C_CR1_ACK;
			*buffer = I2C1->DR;
			buffer++;
			crd--;
		}
		if(crd == 3)
		{
			while((I2C1->SR1 & I2C_SR1_BTF)!= I2C_SR1_BTF);
			I2C1->CR1 &= ~I2C_CR1_ACK;
			__disable_irq();
			*buffer = I2C1->DR; buffer++; I2C1->CR1 |= I2C_CR1_STOP;
			*buffer = I2C1->DR; 
			__enable_irq();
			buffer++;
			while((I2C1->SR1 & I2C_SR1_RXNE) != I2C_SR1_RXNE);
			*buffer = I2C1->DR;
			crd = 0;
		}
	}

}
*/

#endif //SYSTEM_H

