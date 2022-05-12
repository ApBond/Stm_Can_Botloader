#include <QCoreApplication>
#include <iostream>
#include <string>
#include <fstream>
#include <windows.h>
#include "canbus.h"
#include "firmware.h"
#include "bootloader.h"

int main(int argc, char *argv[])
{
    if(argc==4)
    {
        QString com = argv[1];
        QString id = argv[2];
        QString path = argv[3];
        uint16_t workCanId=id.toInt();
        Bootloader boot(com.toInt(),path,workCanId);
        int count =0;
        while(!boot.switchToBootMode())
        {
            count++;
            if(count==5)
            {
                std::cout << "Device is not boot mode" << std::endl;
                return 0;
            }
        }
        boot.bootProcess();
    }
    else
    {
        std::cout << "Not enogth arguments" << std::endl;
    }
    return 0;
}
