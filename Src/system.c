#include "system.h"
#include "ws2812.h"
#include "time.h"
#include "gdi.h"
#include "ds3231.h"

#include "stm32f1xx.h"
#include "stm32f103x6.h"

#define ADC_CONV_COUNT 100

unsigned short int encoderMaxValue = 9*ENC_DEV_COEF+1;
unsigned short int encoderCounter = 0;

struct I2CDev i2c;
extern unsigned char timeNDateBuffer[];
extern volatile struct Button bt0,bt2,bt1;//bt0 bt2 - buttons, bt1 - encoder button
extern volatile struct Button *be;
unsigned char flag = 0;
extern struct LedHwPack hwBuff;
unsigned short int lightSensBuff[SENS_BUFF_SZ];
volatile unsigned char flgWSTransfer = 0;
unsigned char bf[7];
//uint32_t bfa = (uint32_t)bf;
int adcRes = 0;
extern uint32_t hwLedAdress;
void delay(int val)
{
	volatile int i = 0;
	while(i<val)i++;
}
void sysInit()
{
	//sys
	#ifdef ENC_BT0
	be = &bt0;
	#endif
	#ifdef ENC_BT1
	be = &bt1;
	#endif
	#ifdef ENC_BT2
	be = &bt2;
	#endif
	initSystemClock();
	initGPIO();
	initRTC();
	initTimer1();
	startTimer1();
	initTimer2();
	SysTick_Config(SYS_TIMER_FREQ);
	ledDbgOff();
	initI2C1i();
	enableI2C1();			

	//for encoder (mcu c6 - interrupts mode, c8 - timer mode)
	//initTimer4();
	initEncoderGPIOEXTI();

	startTimer2();
	initInds();
	initWSDMA();
	initTimer3();
		//initUart();
	enableInterrupts();
		
	initLightSensBuff();
	initLightSensDMA();
	initADC();
	
	//use for c8 mcu
	//TIM4->ARR = 9*2+1;
	//TIM4->CNT = 0;

}



