#ifndef DS3231_H
#define DS3231_H

#define RTC_ADR 0xD0
#define RTC_WR 0x00
#define RTC_RD 0x01

#define RTC_SEC_ADR 0x00
#define RTC_MIN_ADR 0x01
#define RTC_HR_ADR 0x02
#define RTC_WDAY_ADR 0x03
#define RTC_MDAY_ADR 0x04
#define RTC_MONTH_ADR 0x05
#define RTC_YEAR_ADR 0x06
#define RTC_AM1_SEC 0x07
#define RTC_AM1_MIN 0x08
#define RTC_AM1_HR 0x09
#define RTC_AM1_DATE 0x0A
#define RTC_AM2_MIN 0x0B
#define RTC_AM2_HR 0x0C
#define RTC_AM2_DATE 0x0D
#define RTC_TEMPH_ADR 0x11
#define RTC_TEMPL_ADR 0x12

#define RTC_CR_ADR 0x0E
#define RTC_SR_ADR 0x0F
#define RTC_AGOFFS_ADR 0x10
#define RTC_TEMPH_ADR 0x11
#define RTC_TEMPL_ADR 0x12

#define RTC_OSC_START 0xfe
#define RTC_OSC_STOP 0x01
#define RTC_HOUR_10 0x10
#define RTC_HOUR_20 0x20

#include "system.h"

void ds3231StartOsc(void);
void ds3231StopOsc(void);
void ds3231GetDate(unsigned char* dateBuffer);
void ds3231GetTime(unsigned char* timeBuffer);
void ds3231SetDate(unsigned char* dateBuffer);
void ds3231SetTime(unsigned char* timeBuffer);
void ds3231GetTemp(char* tempBuffer);
#endif //DS3231
