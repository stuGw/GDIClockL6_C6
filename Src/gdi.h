#ifndef GDI_H
#define GDI_H
#include <string.h>
#include "system.h"
#include "devconfdef.h"


#ifdef MODULE_IN4_4
#define CATHODE_9 0x0010//k
#define CATHODE_8 0x0020//k
#define CATHODE_7 0x0008//k
#define CATHODE_6 0x0040//k
#define CATHODE_5 0x0200//k
#define CATHODE_4 0x0080//k
#define CATHODE_3 0x0400//k
#define CATHODE_2 0x0800//
#define CATHODE_1 0x0100//k
#define CATHODE_0 0x1000//k
#define ZPT_L 0
#define ZPT_R 0

#define LAMP_1 0x4000
#define LAMP_2 0x8000
#define LAMP_3 0x1000
#define LAMP_4 0x2000
#define LAMP_5 0
#define LAMP_6 0
#endif

#ifdef MODULE_IN4
#define CATHODE_6 0x0010//k6
#define CATHODE_4 0x0020//k4
#define CATHODE_8 0x0008//k8
#define CATHODE_9 0x0040//k9
#define CATHODE_5 0x0200//k
#define CATHODE_7 0x0080//k7
#define CATHODE_3 0x0400//k
#define CATHODE_2 0x0800//
#define CATHODE_1 0x0100//k
#define CATHODE_0 0x1000//k
#define ZPT_L 0
#define ZPT_R 0


#define LAMP_3 0x4000
#define LAMP_4 0x8000
#define LAMP_1 0x1000
#define LAMP_2 0x2000
#define LAMP_5 0x0400
#define LAMP_6 0x0800
#endif

#ifdef MODULE_IN4_4DBG
#define CATHODE_9 0x0010//k6
#define CATHODE_8 0x0020//k4
#define CATHODE_7 0x0008//k8
#define CATHODE_6 0x0040//k9
#define CATHODE_5 0x0200//k
#define CATHODE_4 0x0080//k7
#define CATHODE_3 0x0400//k
#define CATHODE_2 0x0800//
#define CATHODE_1 0x0100//k
#define CATHODE_0 0x1000//k
#define ZPT_L 0
#define ZPT_R 0


#define LAMP_3 0x4000
#define LAMP_4 0x8000
#define LAMP_5 0x1000
#define LAMP_6 0x2000
#define LAMP_1 0
#define LAMP_2 0
#endif

#ifdef MODULE_IN14_R
#define CATHODE_9 0x0100//k
#define CATHODE_7 0x0020//k
#define CATHODE_8 0x0080//k
#define CATHODE_2 0x0200//k
#define CATHODE_6 0x0010//k
#define CATHODE_0 0x0400//k
#define CATHODE_5 0x0008//k
#define CATHODE_4 0x0004//
#define CATHODE_3 0x0040//k
#define CATHODE_1 0x8000//k
#define ZPT_L 0x1000
#define ZPT_R 0x0800


#define LAMP_1 0x0400
#define LAMP_2 0x0800
#define LAMP_3 0x1000
#define LAMP_4 0x2000
#define LAMP_5 0x4000
#define LAMP_6 0x8000
#endif


#ifdef MODULE_IN14_SPB
#define CATHODE_9 0x0080//k
#define CATHODE_7 0x0020//k
#define CATHODE_8 0x0040//k
#define CATHODE_2 0x1000//k
#define CATHODE_6 0x0010//k
#define CATHODE_0 0x0200//k
#define CATHODE_5 0x0008//k
#define CATHODE_4 0x0004//
#define CATHODE_3 0x0100//k
#define CATHODE_1 0x0800//k
#define ZPT_L 0x0400
#define ZPT_R 0x0400


#define LAMP_1 0x4000
#define LAMP_2 0x2000
#define LAMP_3 0x1000
#define LAMP_4 0x8000
#define LAMP_5 0x0400
#define LAMP_6 0x0800
#endif