void enableInterrupts()
{
	/*NVIC_SetPriority(TIM2_IRQn,3);
	NVIC_SetPriority(TIM1_UP_IRQn,3);
	NVIC_SetPriority(EXTI0_IRQn,3);
	NVIC_SetPriority(EXTI1_IRQn,3);
	NVIC_SetPriority(EXTI2_IRQn,3);
	NVIC_SetPriority(RTC_IRQn,3);
	NVIC_SetPriority(DMA1_Channel6_IRQn,3);
	NVIC_SetPriority(DMA1_Channel7_IRQn,3);
	NVIC_SetPriority(TIM3_IRQn,3);
	NVIC_SetPriority(I2C1_EV_IRQn,2);*/
//	NVIC_SetPriority(I2C1_EV_IRQn,1);
	//NVIC_SetPriority(RTC_IRQn,0);
//	NVIC_SetPriorityGrouping(4);
	NVIC_EnableIRQ(TIM2_IRQn);  // Enable IRQ
	NVIC_EnableIRQ(TIM1_UP_IRQn);
	
//#ifdef ENC_BT0
	NVIC_EnableIRQ(EXTI0_IRQn);
//	#endif
//#ifdef ENC_BT1
	NVIC_EnableIRQ(EXTI1_IRQn);
//#endif
//#ifdef ENC_BT2
	NVIC_EnableIRQ(EXTI2_IRQn);
//#endif
	


	NVIC_EnableIRQ(RTC_IRQn);
	NVIC_EnableIRQ(DMA1_Channel6_IRQn);
	NVIC_EnableIRQ(DMA1_Channel7_IRQn);
	NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_EnableIRQ(I2C1_EV_IRQn);
//NVIC_EnableIRQ(USART1_IRQn);

	NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void initSystemClock()
{
	
	//HSE on
	RCC->CR|=RCC_CR_HSEON; 
	//Wait HSE
	while (!(RCC->CR & RCC_CR_HSERDY)) {}; 
    //Set HSE As system clock
	RCC->CFGR &=~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_HSE; 
		
	RCC->CFGR |=RCC_CFGR_PPRE1_2;	
		
	//Enable clock security system
	RCC->CR|=RCC_CR_CSSON; 
	//PLL
	RCC->CFGR &=~((RCC_CFGR_PLLSRC|RCC_CFGR_PLLXTPRE|RCC_CFGR_PLLMULL)); // predochistka
	RCC->CFGR|=RCC_CFGR_PLLSRC;//RCC_CFGR_PLLSRC_HSE; //vibyraem istochnik HSE
	RCC->CFGR|=RCC_CFGR_PLLMULL5; // koefficient umnojenia - 2
	RCC->CR |= RCC_CR_PLLON; // vklychaem PLL
	while ((RCC->CR & RCC_CR_PLLRDY)==0) {} //jdem gotovnosti  PLL.
	RCC->CFGR &=~RCC_CFGR_SW; // vklychaem HSI pered zadaniem PLL.
	RCC->CFGR |= RCC_CFGR_SW_PLL; //vklychaem PLL kak osnovnoy istochnikPLL.
	while ((RCC->CFGR&RCC_CFGR_SWS)!=0x08) {} // jdem gotovnosti PLL.
}

void initGPIO()
{
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;  // Enable PORTA Periph clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;  // Enable PORTC Periph clock
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;  // Enable PORTB Periph clock
	
	//disable jtag(free pb4 pb3 pa15 pins)
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1;
	initExtIntGPIO();
	initCathsGPIO();
	initAnodesGPIO();
	initWsGPIO();
	
	
	//for c8 mcu
	//initEncoderGPIO();//ENCODER CONF

	/*OTHER PINS*/
	//just led
	GPIOA->CRL &= ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF);  
	GPIOA->CRL |= GPIO_CRL_MODE0_0;
	
	GPIOC->CRH &= ~(GPIO_CRH_MODE15 | GPIO_CRH_CNF15);
	GPIOC->CRH |= GPIO_CRH_CNF15_0;
}

void initExtIntGPIO()
{
	//exti pins
//#ifdef ENC_BT0
	GPIOB->CRL &= ~GPIO_CRL_MODE0_0;
	GPIOB->CRL &= ~GPIO_CRL_MODE0_1;
	GPIOB->CRL &= ~GPIO_CRL_CNF0;
	GPIOB->CRL |= GPIO_CRL_CNF0_1;
	GPIOB->ODR |= GPIO_ODR_ODR0;
	AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PB;
	EXTI->IMR|=EXTI_IMR_MR0;
	EXTI->FTSR|=EXTI_FTSR_TR0;
//	#endif
//#ifdef ENC_BT1
	GPIOB->CRL &= ~GPIO_CRL_MODE1_0;
	GPIOB->CRL &= ~GPIO_CRL_MODE1_1;
	GPIOB->CRL &= ~GPIO_CRL_CNF1;
	GPIOB->CRL |= GPIO_CRL_CNF1_1;
	GPIOB->ODR |= GPIO_ODR_ODR1;
	AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI1_PB;
	EXTI->IMR|= EXTI_IMR_MR1;
	EXTI->FTSR|=EXTI_FTSR_TR1;
//	#endif
	
//#ifdef ENC_BT2
	GPIOB->CRL &= ~GPIO_CRL_MODE2_0;
	GPIOB->CRL &= ~GPIO_CRL_MODE2_1;
	GPIOB->CRL &= ~GPIO_CRL_CNF2;
	GPIOB->CRL |= GPIO_CRL_CNF2_1;
	GPIOB->ODR |= GPIO_ODR_ODR2;
	AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI2_PB;
	EXTI->IMR|= EXTI_IMR_MR2;
	EXTI->FTSR|=EXTI_FTSR_TR2;
//#endif
}

