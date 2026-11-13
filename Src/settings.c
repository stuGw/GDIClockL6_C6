#include <string.h>
#include "settings.h"
#include "system.h"
#include "ws2812.h"
#include "gdi.h"
#include "menu.h"

/*
 * Сохранение настроек во внутреннюю flash.
 *
 * Под настройки отведена последняя страница flash (1 КБ), адрес задает линковщик (_settings_page).
 * Страница работает как журнал: каждая запись (SetRec) дописывается в первый свободный слот,
 * действующей считается последняя корректная запись. Страница стирается только когда журнал
 * заполнен (раз в SET_REC_COUNT сохранений), что на порядки продлевает ресурс flash.
 *
 * Запись корректна, если совпадают признак, версия и контрольная сумма. Недописанная запись
 * (пропало питание) контрольную сумму не пройдет и будет пропущена: действующей останется
 * предыдущая. Если питание пропало в момент стирания страницы, будут заводские настройки.
 *
 * Запись во flash останавливает ядро на время операции (стирание ~20-40 мс, программирование
 * полуслова ~60 мкс), поэтому на это время прерывания запрещаются, а лампы гасятся: иначе
 * текущая лампа горела бы постоянным током. Сохраняем только когда нет обмена по I2C и передачи
 * на светодиоды, и не раньше чем через SETTINGS_SAVE_DELAY_MS после последнего изменения.
 * HSI должен быть включен (он не выключается нигде в проекте).
 */

#define SET_PAGE_SIZE 1024u
#define SET_MAGIC 0x51C0u
#define SET_VERSION 1u			//Увеличить при изменении состава или порядка значений (menu.c)
#define SET_CHECK_MS 100u		//Период проверки изменений
#define SET_MAX_RETRY 5			//Число попыток записи после ошибок до следующего изменения настроек

struct SetRec
{
	uint16_t magic;
	uint16_t version;
	uint16_t v[CONF_SAVED_COUNT];
	uint16_t crc;//Последним: считается по всем предыдущим полям
};
_Static_assert(sizeof(struct SetRec) % 2 == 0, "запись должна состоять из целого числа полуслов");
#define SET_REC_SIZE ((unsigned)sizeof(struct SetRec))
#define SET_REC_COUNT (SET_PAGE_SIZE/SET_REC_SIZE)
#define SET_REC_HW (SET_REC_SIZE/2u)

volatile unsigned int settingsSaveCount = 0;
volatile unsigned int settingsErrCount = 0;

extern volatile unsigned int sysCounter;
extern volatile unsigned char flgWSTransfer;
extern volatile unsigned char flagConfMode;
extern struct I2CDev i2c;

/* ------------------------------ низкий уровень: flash ------------------------------ */
#ifdef SETTINGS_TEST
#include "settings_test_hw.h"
#else
extern uint8_t _settings_page[];
#define SET_BASE ((uint32_t)_settings_page)
#define HW_WAIT_LOOPS 4000000UL

static void hwUnlock(void)
{
	if(FLASH->CR & FLASH_CR_LOCK){ FLASH->KEYR = FLASH_KEY1; FLASH->KEYR = FLASH_KEY2; }
}

static void hwLock(void)
{
	FLASH->CR |= FLASH_CR_LOCK;
}

static unsigned char hwWait(void)
{
	uint32_t n = HW_WAIT_LOOPS;
	while(FLASH->SR & FLASH_SR_BSY){ if(--n == 0) return 0; }
	return 1;
}

//Итог операции: флаг окончания есть, ошибок нет. Флаги сбрасываются записью единицы.
static unsigned char hwFinish(void)
{
	uint32_t sr = FLASH->SR;
	FLASH->SR = FLASH_SR_EOP | FLASH_SR_PGERR | FLASH_SR_WRPRTERR;
	return (sr & FLASH_SR_EOP) && !(sr & (FLASH_SR_PGERR | FLASH_SR_WRPRTERR));
}

//Стирание страницы настроек
static unsigned char hwErase(void)
{
	unsigned char ok;
	if(!hwWait()) return 0;
	FLASH->SR = FLASH_SR_EOP | FLASH_SR_PGERR | FLASH_SR_WRPRTERR;
	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR = SET_BASE;
	FLASH->CR |= FLASH_CR_STRT;
	ok = hwWait();
	FLASH->CR &= ~FLASH_CR_PER;
	return ok && hwFinish();
}

//Запись полуслова (ячейка должна быть стерта)
static unsigned char hwProgram(uint32_t addr, uint16_t data)
{
	unsigned char ok;
	if(!hwWait()) return 0;
	FLASH->SR = FLASH_SR_EOP | FLASH_SR_PGERR | FLASH_SR_WRPRTERR;
	FLASH->CR |= FLASH_CR_PG;
	*(volatile uint16_t*)addr = data;
	ok = hwWait();
	FLASH->CR &= ~FLASH_CR_PG;
	return ok && hwFinish();
}
#endif

/* ------------------------------ журнал записей ------------------------------ */
static uint16_t crc16(const uint16_t* p, unsigned n)
{
	uint16_t crc = 0xFFFF;
	unsigned i, b;
	for(i = 0; i < n; i++)
	{
		crc ^= p[i];
		for(b = 0; b < 16; b++) crc = (crc & 1) ? (uint16_t)((crc >> 1) ^ 0xA001) : (uint16_t)(crc >> 1);
	}
	return crc;
}

