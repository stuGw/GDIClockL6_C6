#ifndef SETTINGS_H
#define SETTINGS_H
#include <stdint.h>
#include "menu.h"

//Сохранение настроек во внутреннюю flash (последняя страница, см. settings.c).
//Запись выполняется после того, как настройки не менялись SETTINGS_SAVE_DELAY_MS и меню закрыто.
#define SETTINGS_SAVE_DELAY_MS 5000

extern volatile unsigned int settingsSaveCount;//Число успешных записей (для отладки)
extern volatile unsigned int settingsErrCount;//Число неудачных записей (для отладки)

//Вызывается один раз при старте после initConfig(): применяет сохраненные настройки, если они есть
void settingsLoad(void);
//Вызывается из главного цикла: при изменении настроек записывает их во flash
void settingsPoll(void);

//Низкий уровень (журнал записей на странице flash). 1 - успех, 0 - нет записи или ошибка
unsigned char settingsFlashRead(uint16_t* v);
unsigned char settingsFlashWrite(const uint16_t* v);

#ifdef SETTINGS_TEST
void settingsTestReset(void);
#endif
#endif //SETTINGS_H
