#include "firmware.h"

bool Firmware::open()
{
   firmware = new QFile (filePath);
    if(firmware->exists())
    {
        if(firmware->open(QIODevice::ReadOnly))
        {
            firmwareSize = firmware->size();
            return true;
        }
    }
    return false;
}

qint64 Firmware::getFirmwareSize()
{
    return firmwareSize;
}

unsigned int Firmware::readFirmware(unsigned int size,char * data)
{
    unsigned int blockSize = firmware->read(data,size);
    return blockSize;
}

Firmware::~Firmware(void)
{
    firmware->close();
    delete(firmware);
}