static const volatile uint16_t* slotPtr(unsigned i)
{
	return (const volatile uint16_t*)(SET_BASE + i*SET_REC_SIZE);
}

static unsigned char slotErased(unsigned i)
{
	unsigned k;
	for(k = 0; k < SET_REC_HW; k++) if(slotPtr(i)[k] != 0xFFFF) return 0;
	return 1;
}

//Читает слот; 1 - запись корректна
static unsigned char slotRead(unsigned i, struct SetRec* r)
{
	unsigned k;
	uint16_t* d = (uint16_t*)r;
	for(k = 0; k < SET_REC_HW; k++) d[k] = slotPtr(i)[k];
	return (r->magic == SET_MAGIC) && (r->version == SET_VERSION) && (r->crc == crc16(d, SET_REC_HW-1));
}

//Номер последней корректной записи (-1, если нет) и первого свободного слота (SET_REC_COUNT, если
//журнал заполнен). Недописанные слоты пропускаются: они не стерты, но и не корректны.
static int findCurrent(unsigned* nextFree)
{
	int cur = -1;
	unsigned i;
	struct SetRec r;
	*nextFree = SET_REC_COUNT;
	for(i = 0; i < SET_REC_COUNT; i++)
	{
		if(slotErased(i)){ *nextFree = i; break; }
		if(slotRead(i,&r)) cur = (int)i;
	}
	return cur;
}

unsigned char settingsFlashRead(uint16_t* v)
{
	struct SetRec r;
	unsigned next;
	int cur = findCurrent(&next);
	if(cur < 0) return 0;
	slotRead((unsigned)cur,&r);
	memcpy(v, r.v, sizeof(r.v));
	return 1;
}

unsigned char settingsFlashWrite(const uint16_t* v)
{
	struct SetRec r, chk;
	unsigned next, i;
	unsigned char ok = 1;
	findCurrent(&next);
	r.magic = SET_MAGIC;
	r.version = SET_VERSION;
	memcpy(r.v, v, sizeof(r.v));
	r.crc = crc16((const uint16_t*)&r, SET_REC_HW-1);
	{
		IRQ_SAVE();
		offInds();//Лампы гасим: на время записи ядро стоит и мультиплексирование не работает
		hwUnlock();
		if(next >= SET_REC_COUNT){ ok = hwErase(); next = 0; }//Журнал заполнен - начинаем заново
		for(i = 0; ok && (i < SET_REC_HW); i++)
			ok = hwProgram(SET_BASE + next*SET_REC_SIZE + 2u*i, ((const uint16_t*)&r)[i]);
		hwLock();
		IRQ_RESTORE();
	}
	//Проверка чтением
	return ok && slotRead(next,&chk) && (memcmp(&chk,&r,sizeof(r)) == 0);
}

/* ------------------------------ политика сохранения ------------------------------ */
static uint16_t saved[CONF_SAVED_COUNT];//Что записано (или применено при старте)
static uint16_t last[CONF_SAVED_COUNT];//Последнее замеченное состояние
static unsigned int lastCheck = 0;
static unsigned int changeTm = 0;//Когда настройки менялись в последний раз
static unsigned char retries = 0;

#ifdef SETTINGS_TEST
void settingsTestReset(void)
{
	memset(saved,0,sizeof(saved)); memset(last,0,sizeof(last));
	lastCheck = 0; changeTm = 0; retries = 0;
	settingsSaveCount = 0; settingsErrCount = 0;
}
#endif

void settingsLoad(void)
{
	uint16_t v[CONF_SAVED_COUNT];
	if(settingsFlashRead(v)) confRestore(v);//Недопустимая запись игнорируется - остаются заводские
	confSnapshot(saved);
	memcpy(last, saved, sizeof(last));
	retries = 0;
	lastCheck = sysCounter;
	changeTm = sysCounter;
}

//Можно ли сейчас остановить ядро на время записи
static unsigned char safeToWrite(void)
{
	return (flgWSTransfer == WS_TRANSFER_READY) && (i2c.flg == I2C_FREE) && !i2c.rcvRdy;
}

void settingsPoll(void)
{
	uint16_t cur[CONF_SAVED_COUNT];
	if((unsigned int)(sysCounter - lastCheck) < SET_CHECK_MS) return;
	lastCheck = sysCounter;

	confSnapshot(cur);
	if(memcmp(cur, last, sizeof(cur)) != 0)
	{
		memcpy(last, cur, sizeof(last));
		changeTm = sysCounter;//Любое изменение начинает отсчет заново
		retries = 0;
	}
	if(memcmp(cur, saved, sizeof(cur)) == 0) return;//Нечего сохранять
	if(retries >= SET_MAX_RETRY) return;//Сдаемся до следующего изменения
	if(flagConfMode){ changeTm = sysCounter; return; }//В меню не пишем: отсчет пойдет после выхода
	if((unsigned int)(sysCounter - changeTm) < SETTINGS_SAVE_DELAY_MS) return;
	if(!safeToWrite()) return;

	feedIWDG();
	if(settingsFlashWrite(cur))
	{
		memcpy(saved, cur, sizeof(saved));
		settingsSaveCount++;
		retries = 0;
	}
	else
	{
		settingsErrCount++;
		retries++;
		changeTm = sysCounter;//Повтор через SETTINGS_SAVE_DELAY_MS
	}
	feedIWDG();
}
