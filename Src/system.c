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
extern volatile unsigned int sysCounter;//Системный тик, мс (main.c)
static unsigned int i2cStartTm = 0;//Тик начала текущей фоновой транзакции I2C
volatile unsigned int i2cErrCount = 0;//Счетчик перезапусков I2C (для отладки)
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
extern const uint32_t* hwLedAdress;
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

	//Энкодер на внешних прерываниях (EXTI6, EXTI7)
	initEncoderGPIOEXTI();

	startTimer2();
	initInds();
	initWSDMA();
	initTimer3();
	enableInterrupts();
		
	initLightSensBuff();
	initLightSensDMA();
	initADC();
	

}



//Сторожевой таймер IWDG: перезагрузка, если главный цикл не сбросит его за ~2 c
//(LSI 30..60 кГц -> реальный таймаут 1.3..2.7 c)
void initIWDG(void)
{
#ifdef USE_IWDG
	DBGMCU->CR |= DBGMCU_CR_DBG_IWDG_STOP;//Не срабатывать, пока ядро остановлено отладчиком
	IWDG->KR = 0xCCCC;//Запуск (LSI включается автоматически)
	IWDG->KR = 0x5555;//Доступ к PR и RLR
	IWDG->PR = 4;//LSI/64
	IWDG->RLR = 1250;//1250 * 64 / 40000 = 2 c
	while(IWDG->SR & (IWDG_SR_PVU | IWDG_SR_RVU)) {}
	IWDG->KR = 0xAAAA;
#endif
}

void feedIWDG(void)
{
#ifdef USE_IWDG
	IWDG->KR = 0xAAAA;
#endif
}

//Приоритеты прерываний: 0 - высший, 15 - низший (4 бита, все - приоритет вытеснения).
//Главное требование - не допускать дрожания ШИМ ламп: ничто не должно задерживать TIM2
//(мультиплексирование), поэтому он выше всех. Остальные прерывания только выставляют флаги или
//запускают/останавливают периферию и жестких требований ко времени отклика не имеют.
//SysTick остается на самом низком приоритете (15), его выставляет SysTick_Config.
#define PRIO_GDI_PWM 0	//TIM2: мультиплексирование ламп
#define PRIO_DMA 1		//DMA1_Channel6 (конец кадра WS2812), DMA1_Channel7 и I2C1_EV (обмен с DS3231)
#define PRIO_INPUT 2	//EXTI: энкодер и кнопки
#define PRIO_SLOW 3		//RTC (секундный тик) и TIM1 (эффекты смены цифр)

void enableInterrupts()
{
	NVIC_SetPriorityGrouping(3);//Все 4 бита - приоритет вытеснения, подприоритетов нет
	NVIC_SetPriority(TIM2_IRQn, PRIO_GDI_PWM);
	NVIC_SetPriority(DMA1_Channel6_IRQn, PRIO_DMA);
	NVIC_SetPriority(DMA1_Channel7_IRQn, PRIO_DMA);
	NVIC_SetPriority(I2C1_EV_IRQn, PRIO_DMA);
	NVIC_SetPriority(EXTI0_IRQn, PRIO_INPUT);
	NVIC_SetPriority(EXTI1_IRQn, PRIO_INPUT);
	NVIC_SetPriority(EXTI2_IRQn, PRIO_INPUT);
	NVIC_SetPriority(EXTI9_5_IRQn, PRIO_INPUT);
	NVIC_SetPriority(RTC_IRQn, PRIO_SLOW);
	NVIC_SetPriority(TIM1_UP_IRQn, PRIO_SLOW);

	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_EnableIRQ(TIM1_UP_IRQn);
	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(RTC_IRQn);
	NVIC_EnableIRQ(DMA1_Channel6_IRQn);
	NVIC_EnableIRQ(DMA1_Channel7_IRQn);
	NVIC_EnableIRQ(I2C1_EV_IRQn);
}