#ifdef MODULE_IN8_SPB
#define CATHODE_9 0x0080//k
#define CATHODE_7 0x0010//k
#define CATHODE_8 0x0040//k
#define CATHODE_2 0x0800//k
#define CATHODE_6 0x0008//k
#define CATHODE_0 0x0200//k
#define CATHODE_5 0x0004//k
#define CATHODE_4 0x0100//
#define CATHODE_3 0x1000//k
#define CATHODE_1 0x0400//k
#define ZPT_L 0x0020
#define ZPT_R 0x0020


#define LAMP_1 0x4000
#define LAMP_2 0x2000
#define LAMP_3 0x1000
#define LAMP_4 0x8000
#define LAMP_5 0x0400
#define LAMP_6 0x0800
#endif

#ifdef MODULE_ZM570M
#define CATHODE_0 0x0800//k
#define CATHODE_1 0x1000//k
#define CATHODE_2 0x8000//k
#define CATHODE_3 0x0200//k
#define CATHODE_4 0x0004//k
#define CATHODE_5 0x0008//k
#define CATHODE_6 0x0010//k
#define CATHODE_7 0x0020//
#define CATHODE_8 0x0080//k
#define CATHODE_9 0x0400//k
#define ZPT_L 0x0040
#define ZPT_R 0x0100


#define LAMP_1 0x0400
#define LAMP_2 0x0800
#define LAMP_3 0x1000
#define LAMP_4 0x2000
#define LAMP_5 0x4000
#define LAMP_6 0x8000
#endif

#ifdef MODULE_IN17
//#define CONF_BUTT
#define CATHODE_9 0x0040//k
#define CATHODE_7 0x0400//k
#define CATHODE_8 0x0800//k - 5800
#define CATHODE_2 0x0008//k
#define CATHODE_6 0x0200//k - 8200
#define CATHODE_0 0x0010//k
#define CATHODE_5 0x0100//k - 6100
#define CATHODE_4 0x0080//
#define CATHODE_3 0x0020//k
#define CATHODE_1 0x0004//k
#define ZPT_L 0x0000
#define ZPT_R 0x0000

#define LAMP_6 0x0400
#define LAMP_5 0x0800
#define LAMP_4 0x1000
#define LAMP_3 0x2000
#define LAMP_2 0x4000
#define LAMP_1 0x8000
#endif


#ifdef MODULE_IN12Z_R
#define CATHODE_3 0x0040//k3
#define CATHODE_0 0x0400//k0
#define CATHODE_1 0x0800//k1 
#define CATHODE_7 0x0008//k7
#define CATHODE_9 0x0200//k9 
#define CATHODE_5 0x0010//k5
#define CATHODE_2 0x0100//k2 
#define CATHODE_8 0x0080//8
#define CATHODE_4 0x0020//k4
#define CATHODE_6 0x0004//k6
#define ZPT_R 0x1000


#define LAMP_6 0x0400
#define LAMP_5 0x0800
#define LAMP_4 0x1000
#define LAMP_3 0x2000
#define LAMP_2 0x4000
#define LAMP_1 0x8000
#endif

#ifdef MODULE_IN14
#define CATHODE_8 0x0040//k8
#define CATHODE_0 0x0400//k0
#define CATHODE_9 0x0100//k9
#define CATHODE_5 0x0008//k5
#define CATHODE_1 0x0800//k1
#define CATHODE_7 0x0010//k7
#define CATHODE_2 0x0200//k2
#define CATHODE_3 0x0080//3
#define CATHODE_4 0x0020//k4
#define CATHODE_6 0x0004//k6
#define ZPT_L 0x1000
#endif

#ifdef MODULE_IN8_DBG
#define CATHODE_2 0x0040//kzz
#define CATHODE_5 0x0010//k00
#define CATHODE_8 0x0100//k99
#define CATHODE_6 0x0008//k66
#define CATHODE_9 0x0800//k22
#define ZPT_L 0x0400//k88
#define CATHODE_4 0x0200//k33
#define CATHODE_3 0x0080//44
#define CATHODE_1 0x0020//k55
#define CATHODE_7 0x0004//k77
#define CATHODE_0 0x1000
#endif