void initAnodesGPIO()
{
//B10 B11 B12 B13 B14 B15
	GPIOB->CRH |= GPIO_CRH_MODE10_0 | GPIO_CRH_MODE11_0 | GPIO_CRH_MODE12_0 | GPIO_CRH_MODE13_0 | GPIO_CRH_MODE14_0 | GPIO_CRH_MODE15_0;
	GPIOB->CRH |= GPIO_CRH_MODE10_1 | GPIO_CRH_MODE11_1 | GPIO_CRH_MODE12_1 | GPIO_CRH_MODE13_1 | GPIO_CRH_MODE14_1 | GPIO_CRH_MODE15_1;
	GPIOB->CRH &=~GPIO_CRH_CNF10;
	GPIOB->CRH &=~GPIO_CRH_CNF11;
	GPIOB->CRH &=~GPIO_CRH_CNF12;
	GPIOB->CRH &=~GPIO_CRH_CNF13;
	GPIOB->CRH &=~GPIO_CRH_CNF14;
	GPIOB->CRH &=~GPIO_CRH_CNF15;
}
void initCathsGPIO()
{
//A2 A3 A4 A5 A6 A7 A8 A9 A10 A11 A12
	GPIOA->CRL |= GPIO_CRL_MODE2_0 | GPIO_CRL_MODE2_1 | GPIO_CRL_MODE3_0 | GPIO_CRL_MODE3_1 | GPIO_CRL_MODE4_0 | GPIO_CRL_MODE4_1 | 
	GPIO_CRL_MODE5_0 | GPIO_CRL_MODE5_1 | GPIO_CRL_MODE6_0 | GPIO_CRL_MODE6_1 | GPIO_CRL_MODE7_0 | GPIO_CRL_MODE7_1;
	GPIOA->CRH |= GPIO_CRH_MODE8_0 | GPIO_CRH_MODE8_1 | GPIO_CRH_MODE9_0 | GPIO_CRH_MODE9_1 | GPIO_CRH_MODE10_0 | GPIO_CRH_MODE10_1 |
	GPIO_CRH_MODE11_0 | GPIO_CRH_MODE11_1 | GPIO_CRH_MODE12_0 | GPIO_CRH_MODE12_1 | GPIO_CRH_MODE15_0 | GPIO_CRH_MODE15_1;
	GPIOA->CRL &=~GPIO_CRL_CNF2;
	GPIOA->CRL &=~GPIO_CRL_CNF3;
  GPIOA->CRL &=~GPIO_CRL_CNF4;
	GPIOA->CRL &=~GPIO_CRL_CNF5;
	GPIOA->CRL &=~GPIO_CRL_CNF6;
	GPIOA->CRL &=~GPIO_CRL_CNF7;
	GPIOA->CRH &=~GPIO_CRH_CNF8;
	GPIOA->CRH &=~GPIO_CRH_CNF9;
	GPIOA->CRH &=~GPIO_CRH_CNF10;
	GPIOA->CRH &=~GPIO_CRH_CNF11;
	GPIOA->CRH &=~GPIO_CRH_CNF12;
	GPIOA->CRH &=~GPIO_CRH_CNF15;
}


//effs timer
void initTimer1()
{
	 RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;  // Enable TIM2 Periph clock
   TIM1->PSC = 1000; // 1000 tick/sec//last 1000000
   TIM1->ARR = CORE_CLOCK_MHZ;  // 1 Interrupt/1 sec
   TIM1->DIER |= TIM_DIER_UIE; // Enable tim2 interrupt
}

void startTimer1()
{
	 TIM1->CR1 |= TIM_CR1_CEN;   // Start count
}

