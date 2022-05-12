#include "bootloader.h"

Bootloader::Bootloader(BYTE canPort,QString firmwarePath,uint16_t ID)
{
    workCanId=ID;
    bootFirmware = new Firmware(firmwarePath);
    if(bootFirmware->open())
    {
        printLog<qint64>("Firmware size: ", bootFirmware->getFirmwareSize());
        can = new CanBus(canPort);
        if(can->getErrorState()==0)
        {
            printLog("Can open OK");
            error=false;
        }
        else
        {
            printLog("Can error");
            error=true;
        }
    }
    else
    {
        printLog("Firmware open error");
        error=true;
    }

}

Bootloader::~Bootloader(void)
{
    delete(bootFirmware);
    delete(can);
}

void Bootloader::bootProcess(void)
{
    if(!error)
    {
        can->clear();
        resetBoot();
        canFrame_t frame;
        can->sendMsg(reinterpret_cast<char *>(&bootKey),sizeof(uint32_t),workCanId);
        bool waitFlag = can->waitReciveFrom(&frame,workCanId,timeout_ms);
        if(waitFlag)
        {
            if(!frame.data[0])
            {
                printLog("Key error");
                return;
            }
        }
        else
        {
            printLog("Timeout error");
            resetBoot();
            return;
        }
        printLog("Key OK");
        can->clear();
        uint32_t size = static_cast<uint32_t>(bootFirmware->getFirmwareSize());
        can->sendMsg(reinterpret_cast<char *>(&size),sizeof(size),workCanId);
        waitFlag = can->waitReciveFrom(&frame,workCanId,timeout_ms);
        if(waitFlag)
        {
            if(!memcmp(&size,frame.data,sizeof(uint32_t)))
            {
                printLog("Size transmitt OK");
                sendFirmware();
            }
            else
            {
                printLog("Communicate error");
                resetBoot();
            }
        }
        else
        {
            printLog("Timeout error");
        }
    }
}

void Bootloader::sendFirmware(void)
{
    can->clear();
    printLog("Send firmware start");
    unsigned int readSize;
    unsigned int factSize=0;
    char data[8];
    canFrame_t frame;
    do
    {
        readSize=bootFirmware->readFirmware(8,data);
        can->sendMsg(reinterpret_cast<char*>(&data),readSize,workCanId+10);
        uint8_t crc=Crc8(reinterpret_cast<uint8_t *>(data),readSize);
        bool waitFlag = can->waitReciveFrom(&frame,workCanId+10,timeout_ms);
        if(!(waitFlag) || (frame.data[0]!=static_cast<char>(crc)))
        {
            printLog("Firmware upload fail");
            resetBoot();
            return;
        }
        factSize+=readSize;
        printLog<unsigned int>("Load size: ",factSize);
    } while(readSize==8);
    printLog("Firmware upload complite");
    printLog<unsigned int>("Upload size: ",factSize);
    bool waitFlag = can->waitReciveFrom(&frame,workCanId+10,timeout_ms);
    if(waitFlag && frame.data[0])
    {
        printLog("COMPLITE");
    }
    else
    {
        printLog("ERROR");
        resetBoot();
    }
}

void Bootloader::printLog(std::string printData)
{
    std::cout << printData << std::endl;
}

template<class LogType> void Bootloader::printLog(std::string printData,LogType data)
{
    std::cout << printData << data << std::endl;
}

uint8_t Bootloader::Crc8(uint8_t *pcBlock, uint8_t len)
{
    uint8_t crc = 0xFF;
    uint8_t i;
    while (len--)
    {
        crc ^= *pcBlock++;

        for (i = 0; i < 8; i++)
            crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }
    return crc;
}

void Bootloader::resetBoot(void)
{
    char data[8];
    data[0]=0;
    can->sendMsg(reinterpret_cast<char*>(&data),1,workCanId+20);
}

bool Bootloader::switchToBootMode(void)
{
    if(!error)
    {
        char data[8];
        data[0]=0xEE;
        can->clear();
        can->sendMsg(reinterpret_cast<char*>(&data),1,workCanId);
        canFrame_t frame;
        bool waitFlag = can->waitReciveFrom(&frame,workCanId,100);
        if(waitFlag && frame.data[0])
        {
            printLog("BOOT MODE:");
            return true;
        }
    }
    return false;
}
