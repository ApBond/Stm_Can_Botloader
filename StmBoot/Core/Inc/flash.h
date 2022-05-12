#ifndef __FLASH_H
#define __FLASH_H

#include "main.h"

#define FLASH_KEY_1 (uint32_t)0x45670123
#define FLASH_KEY_2 (uint32_t)0xCDEF89AB

void flashUnlock(void);
void flashLock(void);
void flashWriteData(uint32_t adr, uint32_t data);
void flashPageErase(uint32_t adr);
uint32_t flashRead(uint32_t address);

#endif