//Encoder with timer
void initEncoderGPIO()
{
	GPIOB->CRL &= ~(GPIO_CRL_CNF6_0 | GPIO_CRL_CNF7_0);
	GPIOB->CRL |= GPIO_CRL_CNF7_1 | GPIO_CRL_CNF6_1;
	GPIOB->CRL &= ~(GPIO_CRL_MODE6_0|GPIO_CRL_MODE6_1|GPIO_CRL_MODE7_0|GPIO_CRL_MODE7_1);
	GPIOB->ODR |= GPIO_ODR_ODR6 | GPIO_ODR_ODR7;
}
void initTimer4()
{
	/*RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	initEncoderGPIO();
	TIM4->CCMR1 |= TIM_CCMR1_CC1S_0; //capture compare on
	TIM4->SMCR|= TIM_SMCR_SMS_0;//счет энкодера только по одному каналу
	TIM4->CCER &= ~(TIM_CCER_CC1P);//?
	TIM4->ARR = 120;
	TIM4->CR1 |= TIM_CR1_CEN;//|TIM_CR1_ARPE;
	TIM4->CNT = 0;
	*/
}

//encoder with exti, see exti9_5 handler too
uint8_t getEncoderPinsValue()
{
	return ((GPIOB->IDR&0x40) | (GPIOB->IDR&0x80))>>6;
}

void initEncoderGPIOEXTI()
{
	GPIOB->CRL &= ~GPIO_CRL_MODE6_0;
	GPIOB->CRL &= ~GPIO_CRL_MODE6_1;
	GPIOB->CRL &= ~GPIO_CRL_CNF6;
	GPIOB->CRL |= GPIO_CRL_CNF6_1;
	GPIOB->ODR |= GPIO_ODR_ODR6;
	AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI6_PB;
	EXTI->IMR|=EXTI_IMR_MR6;
	EXTI->FTSR|=EXTI_FTSR_TR6;
	EXTI->RTSR|=EXTI_RTSR_TR6;

	GPIOB->CRL &= ~GPIO_CRL_MODE7_0;
	GPIOB->CRL &= ~GPIO_CRL_MODE7_1;
	GPIOB->CRL &= ~GPIO_CRL_CNF7;
	GPIOB->CRL |= GPIO_CRL_CNF7_1;
	GPIOB->ODR |= GPIO_ODR_ODR7;
	AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI7_PB;
	EXTI->IMR|=EXTI_IMR_MR7;
	EXTI->FTSR|=EXTI_FTSR_TR7;
	EXTI->RTSR|=EXTI_RTSR_TR7;


}



void initTimer2()
{	
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;  // Enable TIM2 Periph clock
    TIM2->PSC = GDI_TIM_PSC;//lst100//SYS_CORE_CLOCK/1000000; // 1000 tick/sec//last 1000000
    TIM2->ARR = GDI_TIM_ARR;//lst20;  // 1 Interrupt/1 sec
    TIM2->DIER |= TIM_DIER_UIE; // Enable tim2 interrupt
} 

void startTimer2()
{
	  TIM2->CR1 |= TIM_CR1_CEN;   // Start count
}
//wstimer
void initTimer3()
{
	//init pin
	GPIOB->CRL &=~GPIO_CRL_CNF4;
	GPIOB->CRL |= GPIO_CRL_CNF4_1;
	GPIOB->CRL |= GPIO_CRL_CNF4_0;
	GPIOB->CRL |= GPIO_CRL_MODE4_1;
	GPIOB->CRL |= GPIO_CRL_MODE4_0;
	GPIOB->BSRR &= ~GPIO_BSRR_BS4;
	GPIOB->BSRR |= GPIO_BSRR_BR4;
	
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;  // Enable TIM2 Periph clock

	//remap ch1 to pb4
	AFIO->MAPR &= ~AFIO_MAPR_TIM3_REMAP;
	AFIO->MAPR |= AFIO_MAPR_TIM3_REMAP_1;

	TIM3->ARR = 49;//old 59 for 48MHz
	TIM3->CCER |= TIM_CCER_CC1E;
	TIM3->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2|TIM_CCMR1_OC1PE ;
	TIM3->CCR1 = 0x0013;
	TIM3->DIER   |= TIM_DIER_CC1DE | TIM_DIER_UIE; 

}



