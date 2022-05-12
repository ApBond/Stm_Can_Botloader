#include "main.h"


int main(void)
{
	uint16_t filter[]={INDIVIDUAL_ID1,INDIVIDUAL_ID1+10,INDIVIDUAL_ID1+20,INDIVIDUAL_ID1+30};
	uint32_t bootKey;
	RccClockInit();
	delayInit();
	bootKey = flashRead(BOOT_KEY_ADR);//Считыввание boot ключа
	if(bootKey!=BOOT_KEY && flashRead(PROGRAMM_START_ADR)!=0xFFFFFFFF)//Проверка boot ключа и наличия основное программы
	{
		jumpToMainApp();
	}
	canInit(0x011c0008);
	canConfigReciveFIFO(1,0,ID,&filter);
	uint32_t i=0;
    while(1)
    {
		bootProcess();
	}
}
