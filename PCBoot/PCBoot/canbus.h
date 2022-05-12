#ifndef CANBUS_H
#define CANBUS_H
#include <Windows.h>
#include "VCI_CAN.h"
#include <unistd.h>

struct canFrame_t
{
    int ID;
    int dataLen;
    char data[8];
};

class CanBus
{
private:
BYTE devPort;
BYTE devType;
DWORD CAN1_Baud;
DWORD CAN2_Baud;
char errorState;
public:
    CanBus(BYTE port=0,BYTE dev=I7565H2,DWORD baud1=CANBaud_250K,DWORD baud2=CANBaud_250K) :
        devPort(port), devType(dev), CAN1_Baud(baud1), CAN2_Baud(baud2)
    {
        errorState=VCI_OpenCAN_NoStruct(devPort,devType,CAN1_Baud,CAN2_Baud);
        errorState=VCI_Set_CANFID_AllPass(CAN1);
    }

    ~CanBus()
    {
        VCI_CloseCAN(devPort);
    }

    char getErrorState(void){return errorState;}

    void sendMsg(char *data,char dataLen,int ID)
    {
        _VCI_CAN_MSG sendMsg;
        sendMsg.DLC=dataLen;
        sendMsg.RTR=0;
        sendMsg.ID=ID;
        sendMsg.Mode=0;
        memcpy(sendMsg.Data,data,dataLen);
        VCI_SendCANMsg(CAN1,&sendMsg);
    }

    bool readMsg(canFrame_t * frame)
    {
        DWORD msgCount;
        _VCI_CAN_MSG reciveMsg;
        VCI_Get_RxMsgCnt(CAN1,&msgCount);
        if(msgCount>0)
        {
            VCI_RecvCANMsg(CAN1,&reciveMsg);
            frame->dataLen=reciveMsg.DLC;
            frame->ID=reciveMsg.ID;
            memcpy(frame->data,reciveMsg.Data,frame->dataLen);
            return true;
        }
        return false;
    }

    bool waitReciveFrom(canFrame_t * frame,int ID,int timeout_ms)
    {
        int counter=0;
        while(counter!=timeout_ms)
        {
            if(readMsg(frame))
            {
                if(frame->ID==ID)
                {
                    return true;
                }
            }
            Sleep(1);
            counter++;
        }
        return false;
    }

    void clear()
    {
        VCI_Clr_RxMsgBuf(1);
    }
};

#endif // CANBUS_H