//Число опросов флага готовности генератора/PLL до отказа (на 8 МГц ~200 мс, кварц стартует за единицы мс)
#define CLK_WAIT_LOOPS 200000UL

//Ожидание флага с ограничением по числу опросов. 1 - флаг выставлен, 0 - таймаут
static unsigned char waitFlag(volatile uint32_t* reg, uint32_t mask, uint32_t loops)
{
	while(!(*reg & mask)) { if(--loops == 0) return 0; }
	return 1;
}

void initSystemClock()
{
	unsigned char hseOk;

	//Flash: 1 такт ожидания для 24 < SYSCLK <= 48 МГц (RM0008, 3.3.3) + буфер предвыборки.
	//Должно быть выставлено ДО переключения на PLL (40 МГц).
	FLASH->ACR = (FLASH->ACR & ~FLASH_ACR_LATENCY) | FLASH_ACR_LATENCY_0 | FLASH_ACR_PRFTBE;
	while((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_0) {}

	//HSE on и ожидание готовности (с таймаутом: раньше при неисправном кварце контроллер висел при старте)
	RCC->CR |= RCC_CR_HSEON;
	hseOk = waitFlag(&RCC->CR, RCC_CR_HSERDY, CLK_WAIT_LOOPS);

	RCC->CFGR |= RCC_CFGR_PPRE1_2;//APB1 = SYSCLK/2
	RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL);
	if(hseOk)
	{
		//HSE как системная частота, пока запускается PLL
		RCC->CFGR &= ~RCC_CFGR_SW;
		RCC->CFGR |= RCC_CFGR_SW_HSE;
		//Enable clock security system (при отказе кварца - NMI, см. NMI_Handler)
		RCC->CR |= RCC_CR_CSSON;
		RCC->CFGR |= RCC_CFGR_PLLSRC;//Источник PLL - HSE
		RCC->CFGR |= RCC_CFGR_PLLMULL5;//8 МГц x5 = 40 МГц
	}
	else
	{
		//Кварц не запустился: PLL от HSI/2 (4 МГц) x10 - те же 40 МГц, поэтому все тайминги остаются
		//верными (HSI менее точен, но часы читаются с DS3231, а WS2812 терпит +-2%).
		RCC->CR &= ~RCC_CR_HSEON;
		RCC->CFGR |= RCC_CFGR_PLLMULL10;
	}
	RCC->CR |= RCC_CR_PLLON;
	if(!waitFlag(&RCC->CR, RCC_CR_PLLRDY, CLK_WAIT_LOOPS)) return;//PLL не запустился - остаемся на HSE/HSI
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	waitFlag(&RCC->CFGR, RCC_CFGR_SWS_PLL, CLK_WAIT_LOOPS);
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
	 RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;  // Enable TIM1 Periph clock
   TIM1->PSC = 1000; // 40 МГц / 1001 = ~40 кГц
   TIM1->ARR = CORE_CLOCK_MHZ;  // период 41 такт: прерывание ~975 Гц (шаг эффектов смены цифр)
   TIM1->DIER |= TIM_DIER_UIE; // прерывание по переполнению
}

