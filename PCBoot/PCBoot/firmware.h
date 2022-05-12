#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <iostream>
#include <string>
#include <fstream>
#include <QtCore>

class Firmware
{
private:
    QString filePath;
    QFile * firmware;
    qint64 firmwareSize;
public:
    Firmware(QString path):filePath(path){};
    ~Firmware(void);
    bool open();
    qint64 getFirmwareSize();
    unsigned int readFirmware(unsigned int size,char * data);

};

#endif // FIRMWARE_H