void startTimer3()
{
	TIM3->CR1 |= TIM_CR1_CEN| TIM_CR1_ARPE;
	
}
void stopTimer3()
{
	TIM3->CR1&= ~TIM_CR1_CEN;
}
//Одниночная передача на все диоды
void startWSDMA(void)
{
	DMA1_Channel6->CNDTR = WS_COUNT_BITS;
	DMA1_Channel6->CCR = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_EN | DMA_CCR_PSIZE_0|DMA_CCR_TCIE|DMA_CCR_PL_0|DMA_CCR_PL_1;
}
//Постоянная передача на все диоды
void startWSDMACircle()
{
	DMA1_Channel6->CNDTR = WS_COUNT_BITS;
	DMA1_Channel6->CCR = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_EN | DMA_CCR_PSIZE_0|DMA_CCR_CIRC|DMA_CCR_PL_0|DMA_CCR_PL_1;
}

void initWSDMA(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	
	//ws2812DMA
	DMA1_Channel6->CPAR = (uint32_t) &TIM3->CCR1;
	DMA1_Channel6->CMAR = hwLedAdress;
	DMA1_Channel6->CNDTR = WS_COUNT_BITS;
	DMA1_Channel6->CCR = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_PSIZE_0|DMA_CCR_TCIE;//| DMA_CCR6_EN ;
		DMA1_Channel6->CCR|= DMA_CCR_PL_1;//priority high(very high used for ws2812)
	DMA1_Channel6->CCR|=DMA_CCR_PL_0;
}
//Работа с датчиком освещения
//Инициализация
//initLightSensBuff();
//initADC()
//initLightSensDMA()
//Value = getLightSensValue() - для получения значения
//АЦП складывает свои значения через ПДП 
//в кольцевой буфер lightSensBuff
//В любой момент времени можно получить 
//усредненное значение с датчика вызвав 
//функцию getLightSensValue();
void initLightSensBuff(void){	memset(lightSensBuff,0,sizeof(unsigned short int));}

void initTimeDMA(void)
{
	DMA1_Channel7->CCR = 0;
	DMA1_Channel7->CPAR = (uint32_t)&I2C1->DR;
	DMA1_Channel7->CMAR =  (uint32_t)i2c.devData;
	DMA1_Channel7->CCR&= ~DMA_CCR_MEM2MEM;//meme2mem disabled
	DMA1_Channel7->CCR|= DMA_CCR_PL_1;//priority high(very high used for ws2812)
	DMA1_Channel7->CCR&=~DMA_CCR_PL_0;
	DMA1_Channel7->CCR&=~DMA_CCR_MSIZE;//8bit
	DMA1_Channel7->CCR|=DMA_CCR_PSIZE_0;//8bit
	DMA1_Channel7->CCR|=DMA_CCR_MINC;//mem inc enable
	DMA1_Channel7->CCR&=~DMA_CCR_PINC;// periph inc disable
	DMA1_Channel7->CCR&=~DMA_CCR_CIRC;//once mode
	DMA1_Channel7->CCR&=~DMA_CCR_DIR;//read from periph
	DMA1_Channel7->CCR|=DMA_CCR_TCIE;//interrupt enable
	
	DMA1_Channel7->CNDTR = TIME_BUFF_SZ;//7byte
}
void startTimeDma(void)
{
	
	DMA1_Channel7->CCR |= DMA_CCR_EN;
}


void initLightSensDMA()
{
	DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
	DMA1_Channel1->CMAR =  (uint32_t)&lightSensBuff;
	DMA1_Channel1->CCR&=~DMA_CCR_DIR;
	DMA1_Channel1->CNDTR = SENS_BUFF_SZ;
	DMA1_Channel1->CCR &= ~DMA_CCR_PINC;
	DMA1_Channel1->CCR |= DMA_CCR_MINC;
	DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0;
	DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0;
	DMA1_Channel1->CCR |= DMA_CCR_CIRC;
	DMA1_Channel1->CCR |= DMA_CCR_EN;
}