#ifdef MODULE_IN8_R
#define CATHODE_2 0x8000//kzz
#define CATHODE_5 0x0004//k00
#define CATHODE_8 0x0080//k99
#define CATHODE_6 0x0008//k66
#define CATHODE_9 0x0100//k22
#define ZPT_L 0x0020//k88
#define ZPT_R 0x0020//k88
#define CATHODE_4 0x0040//k33
#define CATHODE_3 0x0200//44
#define CATHODE_1 0x1000//k55
#define CATHODE_7 0x0010//k77
#define CATHODE_0 0x0400

#define LAMP_1 0x0400
#define LAMP_2 0x0800
#define LAMP_3 0x1000
#define LAMP_4 0x2000
#define LAMP_5 0x4000
#define LAMP_6 0x8000


#endif

#ifdef MODULE_IN8_RN
#define CATHODE_7 0x0400
#define CATHODE_3 0x0020
#define CATHODE_6 0x0080
#define CATHODE_8 0x0800
#define CATHODE_2 0x0010
#define CATHODE_4 0x0100
#define CATHODE_9 0x1000
#define ZPT_L 0x0040
#define CATHODE_5 0x0200
#define CATHODE_1 0x0004
#define CATHODE_0 0x0008

#define LAMP_5 0x400
#define LAMP_6 0x800
#define LAMP_4 0x1000
#define LAMP_2 0x2000
#define LAMP_1 0x4000
#define LAMP_3 0x8000


#endif

#ifdef MODULE_IN12_6
#define CATHODE_0 0x0010
#define CATHODE_1 0x0008
#define CATHODE_2 0x0080
#define CATHODE_3 0x0020
#define CATHODE_4 0x1000
#define CATHODE_5 0x0400
#define CATHODE_6 0x0200
#define CATHODE_7 0x0100
#define CATHODE_8 0x0800
#define CATHODE_9 0x0040
#define ZPT_L 0x0004
#define ZPT_R 0x0004


#define LAMP_1 0x0800
#define LAMP_2 0x0400
#define LAMP_3 0x1000
#define LAMP_4 0x2000
#define LAMP_5 0x4000
#define LAMP_6 0x8000
#endif


#ifdef MODULE_IN14_SP
#define CATHODE_0 0x0008
#define CATHODE_1 0x0010
#define CATHODE_2 0x0020
#define CATHODE_3 0x0100
#define CATHODE_4 0x0200
#define CATHODE_5 0x0080
#define CATHODE_6 0x0400
#define CATHODE_7 0x0040
#define CATHODE_8 0x0800
#define CATHODE_9 0x1000
#define ZPT_L 0x4
#define ZPT_R 0x4


#define LAMP_1 0x4000
#define LAMP_2 0x2000
#define LAMP_3 0x8000
#define LAMP_4 0x1000
#define LAMP_5 0x0400
#define LAMP_6 0x0800
#endif

#define L_MAX_YARK 60 
#define L_MIN_YARK 2  
#define L_INTERVAL 5

#define GDI_COUNT_EFF 7
#define GDI_EFF_STD 0
#define GDI_EFF_RUE 1
#define GDI_EFF_2CF 2
#define GDI_EFF_PL1 3
#define GDI_EFF_ALL 4
#define GDI_EFF_PL2 5
#define GDI_EFF_PL3 6


#define FIRE_EFF_MAX 2
#define FIRE_EFF_OFF 0
#define FIRE_EFF1 1
#define FIRE_EFF2 2
#define GDI_EFF_TM 50
#define MAX_FADE_TIMER_VALUE 900
#define MAX_RUES_VALUE 600
#define FIRE_COEF_CNT 182

#define FTSK_0 0x01
#define FTSK_1 0x02
#define FTSK_2 0x04
#define FTSK_3 0x08
#define FTSK_4 0x10
#define FTSK_5 0x20

//10,11 - точки, 12 - отключить
#define GDI_CM1 10
#define GDI_CM2 11
#define GDI_OFF 12
#define GDI_AUTOBR_MIN 3

#define SEC_CATHODE 0x00
#define SEC_OFF 0x01
#define SEC_COMMA1 0x03
#define SEC_COMMA2 0x02
//Флаг для  плавной смены индикаторов
extern unsigned short int flgFTsk;
extern const unsigned char tskFArr[6];
//Номер текущего эффекта
extern unsigned char currGdiEff[];
extern unsigned char cge;

