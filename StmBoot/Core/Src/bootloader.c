#include "bootloader.h"

Boot_state_t state= NOT_OPERATION;

uint8_t Crc8(uint8_t *pcBlock, uint8_t len)
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

void jumpToMainApp(void)
{
	uint32_t app_jump_address;
	typedef void(*pFunction)(void);
	pFunction Jump_To_Application;
	//Сброс периферии
	RCC->APB1RSTR = 0xFFFFFFFF;
	RCC->APB1RSTR = 0x0; 
	RCC->APB2RSTR = 0xFFFFFFFF;
	RCC->APB2RSTR = 0x0; 
	RCC->APB1ENR = 0x0;
	RCC->APB2ENR = 0x0;
	RCC->AHBENR = 0x0;
	__disable_irq();
	app_jump_address = *( uint32_t*) (PROGRAMM_START_ADR + 4);//Адрес перехода к основной программе 
	Jump_To_Application = (pFunction)app_jump_address;
	__set_MSP(*(__IO uint32_t*) PROGRAMM_START_ADR);//Перенос стека
    //SCB->VTOR=PROGRAMM_START_ADR;
	Jump_To_Application();//Переход к основной программе
}

uint8_t toApplMode()
{
	uint32_t temp;
	if(flashRead(PROGRAMM_START_ADR)!=0xFFFFFFFF)//Проверка наличия программы
	{
		//Стирание boot ключа из flash
		flashUnlock();
		flashPageErase(BOOT_KEY_ADR);
		flashLock();
		return 1;
	}
    return 0;
}

void bootProcess(void)
{
    can_recive_message_t* reciveData;
    static uint32_t validAddr = PROGRAMM_START_ADR;//Текущий адрез загружаемого flash
    static uint64_t mainAppLen;//Размер основной программы
    static uint64_t recivedLen=0;//Кол-во отправленных данных
    uint32_t i,tempData;
    uint8_t shift,crc;
    reciveData=canRead();
    if(reciveData!=0)
    {
        if (reciveData->messageId==INDIVIDUAL_ID1+20)
        {
            if(!reciveData->data[0])
            {
                validAddr = PROGRAMM_START_ADR;
                state=NOT_OPERATION;
                return;
            }
        }
        switch(state)
        {
            case NOT_OPERATION:
                if(reciveData->messageId==INDIVIDUAL_ID1)
                {
                    if(reciveData->data[0]==0xEE)
                    {
                        canWrite(BOOT_TRUE,1,INDIVIDUAL_ID1);
                    }
                    else
                    {
                        memcpy(&tempData,reciveData->data,sizeof(uint32_t));
                        if(tempData==BOOT_KEY)
                        {
                            state=PREPARE_UPLOAD;
                            canWrite(BOOT_TRUE,1,INDIVIDUAL_ID1);
                        }
                        else
                        {
                            canWrite(BOOT_FALSE,1,INDIVIDUAL_ID1);
                        }
                    }
                }
                break;
            case PREPARE_UPLOAD:
                if(reciveData->messageId==INDIVIDUAL_ID1)
                {
                    memcpy(&mainAppLen,reciveData->data,sizeof(uint32_t));//Извлекаем размер прошивки 
                    for(i=0;i<(mainAppLen/1024+1);i++)//Стираем flash память
                    {
                        flashUnlock();
                        flashPageErase(validAddr+i*1024);
                        flashLock();
                    }
                    canWrite(reciveData->data,reciveData->dataLen,INDIVIDUAL_ID1);
                    state=UPLOAD;
                }
                break;
            case UPLOAD:
                    if(reciveData->messageId==INDIVIDUAL_ID1+10)
                    {
                        tempData=0;
                        shift=0;
                        crc=Crc8(reciveData->data,reciveData->dataLen);
                        if(reciveData->dataLen>=4)
                        {
                            tempData = (reciveData->data[3]<<24)|(reciveData->data[2]<<16)|(reciveData->data[1]<<8)|reciveData->data[0];
                            flashUnlock();
                            flashWriteData(validAddr,tempData);///Запись прошивки в flash память
                            flashLock();
                            validAddr+=4;
                            shift=4;
                        }
                        tempData=0;
                        for(i=shift;i<reciveData->dataLen;i++)
                        {
                            tempData+=reciveData->data[i]<<((i-shift)*8);
                        }
                        flashUnlock();
                        flashWriteData(validAddr,tempData);///Запись прошивки в flash память
                        flashLock();
                        validAddr+=reciveData->dataLen-shift;
                        recivedLen+=reciveData->dataLen;
                        canWrite(&crc,1,INDIVIDUAL_ID1+10);
                        if(recivedLen==mainAppLen)
                        {
                            if(toApplMode)
                            {
                                delay_ms(500);
                                canWrite(BOOT_TRUE,1,INDIVIDUAL_ID1+10);
                                delay_ms(500);
                                toApplMode();
                                jumpToMainApp();
                            }
                            else
                            {
                                validAddr = PROGRAMM_START_ADR;
                                state=NOT_OPERATION;
                            }
                        }
                    }
                break;
        }
        
    }
}