void startLightSensDMA(void)
{
	DMA1_Channel1->CCR |= DMA_CCR_MINC;
	DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0;
	DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0;
	DMA1_Channel1->CCR |= DMA_CCR_CIRC;
	DMA1_Channel1->CCR |= DMA_CCR_EN;
}

void stopLightSensDMA(void)
{
	DMA1_Channel1->CCR&= ~DMA_CCR_EN;
}
//Режим непрерывной передачи для ws
void wsDmaTmCirc(void)
{
	startWSDMACircle();
	TIM3->DIER&= ~TIM_DIER_UIE; 
	startTimer3();
}
//Режим одиночной передачи для ws
void wsDmaTmOnce(void)
{
	DMA1_Channel6->CCR&= ~DMA_CCR_CIRC;
	DMA1_Channel6->CCR|=DMA_CCR_TCIE;
	TIM3->DIER|=TIM_DIER_UIE;
}
void initADC()
{
	//ADC Pin

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;	//Enable clock for port A
	GPIOA->CRL &= ~GPIO_CRL_CNF1;	//Config GPIO
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV8;	//frequency for adc divided by 8
	//
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN ;    //enable clock adc
	ADC1->CR2 |= ADC_CR2_CAL;//Adc calibration on 
  while (!(ADC1->CR2 & ADC_CR2_CAL));//Wait while calibration complete
	
	//NVIC->ISER[0] |= NVIC_ISER_SETENA_18;
	NVIC_EnableIRQ(ADC1_2_IRQn);
	ADC1->SMPR2 |= ADC_SMPR2_SMP1_0; 
	ADC1->CR2 |=ADC_CR2_DMA; //enable work with dma
	ADC1->CR2|= ADC_CR2_ADON;//adc on
	ADC1->SQR1 = 0x00000000;//count sequence
	ADC1->SQR2 = 0x00000000;
	ADC1->SQR3 = 0x00000001;
	ADC1->CR2 |= ADC_CR2_CONT;
	ADC1->CR2 |= ADC_CR2_ADON;
	ADC1->CR2 |= ADC_CR2_ADON;
}

unsigned short int getLightSensValue(void)
{
	int i = 0,res = 0;
	while(i<SENS_BUFF_SZ){res+=lightSensBuff[i];i++;}
	res = L_MAX_YARK - (res/SENS_BUFF_SZ)/40;///LIGHT_COEF;// L_MAX_YARK-(res/10000)*2;
	//set2NumsToIndsEff(res, res);
	
	if(res<1) res = 1;
	if(res>59) res = 59;
	return (unsigned short int)res;
}

void initRTC()
{
	#ifdef RTC_INT_CLOCK
	//HSE
	RCC->APB1ENR|=RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
	PWR->CR = PWR_CR_DBP;
	RCC->BDCR|= RCC_BDCR_BDRST;
	RCC->BDCR&=~RCC_BDCR_BDRST;
	RCC->BDCR|= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_HSE;
	RTC->CRL|= RTC_CRL_CNF;
	RTC->PRLL = 62501;
	RTC->CRH = RTC_CRH_SECIE;
	RTC->CRL&=~RTC_CRL_CNF;
	#endif
	#ifdef RTC_EXT_CLOCK
	RCC->APB1ENR|=RCC_APB1ENR_PWREN | RCC_APB1ENR_BKPEN;
	PWR->CR = PWR_CR_DBP;
	RCC->BDCR|= RCC_BDCR_BDRST;
	RCC->BDCR&=~RCC_BDCR_BDRST;
	RCC->BDCR|= RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSE;
	RCC->BDCR |= RCC_BDCR_LSEBYP;//no rezonator
	RCC->BDCR |= RCC_BDCR_LSEON;
		//wait LSE OK
	while((RCC->BDCR & RCC_BDCR_LSERDY) != RCC_BDCR_LSERDY);
	RTC->CRL|= RTC_CRL_CNF;
	RTC->PRLL = 32768;
	RTC->CRH = RTC_CRH_SECIE;
	RTC->CRL&=~RTC_CRL_CNF;
	#endif
	
	}

