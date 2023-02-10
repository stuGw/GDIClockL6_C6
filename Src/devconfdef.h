#ifndef DEVCONFDEF_H
#define DEVCONFDEF_H
//Файл описывает параметры для различных типов часов
//Тип энкодера
#define ENC_DEV_COEF 1
//alum bt1
//in4 bt1
//in12_6 bt2
//#define ENC_BT0
//#define ENC_BT1
#define ENC_BT2



//Управление
#define CONF_BUTT
//#define CONF_B_ENC
#define RTC_EXT_CLOCK
//#define RTC_INT_CLOCK
//Тип ламп
//#define MODULE_IN4
//#define MODULE_IN4_4DBG
//#define MODULE_IN8_SPB
#define MODULE_IN14_SPB
//#define MODULE_IN8_R
//#define MODULE_IN8_RN
//#define MODULE_IN14_R
//#define MODULE_IN8_DBG
//#define MODULE_IN17
//#define MODULE_IN12Z_R
//#define MODULE_IN14_SP
//#define MODULE_ZM570M
//#define MODULE_IN12_6
//Время отображения даты и времени(в секундах)
#define SHOW_DATE_SEC 5
#define SHOW_TIME_SEC 30


#define SHOW_DT_M 5
#define SHOW_DT_H 7
#define SHOW_DT_S 0

#define SHOW_DT_M2 00
#define SHOW_DT_H2 23
#define SHOW_DT_S2 0
#endif
