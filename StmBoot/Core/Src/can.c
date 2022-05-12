#include "can.h"

static can_recive_message_t messageBuff[CAN_RECIVE_BUFFER_SIZE];
static uint16_t bufferPointer=0;
static uint16_t bufferReadPointer=0;
static uint16_t bufferMessageCount=0;

void CAN_SCE_IRQHandler(void)
{
    CAN->MSR|= CAN_MSR_ERRI;
    //TODO
}

void CAN_RX0_IRQHandler(void)
{
    uint8_t messageCount;
    uint8_t i;
    messageCount=CAN->RF0R & CAN_RF0R_FMP0_Msk;
    for(i=0;i<messageCount;i++)
    {
        canMessageProcess(0);
        CAN->RF0R|=CAN_RF0R_RFOM0;
    }
}

void CAN_RX1_IRQHandler(void)
{
    uint8_t messageCount;
    uint8_t i;
    messageCount=CAN->RF1R & CAN_RF1R_FMP1_Msk;
    for(i=0;i<messageCount;i++)
    {
        canMessageProcess(1);
        CAN->RF1R|=CAN_RF1R_RFOM1;
    } 
}

static void canMessageProcess(uint8_t mailboxNumber)
{
    uint8_t i;
    uint8_t dataLength = CAN->sFIFOMailBox[mailboxNumber].RDTR & CAN_RDT0R_DLC_Msk;
    messageBuff[bufferPointer].dataLen=dataLength;
    messageBuff[bufferPointer].messageId=(CAN->sFIFOMailBox[mailboxNumber].RIR & CAN_RI0R_STID_Msk)>>CAN_RI0R_STID_Pos;
    for(i=0;i<8;i++)
    {
        messageBuff[bufferPointer].data[i]=0;
    }
    for(i=0;i<dataLength;i++)
    {
        if(i<4)
            messageBuff[bufferPointer].data[i]=(CAN->sFIFOMailBox[mailboxNumber].RDLR & 0xFF<<(i*8))>>(i*8);
        else
            messageBuff[bufferPointer].data[i]=(CAN->sFIFOMailBox[mailboxNumber].RDHR & 0xFF<<((i-4)*8))>>((i-4)*8);
    }
    bufferPointer++;
    if(bufferMessageCount<CAN_RECIVE_BUFFER_SIZE) bufferMessageCount++;
    if(bufferPointer==CAN_RECIVE_BUFFER_SIZE) bufferPointer=0;
    //Если используется RTOS добавить установку семафора на чтение
}


static void gpioInit(void)
{
    CAN_PORT->MODER|=0b10<<(2*CAN_TX_PIN) | 0b10<<(2*CAN_RX_PIN);
    CAN_PORT->OSPEEDR|=0b11<<(2*CAN_TX_PIN) | 0b11<<(2*CAN_RX_PIN);
    CAN_PORT->AFR[CAN_TX_PIN/8]|=CAN_TX_AF<<(4*(CAN_TX_PIN-8*(CAN_TX_PIN/8)));
    CAN_PORT->AFR[CAN_RX_PIN/8]|=CAN_RX_AF<<(4*(CAN_RX_PIN-8*(CAN_RX_PIN/8)));

}

CAN_STATE_t canInit(uint32_t baudRateValue)
{
    gpioInit();
    uint16_t i;
    RCC->APB1ENR |= RCC_APB1ENR_CANEN;
    CAN->MCR|=CAN_MCR_INRQ;//Перевод can в режим инициализации
    CAN->MCR|=CAN_MCR_NART;//Отключить ретрансляцию
    CAN->MCR|=CAN_MCR_AWUM;//Автоматический выход из спящего режима
    CAN->BTR=baudRateValue;//Настройка скорости обмена
    //Конфигурация прерываний
    CAN->IER|=CAN_IER_ERRIE | CAN_IER_EWGIE;//Преревание по ошибке на шине
    CAN->IER|=CAN_IER_FMPIE0 | CAN_IER_FMPIE1;//Перерывание при получении сообщения в FIFO0 и FIFO1
    NVIC_EnableIRQ(CAN_SCE_IRQn);
    NVIC_EnableIRQ(CAN_RX0_IRQn);
    NVIC_EnableIRQ(CAN_RX1_IRQn);
    __enable_irq();
    CAN->MCR &= ~CAN_MCR_INRQ;//Перевод CAN в нормальный режим
    while(CAN->MSR & CAN_MSR_INAK) 
    {
        if(i>CAN_MAX_INIT_TIME_MS) return CAN_ERROR;
        //delay_ms(1);
        i++;
    }
    CAN->MCR&=~CAN_MCR_SLEEP;
    return CAN_READY;
}

void canConfigReciveFIFO(uint8_t FIFONumber,uint8_t filterNumber,Can_filter_mode_t mode,uint16_t* id)
{
    CAN->FMR=1;//Ввести фильтр в режим инициализации
    CAN->sFilterRegister[filterNumber].FR1=id[0]<<5 | id[1]<<21;
    CAN->sFilterRegister[filterNumber].FR2=id[2]<<5 | id[3]<<21;
    if(mode==ID)
        CAN->FM1R|=1<<filterNumber;
    CAN->FA1R|=1<<filterNumber;
    if(FIFONumber==1)
        CAN->FFA1R|=1<<filterNumber;
    CAN->FMR=0;
}

void canWrite(uint8_t *pData, size_t dataLength,uint16_t messageId)
{
    uint8_t freeMailbox;
    uint8_t i;
    char temp;
    temp=dataLength-4;
    freeMailbox=(CAN->TSR&CAN_TSR_CODE)>>CAN_TSR_CODE_Pos;//Получиние свободного mailbox
    CAN->sTxMailBox[freeMailbox].TDLR = 0;
	CAN->sTxMailBox[freeMailbox].TDHR = 0;
    CAN->sTxMailBox[freeMailbox].TDTR &= ~CAN_TDT0R_DLC;
    CAN->sTxMailBox[freeMailbox].TDTR |= (dataLength << CAN_TDT0R_DLC_Pos);//Кол-во информации в одном кадре
    CAN->sTxMailBox[freeMailbox].TIR=0;
    CAN->sTxMailBox[freeMailbox].TIR |= messageId<<CAN_TI0R_STID_Pos;//Идентификатор сообщения
    for(i=0;i<dataLength;i++)
    {
        if(i<4)
            CAN->sTxMailBox[freeMailbox].TDLR|=pData[i]<<(i*8);
        else
            CAN->sTxMailBox[freeMailbox].TDHR|=pData[i]<<(i-4)*8;    
    }
    CAN->sTxMailBox[freeMailbox].TIR |= CAN_TI0R_TXRQ;//Передача сообщения
}

can_recive_message_t* canRead(void)
{
    can_recive_message_t* returnData;
    if(bufferMessageCount!=0)
    {
        returnData=&messageBuff[bufferReadPointer];
        bufferReadPointer++;
        if(bufferReadPointer==CAN_RECIVE_BUFFER_SIZE) bufferReadPointer=0;
        if(bufferMessageCount>0) bufferMessageCount--;
        return returnData;
    }
    return 0;
}