void startTimer1()
{
	 TIM1->CR1 |= TIM_CR1_CEN;   // Start count
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
    TIM2->PSC = GDI_TIM_PSC;//шаг ШИМ: 40 МГц/(PSC+1)/(ARR+1) = ~23.8 кГц; кадр GDI_COUNT ламп x 61 шаг: PSC подбирается так, чтобы частота на лампу была ~65 Гц при 4 и при 6 лампах
    TIM2->ARR = GDI_TIM_ARR;
    TIM2->DIER |= TIM_DIER_UIE; // прерывание по переполнению
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
	
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;  // Enable TIM3 Periph clock

	//remap ch1 to pb4
	AFIO->MAPR &= ~AFIO_MAPR_TIM3_REMAP;
	AFIO->MAPR |= AFIO_MAPR_TIM3_REMAP_1;

	TIM3->ARR = 49;//old 59 for 48MHz
	TIM3->CCER |= TIM_CCER_CC1E;
	TIM3->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2|TIM_CCMR1_OC1PE ;
	TIM3->CCR1 = 0;//Пока нет данных - на выходе 0 (иначе первый кадр после старта получит лишний бит)
	//Только запрос DMA. Прерывание по переполнению (UIE) не нужно: конец кадра ловится
	//по окончании передачи DMA (DMA1_Channel6_IRQHandler -> wsTransferComplete)
	TIM3->DIER   |= TIM_DIER_CC1DE;

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
	DMA1_Channel6->CCR &= ~DMA_CCR_EN;//CNDTR можно записать только при выключенном канале
	DMA1->IFCR = DMA_IFCR_CGIF6;
	DMA1_Channel6->CNDTR = WS_COUNT_BITS;
	DMA1_Channel6->CCR = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_EN | DMA_CCR_PSIZE_0|DMA_CCR_TCIE|DMA_CCR_PL_0|DMA_CCR_PL_1;
}
//Постоянная передача на все диоды
void startWSDMACircle()
{
	DMA1_Channel6->CCR &= ~DMA_CCR_EN;//CNDTR можно записать только при выключенном канале
	DMA1->IFCR = DMA_IFCR_CGIF6;
	DMA1_Channel6->CNDTR = WS_COUNT_BITS;
	DMA1_Channel6->CCR = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_EN | DMA_CCR_PSIZE_0|DMA_CCR_CIRC|DMA_CCR_PL_0|DMA_CCR_PL_1;
}

