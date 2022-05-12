#ifndef __BOOT_H
#define __BOOT_H

#include "main.h"

#define PROGRAMM_START_ADR 0x08002400
#define BOOT_KEY 0x861D8B36
#define BOOT_KEY_ADR 0x0800FFF0

#define BOOT_TRUE 1
#define BOOT_FALSE 0

typedef enum
{
	NOT_OPERATION,//Ожидание команды от клиента
    PREPARE_UPLOAD,
	UPLOAD,//Загрузка программы
	COMPLETE,//Загрузка программы завершена, переход к основной программе
	TO_APPL//Переход к основное программе по команде клиента
}Boot_state_t;

void bootProcess(void);
void jumpToMainApp(void);
uint8_t Crc8(uint8_t *pcBlock, uint8_t len);
uint8_t toApplMode();

#endif