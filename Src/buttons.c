#include "buttons.h"
#include "system.h"
#include "devconfdef.h"
#include "gdi.h"

extern char statusb;
volatile extern char down,up;
volatile char flgLongButt = 0;
extern unsigned short int encoderMaxValue;

void initButtons(volatile struct Button* bt0,volatile struct Button* bt1,volatile  struct Button* bt2)
{
	clearButtState(bt0);
	clearButtState(bt1);
	clearButtState(bt2);
	bt0->ipt = 0;
	bt2->ipt = 0;
	bt1->ipt = 0;
	bt0->buttBit = BUTT_NEXT_PB0;
	bt2->buttBit = BUTT_PREV_PB2;	
	bt1->buttBit = BUTT_ENTER_PB1;
}
void clearButtState(volatile struct Button* bt)
{
	bt->st = 0;
	bt->up = 0;
	bt->dn = 0;
	bt->tm = 0;
	bt->tmd = 0;
	bt->flagPare = 0;
	bt->ipt = 0;
}

char checkButtState(volatile struct Button* bt)
{
	char state = 0;
	
	if(bt->st)//Если кнопка в работе
	{
		if(bt->dn >= 2){up = 2;state = BUTT_DOUBLECLICK;}
		else
		if((bt->tm>BUTT_LIFETIME)&&(bt->dn == 1))
		{
			if(bt->buttBit&RG_BUTT_INPREG) state = BUTT_SHORTCLICK;
			else state = BUTT_LONGCLICK;
		}
	}		
	
	if(bt->tm>BUTT_LIFETIME){clearButtState(bt);}
	if(state){clearButtState(bt);}
	return statusb = state;
}
//Обработка прерывания по нажатию кнопки
char handleButtInt(volatile struct Button* bt)
{
	if((bt->ipt == 1)&&(bt->tmd>BUTT_DELAY))
	{
		if(!(bt->buttBit&RG_BUTT_INPREG)){bt->ipt = 0; bt->tmd = 0; bt->dn++;}
	}
	
	/*if((bt->ipt == 2)&&(bt->tmd>30))
	{
		if((bt->buttBit&RG_BUTT_INPREG))//все еще внизу
		{if(bt->dn!=0)bt->up++;
			bt->ipt = 0; bt->tmd = 0; //bt->up++; 
		//	up++;
		}
	}*/
		return checkButtState(bt);
}


signed char handleEncoder(unsigned short int *val)
{
	static	unsigned short int res = 0;
	signed char s = 0;
	*val = getEncVal();
	if(((*val>res)&&((*val-res)==1))||((res-*val)==(encoderMaxValue/ENC_DEV_COEF)))s = 1;
	else if(((*val<res)&&((res-*val)==1))||((*val-res)==(encoderMaxValue/ENC_DEV_COEF)))s = -1;
	res = *val;
	return s;
}

