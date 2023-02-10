#ifndef BUTTONS_H
#define BUTTONS_H

#define BUTT_CHECK_TIME 100
#define BUTT_COUNT_CLD_ONE 1
#define BUTT_LONG 3
#define BUTT_SHORT 1
#define BUTT_COUNT_CLD_TWO 2
#define BUTT_DOUBLE 2
#define BUTT_ON 1
#define BUTT_OFF 0
#define BUTT_CNT_LONG 800
#define BUTT_CNT_SHORT 70
#define BUTT_CLK_LONG 3
#define BUTT_CLK_DOUBLE 2
#define BUTT_CLK_SHORT 1

#define BUTT_DOUBLECLICK 2
#define BUTT_SHORTCLICK 1
#define BUTT_LONGCLICK 3
#define BUTT_DOUBLECLICK_P 12
#define BUTT_SHORTCLICK_P 11
#define BUTT_LONGCLICK_P 13
#define BUTT_LIFETIME 500
#define BUTT_DELAY 30
 struct Button
{
	volatile char st;
		volatile char dn;
		volatile char up;
		volatile unsigned int tm;
		volatile unsigned int tmd;
		volatile char ipt;
		volatile char flagPare;
		volatile short int buttBit;
};

void clearButtState(volatile struct Button* bt);
char checkButtState(volatile struct Button* bt);
//Обработка прерывания по нажатию кнопки
char handleButtInt(volatile struct Button* bt);
signed char handleEncoder(unsigned short int *val);
void initButtons(volatile struct Button* bt0, volatile struct Button* bt1, volatile struct Button* bt2);

#endif//BUTTONS_H