void initI2C1i()
{
	__disable_irq();
	GPIOB->CRH |= GPIO_CRH_MODE9_1 | GPIO_CRH_MODE8_1;
	
	GPIOB->CRH &=~GPIO_CRH_CNF9;
	GPIOB->CRH |= GPIO_CRH_CNF9_1;
	GPIOB->CRH |= GPIO_CRH_CNF9_0;
	GPIOB->CRH &=~GPIO_CRH_CNF8;
	GPIOB->CRH |= GPIO_CRH_CNF8_1;
	//Set SCl to Pushpull, in opendrain its sometimes error with repeated start
//	GPIOB->CRH |= GPIO_CRH_CNF8_0;//OPenDrain
	
	RCC->APB1ENR|= RCC_APB1ENR_I2C1EN;
	AFIO->MAPR|= AFIO_MAPR_I2C1_REMAP;
	I2C1->CR2 &= ~I2C_CR2_FREQ;
	I2C1->CR2 |= 20;
	I2C1->CR2 |= I2C_CR2_ITEVTEN;
	I2C1->CCR &= ~I2C_CCR_CCR;
	I2C1->CCR|= 2;
	I2C1->CCR|= I2C_CCR_FS;
#ifdef I2C_IPTDMA
	I2C1->CR2|= I2C_CR2_DMAEN; 
#endif
	I2C1->CCR|=I2C_CCR_DUTY;
//	I2C1->CR2|=I2C_CR2_LAST;
	I2C1->TRISE = 7;

	__enable_irq();
	
	
//	NVIC_EnableIRQ(I2C1_EV_IRQn);
//	NVIC_SetPriorityGrouping(0);
//	NVIC_SetPriority(I2C1_EV_IRQn,2);
//	NVIC_SetPriority(
	initI2cParams();
	
}
void initI2cParams(void)
{
	i2c.devAM = RTC_ADR;
	i2c.devMA = RTC_SEC_ADR;
	i2c.devIter = 0;
	i2c.rcvRdy = 0;
	i2c.trmRdy = 0;
	i2c.devRcvCnt = 7;
	i2c.devIterRev = 6;
	i2c.flg = I2C_FREE;

}


void sendI2C(void)
{
	if(i2c.flg == I2C_FREE)
	{
#ifdef I2C_IPTDMA
		initTimeDMA();
#endif
		i2c.flg = I2C_NEED_TRM;
		
		I2C1->CR1 |= I2C_CR1_ACK;
#ifdef I2C_IPTDMA
		I2C1->CR2|=I2C_CR2_LAST; 
#endif
		I2C1->CR1|=I2C_CR1_START;
#ifdef I2C_IPTDMA
		startTimeDma(); 
#endif
	}
}


void i2cGetTmDt(void)
{
//	i2c.devAM|=DS3231_READ;
//	i2c.flg = I2C_NEED_RCV;
//	I2C1->CR1|=I2C_CR1_START;
	
}
void initI2C1()
{
	__disable_irq();
	GPIOB->CRH |= GPIO_CRH_MODE9_1 | GPIO_CRH_MODE8_1 | GPIO_CRH_MODE9_0 | GPIO_CRH_MODE8_0;
	GPIOB->CRH &=~GPIO_CRH_CNF9;
	GPIOB->CRH |= GPIO_CRH_CNF9_1;
	GPIOB->CRH |= GPIO_CRH_CNF9_0;
	GPIOB->CRH &=~GPIO_CRH_CNF8;
	GPIOB->CRH |= GPIO_CRH_CNF8_1;
	GPIOB->CRH |= GPIO_CRH_CNF8_0;
	
	RCC->APB1ENR|= RCC_APB1ENR_I2C1EN;
	AFIO->MAPR|= AFIO_MAPR_I2C1_REMAP;
	I2C1->CR2 &= ~I2C_CR2_FREQ;
	I2C1->CR2 |= 48;
	I2C1->CCR &= ~I2C_CCR_CCR;
	I2C1->CCR|= 24;
	//I2C1->CCR|= I2C_CCR_FS;
	I2C1->TRISE = 3;

	__enable_irq();
}

