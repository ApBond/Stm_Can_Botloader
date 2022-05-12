#ifndef __CAN_H
#define __CAN_H

#include "main.h"
#include <string.h>

//Can pins setting
#define CAN_PORT GPIOA
#define CAN_TX_PIN 12
#define CAN_RX_PIN 11
#define CAN_TX_AF 9
#define CAN_RX_AF 9

#define CAN_MAX_INIT_TIME_MS 1000
#define CAN_DATA_LENGTH 8
#define CAN_RECIVE_BUFFER_SIZE 50

typedef enum
{
    CAN_READY,
    CAN_ERROR
}CAN_STATE_t;

typedef enum
{
    ID,
    MASK
}Can_filter_mode_t;

typedef struct 
{
    uint8_t data[8];
    uint8_t dataLen;
    uint16_t messageId;
}can_recive_message_t;


static void gpioInit(void);
CAN_STATE_t canInit(uint32_t baudRateValue);
void canConfigReciveFIFO(uint8_t FIFONumber,uint8_t filterNumber,Can_filter_mode_t mode,uint16_t* id);
void canWrite(uint8_t *pData, size_t dataLength,uint16_t messageId);
static void canMessageProcess(uint8_t mailboxNumber);
can_recive_message_t* canRead(void);


#endif  