void initWSDMA(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	
	//ws2812DMA
	DMA1_Channel6->CPAR = (uint32_t) &TIM3->CCR1;
	DMA1_Channel6->CMAR = (uint32_t)hwLedAdress;
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
void initLightSensBuff(void){	memset(lightSensBuff,0,sizeof(lightSensBuff));}

void initTimeDMA(void)
{
	DMA1_Channel7->CCR = 0;
	DMA1_Channel7->CPAR = (uint32_t)&I2C1->DR;
	DMA1_Channel7->CMAR =  (uint32_t)i2c.devData;
	DMA1_Channel7->CCR&= ~DMA_CCR_MEM2MEM;//meme2mem disabled
	DMA1_Channel7->CCR|= DMA_CCR_PL_1;//priority high(very high used for ws2812)
	DMA1_Channel7->CCR&=~DMA_CCR_PL_0;
	DMA1_Channel7->CCR&=~DMA_CCR_MSIZE;//память: 8 бит
	DMA1_Channel7->CCR|=DMA_CCR_PSIZE_0;//периферия: 16 бит (DR читается полусловом, в память пишется младший байт)
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
	TIM3->DIER&= ~TIM_DIER_UIE;
}
void initADC()
{
	unsigned int n;
	//ADC Pin

	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;	//Enable clock for port A
	GPIOA->CRL &= ~GPIO_CRL_CNF1;	//Config GPIO
	RCC->CFGR |= RCC_CFGR_ADCPRE_DIV8;	//frequency for adc divided by 8
	//
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN ;    //enable clock adc

	//Калибровка (RM0008, 11.4): АЦП должен быть включен (ADON = 1) и выдержать время стабилизации;
	//после установки CAL бит сбрасывается аппаратно по окончании калибровки (~80 тактов АЦП).
	//Раньше CAL ставился при выключенном АЦП, а цикл ждал не сброса, а установки бита.
	ADC1->CR2 |= ADC_CR2_ADON;
	delay(100);
	ADC1->CR2 |= ADC_CR2_CAL;
	n = 100000;
	while((ADC1->CR2 & ADC_CR2_CAL) && --n) {}

	ADC1->SMPR2 |= ADC_SMPR2_SMP1_0;
	ADC1->CR2 |=ADC_CR2_DMA; //enable work with dma
	ADC1->SQR1 = 0x00000000;//count sequence
	ADC1->SQR2 = 0x00000000;
	ADC1->SQR3 = 0x00000001;
	ADC1->CR2 |= ADC_CR2_CONT;
	ADC1->CR2 |= ADC_CR2_ADON;//Повторная запись ADON при включенном АЦП запускает преобразования
}

unsigned short int getLightSensValue(void)
{
	static unsigned int lastTick = 0;
	static int filt = -1;//Сглаженное значение АЦП, сдвинутое на LIGHT_FILT_SHIFT
	int i = 0,res = 0;
	while(i<SENS_BUFF_SZ){res+=lightSensBuff[i];i++;}
	res = (res/SENS_BUFF_SZ) << LIGHT_FILT_SHIFT;
	//Шум датчика и помехи не должны дергать яркость: усредняем по времени, один шаг в мс
	if(filt < 0) filt = res;
	else if(lastTick != sysCounter)
	{
		lastTick = sysCounter;
		filt += (res - filt)/LIGHT_FILT_DIV;
	}
	res = L_MAX_YARK - (filt >> LIGHT_FILT_SHIFT)/40;///LIGHT_COEF;// L_MAX_YARK-(res/10000)*2;

	if(res<1) res = 1;
	if(res>59) res = 59;
	return (unsigned short int)res;
}

//Предделитель и прерывание секунд. Регистры RTC можно менять только при RTOFF = 1 (RM0008, 18.3.4).
//Частота тика = f_RTCCLK / (PRL + 1), поэтому PRL = f - 1.
static void rtcConfig(uint16_t prl)
{
	while(!(RTC->CRL & RTC_CRL_RTOFF)) {}
	RTC->CRL |= RTC_CRL_CNF;
	RTC->PRLH = 0;
	RTC->PRLL = prl;
	RTC->CRH = RTC_CRH_SECIE;
	RTC->CRL &= ~RTC_CRL_CNF;
	while(!(RTC->CRL & RTC_CRL_RTOFF)) {}
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
	rtcConfig(QUARZ_FREQ/128 - 1);//HSE/128
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
	if(waitFlag(&RCC->BDCR, RCC_BDCR_LSERDY, CLK_WAIT_LOOPS * 10))
		rtcConfig(32768 - 1);//32768 Гц от DS3231
	else
	{
		//Нет сигнала 32 кГц от DS3231 (обрыв, плохой контакт): иначе контроллер висел бы при старте
		//с погашенными лампами. Тактируем RTC от HSE/128, а если и кварц не работает - от LSI.
		//Источник RTC меняется только сбросом backup-домена. Время все равно читается с DS3231
		//на каждом тике, поэтому неточность запасного источника на показания не влияет.
		RCC->BDCR = RCC_BDCR_BDRST;
		RCC->BDCR = 0;
		if(RCC->CR & RCC_CR_HSERDY)
		{
			RCC->BDCR = RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_HSE;
			rtcConfig(QUARZ_FREQ/128 - 1);
		}
		else
		{
			RCC->CSR |= RCC_CSR_LSION;
			waitFlag(&RCC->CSR, RCC_CSR_LSIRDY, CLK_WAIT_LOOPS);
			RCC->BDCR = RCC_BDCR_RTCEN | RCC_BDCR_RTCSEL_LSI;
			rtcConfig(40000 - 1);//LSI ~40 кГц (30..60)
		}
	}
	#endif
	
	}

//Выводы I2C1 (ремап): PB8 - SCL, PB9 - SDA, режим альтернативной функции, 2 МГц
static void i2c1PinsAF(void)
{
	GPIOB->CRH &= ~(GPIO_CRH_MODE8 | GPIO_CRH_CNF8 | GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
	//SCL - AF push-pull (в open-drain иногда были ошибки с повторным стартом)
	GPIOB->CRH |= GPIO_CRH_MODE8_1 | GPIO_CRH_CNF8_1;
	//SDA - AF open-drain
	GPIOB->CRH |= GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1 | GPIO_CRH_CNF9_0;
}

//Сброс и настройка регистров I2C1 (PE остается выключенным)
static void i2c1ConfigRegs(void)
{
	I2C1->CR1 = I2C_CR1_SWRST;
	I2C1->CR1 = 0;
	I2C1->CR2 = 20 | I2C_CR2_ITEVTEN	//FREQ = 20 МГц (APB1)
#ifdef I2C_IPTDMA
		| I2C_CR2_DMAEN
#endif
		;
	I2C1->CCR = I2C_CCR_FS | I2C_CCR_DUTY | 2;//400 кГц
	I2C1->TRISE = 7;
}

void initI2C1i()
{
	RCC->APB1ENR|= RCC_APB1ENR_I2C1EN;
	AFIO->MAPR|= AFIO_MAPR_I2C1_REMAP;
	i2c1PinsAF();
	i2c1ConfigRegs();
	initI2cParams();
}

//Освобождение "зависшей" шины: если ведомый держит SDA в нуле (например, контроллер
//сбросился посреди чтения), подаем до 9 тактов SCL, затем формируем STOP.
static void i2c1BusRecovery(void)
{
	unsigned char i;
	GPIOB->BSRR = GPIO_BSRR_BS8 | GPIO_BSRR_BS9;
	GPIOB->CRH &= ~(GPIO_CRH_MODE8 | GPIO_CRH_CNF8 | GPIO_CRH_MODE9 | GPIO_CRH_CNF9);
	GPIOB->CRH |= GPIO_CRH_MODE8_1;//SCL - выход push-pull (как и в режиме AF)
	GPIOB->CRH |= GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_0;//SDA - выход open-drain
	delay(I2C_RECOVERY_DELAY);
	for(i = 0; (i < 9) && !(GPIOB->IDR & GPIO_IDR_IDR9); i++)
	{
		GPIOB->BSRR = GPIO_BSRR_BR8; delay(I2C_RECOVERY_DELAY);
		GPIOB->BSRR = GPIO_BSRR_BS8; delay(I2C_RECOVERY_DELAY);
	}
	//STOP: SDA 0->1 при SCL = 1
	GPIOB->BSRR = GPIO_BSRR_BR8; delay(I2C_RECOVERY_DELAY);
	GPIOB->BSRR = GPIO_BSRR_BR9; delay(I2C_RECOVERY_DELAY);
	GPIOB->BSRR = GPIO_BSRR_BS8; delay(I2C_RECOVERY_DELAY);
	GPIOB->BSRR = GPIO_BSRR_BS9; delay(I2C_RECOVERY_DELAY);
}

//Полный перезапуск I2C1 после ошибки/таймаута
void i2c1Recover(void)
{
	DMA1_Channel7->CCR &= ~DMA_CCR_EN;
	DMA1->IFCR = DMA_IFCR_CGIF7;
	I2C1->CR1 = 0;//PE = 0 - отпускаем выводы
	i2c1BusRecovery();
	i2c1PinsAF();
	i2c1ConfigRegs();
	initI2cParams();
	I2C1->CR1 |= I2C_CR1_PE;
	i2cErrCount++;
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
		i2cStartTm = sysCounter;//Для контроля таймаута (i2cCheckTimeout)

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



void  enableI2C1()
{
	I2C1->CR1 |= I2C_CR1_PE;
}


//Контроль фонового чтения DS3231. Вызывать из главного цикла.
//Если транзакция не завершилась за I2C_TIMEOUT_MS (NACK, помеха, зависшая шина) -
//перезапускаем I2C и возвращаем 1, чтобы вызывающий код разрешил новое чтение.
unsigned char i2cCheckTimeout(void)
{
	if((i2c.flg == I2C_FREE) || i2c.rcvRdy) return 0;
	if((unsigned int)(sysCounter - i2cStartTm) < I2C_TIMEOUT_MS) return 0;
	i2c1Recover();
	return 1;
}

//Ожидание флага в SR1 с ограничением по времени. 0 - ок, 1 - NACK или таймаут
static unsigned char i2c1WaitSR1(uint32_t flag)
{
	uint32_t n = I2C_WAIT_LOOPS;
	while(!(I2C1->SR1 & flag))
	{
		if(I2C1->SR1 & I2C_SR1_AF) return 1;
		if(--n == 0) return 1;
	}
	return 0;
}

//Блокирующая запись в DS3231 (установка времени/даты). 0 - ок, 1 - ошибка.
//Прерывания больше не запрещаются: на время записи отключаются только прерывания
//событий I2C и запросы DMA от I2C (I2C1_TX сидит на канале DMA6 вместе с WS2812).
unsigned char i2c1Write(char devAdress, char memAdress, unsigned char* data, char length)
{
	unsigned char err = 0;
	uint32_t n = I2C_WAIT_LOOPS;
	uint32_t cr2;
	int i;

	//Дожидаемся окончания фонового чтения времени, если оно идет
	while((i2c.flg != I2C_FREE) && !i2c.rcvRdy && --n) {}
	if(n == 0) i2c1Recover();

	cr2 = I2C1->CR2;
	I2C1->CR2 = cr2 & ~(I2C_CR2_ITEVTEN | I2C_CR2_DMAEN | I2C_CR2_LAST);

	I2C1->CR1 |= I2C_CR1_START;
	err = i2c1WaitSR1(I2C_SR1_SB);
	if(!err)
	{
		I2C1->DR = (devAdress|DS3231_WRITE);
		err = i2c1WaitSR1(I2C_SR1_ADDR);
	}
	if(!err)
	{
		(void)I2C1->SR2;//сброс ADDR
		I2C1->DR = memAdress;
		err = i2c1WaitSR1(I2C_SR1_BTF);
	}
	for(i = 0; !err && (i < length); i++)
	{
		I2C1->DR = data[i];
		err = i2c1WaitSR1(I2C_SR1_BTF);
	}
	I2C1->CR1 |= I2C_CR1_STOP;
	n = I2C_WAIT_LOOPS;
	while((I2C1->CR1 & I2C_CR1_STOP) && --n) {}

	if(err || (n == 0))
	{
		i2c1Recover();//заодно восстановит CR2
		return 1;
	}
	I2C1->CR2 = cr2;
	return 0;
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
//Вызывается из прерывания DMA по окончании передачи всего буфера (одиночный режим).
//Последние WS_RES_VAL слотов буфера - нули (сигнал RES), поэтому к этому моменту
//все данные уже выданы, а на линии низкий уровень - таймер можно сразу остановить.
void wsTransferComplete(void)
{
	stopTimer3();
	stopWSDMA();
	startLightSensDMA();//Возвращаем канал датчика освещения
	flgWSTransfer = WS_TRANSFER_READY;//Передача закончена - можно передавать заново
}


//Состояние отложенного запуска кадра WS2812
static volatile unsigned char wsPending = 0;
static unsigned int wsStartTm = 0;

void startWsTransfer(void)
{
	//Канал DMA6 общий для TIM3_CH1 (WS2812) и I2C1_TX (запросы DMA от I2C включены постоянно).
	//Если идет предыдущий кадр или обмен с DS3231 - запрос не теряем, а откладываем:
	//его выполнит wsTransferPoll() из главного цикла.
	if(flgWSTransfer || (i2c.flg != I2C_FREE)){ wsPending = 1; return; }
	wsPending = 0;
	flgWSTransfer = 1;//Флаг начала передачи
	wsStartTm = sysCounter;
	stopLightSensDMA();//Выкл. канал датчика освещения т.к. он прерывает канал WS
	startWSDMA();
	startTimer3();
}


//Вызывается из главного цикла: запускает отложенный кадр и страхует от потери прерывания DMA
void wsTransferPoll(void)
{
	if(flgWSTransfer)
	{
		if((unsigned int)(sysCounter - wsStartTm) >= WS_TRANSFER_TIMEOUT_MS) wsTransferComplete();
		return;
	}
	if(wsPending) startWsTransfer();
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