void  enableI2C1()
{
	I2C1->CR1 |= I2C_CR1_PE;
}

void disableI2C1()
{
	I2C1->CR1 &= ~I2C_CR1_PE;
}

void i2c1Write(char devAdress, char memAdress,register unsigned char* data, register char length)
{
	register int i = 0;
	__disable_irq();
	//set START signal
	I2C1->CR1|= I2C_CR1_START;
	
	//wait while START ok
	while(!(I2C1->SR1&I2C_SR1_SB));
	
	(void)I2C1->SR1;
	
	//send dev adress& write flag
	I2C1->DR = (devAdress|DS3231_WRITE);
	
	//wait dev while adress sended
	while(!(I2C1->SR1 & I2C_SR1_ADDR));
	
	(void)I2C1->SR1;
	(void)I2C1->SR2;
	
	//set adress for write
	I2C1->DR = memAdress;
	
	//wait while adress sended
	while(!(I2C1->SR1 & I2C_SR1_BTF));
	
	while(i<length)
	{
		//write data to dev
		I2C1->DR = data[i];
		//wait while data sended
		while(!(I2C1->SR1 & I2C_SR1_BTF));
		i++;
	}
	I2C1->CR1 |= I2C_CR1_STOP;
	__enable_irq();
}





void initWsGPIO(void)
{
	GPIOB->CRL &=~GPIO_CRL_MODE4;
	GPIOB->CRL |= GPIO_CRL_MODE4_0;
	GPIOB->CRL |= GPIO_CRL_MODE4_1;
	GPIOB->CRL &=~GPIO_CRL_CNF4;
	GPIOB->CRL |= GPIO_CRL_CNF4_0;
}

void ledDbgOn(void)
{
	GPIOA->BSRR &= ~GPIO_BSRR_BR0;
	GPIOA->BSRR |= GPIO_BSRR_BS0;
	flag = 1;
}

void ledDbgOff(void)
{
	GPIOA->BSRR &= ~GPIO_BSRR_BS0;
	GPIOA->BSRR |= GPIO_BSRR_BR0;
	flag = 0;
}

void ledDbgTuggle(void)
{
	if(flag)
		ledDbgOff();
	else
		ledDbgOn();
}

void stopWSDMA(void)
{
	DMA1_Channel6->CCR&=~DMA_CCR_EN;
}
void stopWsTransfer(void)
{
	stopWSDMA();
	stopTimer3();
}


void startWsTransfer(void)
{

	if(flgWSTransfer)return;//Ждем когда закончится предыдущая передача
	flgWSTransfer = 1;//Флаг начала передачи
	stopLightSensDMA();//Выкл. канал датчика освещения т.к. он прерывает канал WS
	startWSDMA();
	startTimer3();
}


unsigned short int getEncVal(){return encoderCounter/ENC_DEV_COEF;}
void setEncVal(unsigned short int val){encoderCounter = val*ENC_DEV_COEF;}
void setMaxEncVal(unsigned short int mval)
{
	if(ENC_DEV_COEF>1)
		encoderMaxValue = (mval*ENC_DEV_COEF)+1;
	else
		encoderMaxValue = mval;
}


void setCath(register unsigned short int cath){GPIOA->ODR &=0x6003;GPIOA->ODR |=cath;}
void setAnode(register unsigned short int cath){GPIOB->ODR &= 0x03ff;GPIOB->ODR |= cath;}


