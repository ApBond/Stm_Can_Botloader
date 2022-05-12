#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <QCoreApplication>
#include <iostream>
#include <string>
#include <fstream>
#include <windows.h>
#include "canbus.h"
#include "firmware.h"
#include <unistd.h>


class Bootloader
{
private:
    CanBus * can;
    Firmware* bootFirmware;
    bool error;
    uint16_t workCanId;
    int timeout_ms=500;
    uint32_t bootKey = 0x861D8B36;

private:
    void printLog(std::string printData);
    template<class LogType> void printLog(std::string printData,LogType data);
    void sendFirmware(void);
    uint8_t Crc8(uint8_t *pcBlock, uint8_t len);
    void resetBoot(void);
public:
    Bootloader(BYTE canPort,QString firmwarePath,uint16_t ID);
    ~Bootloader(void);
    void bootProcess(void);
    void setWaitTime(int time){timeout_ms=time;};
    bool getError(void){return error;};
    bool switchToBootMode(void);

};

#endif // BOOTLOADER_H