//структура описывает Газоразрядный индикатор
struct lamps
{
	unsigned short int cathTm;//Время катода(для плавных изменений)
	unsigned short int codeC;//Код для вывода на порт( содержит биты включенных катодов)
	unsigned char brAll;//Яркость для всего индикатора
	unsigned char br[12];//Яркость конкретного катода 
	unsigned char prevC;//Индекс предыдущего катода
	unsigned char currC;//Индекс Текущего катода
	unsigned short int zptL;//Флаг включения запятой
	unsigned short int zptR;
	unsigned char nRueCat;//
};



extern const unsigned short int LCat[]; //Коды катодов
extern const unsigned short int LAn[]; //Коды анодов

//Позиции для переключение методом перебора
extern const unsigned char in14CathsPos[];// = {1,0,2,4,6,8,9,7,5,3};

//Индикаторы
extern struct lamps gdi[];
//Коэффициент для эффекта горения
extern char fireBrightCoef;

//Эффект горения
void fireEffGdi(int sc);
//Автоматическое получение значенияя ркости с датчика
void autoBright(void);
//Установка яркости для всех индикаторов
void setGdiBrightAll(unsigned char value);

//Начальная инициализация
void initInds(void);
//Зажигание индикатров в стандартном режиме
void fireIndCycle(void);
//Зажигание индикаторов в режиме эффекта горения
void fireIndFireEffCycle(void);
//ить на индикаторе 2 3х значных числа 
void setHw2NumsToInds(unsigned int n1, unsigned int n2);
void setHwNumToInds(unsigned int number);
//Отобразить строку на индикаторах
void setHwStringToInds(const char* str);


void setHwValueToInd(unsigned char ind, unsigned char value, unsigned char yark);
void onIndComma(unsigned char ind);//{ gdi[ind].br[10] = gdi[ind].brAll;}
 void offIndComma(unsigned char ind);// { gdi[ind].br[10] = 0; }
void fireCurrInd(unsigned char currIndex);
void offInds(void);
void offCommas(void);


 void fireCurrIndCath(unsigned char ind, unsigned char cath);
void offCurrIndCath(unsigned char ind, unsigned char cath);
 void setBrightAll(unsigned char value);
unsigned char getPrevValue(unsigned char value);

void set2NumsToIndsEff(unsigned int n1, unsigned int n2);
 void setIndCath(register unsigned char ind,register unsigned char cath);
 void setIndCathFade(register unsigned char ind,register unsigned char cath);
 void setIndCathRue(register unsigned char ind,register unsigned char cath);
 void setIndCathRueS(register unsigned char ind,register unsigned char cath);
 void setIndictorCathBright(unsigned char ind, unsigned char cath, unsigned char bright);
 void setIndCathTwoCaths(register unsigned char ind,register unsigned char cath);

 void cathTwoCaths(unsigned char ind);
 void cathRue(unsigned char ind);
 void cathRueS(unsigned char ind);
 void cathFadeAll(unsigned char ind);
 void cathFade(unsigned char ind);
 void cathFadePrv(unsigned char ind);
 void cathFadeTwo(unsigned char ind);

//Ubrat' dannui catod s dannogo indicatora

extern void (*setIndCathEff[])(register unsigned char ind,register unsigned char cath);
extern void (*catFade[])(unsigned char ind);

void gdiEffHandle(void);
/*{
	if(tmdt.sec == 0) cathRueS(0);
		if(flgFTsk&FTSK_5) catFade[currGdiEff[5]](5);
		if(flgFTsk&FTSK_4) catFade[currGdiEff[4]](4);
		if(flgFTsk&FTSK_3) catFade[currGdiEff[3]](3);
		if(flgFTsk&FTSK_2) catFade[currGdiEff[2]](2);
		if(flgFTsk&FTSK_1) catFade[currGdiEff[1]](1);
		if(flgFTsk&FTSK_0) catFade[currGdiEff[0]](0);
}*/

void gdiEffSet(unsigned char ind, unsigned char val);

void handleIndFire(void);
void handleIndFireAll(void);
void setSecView(unsigned char flg);

#endif //GDI_H
