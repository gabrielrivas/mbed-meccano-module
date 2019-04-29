#ifndef _MECCANO_PORT_RECEIVER_H_
#define _MECCANO_PORT_RECEIVER_H_

#include "mbed.h"

class MeccanoPortReceiver 
{
    public:
    // ****************** Receiver FSM
    enum RECEIVER_STATES
    {
      START_BIT = 0,
      DATA_BITS   
    };

    public:
      void enableReceiver();
      void receiveDataFall();
      void receiveDataRise();
      void disableReceiver();   
      void resetFSM();  
        uint8_t getReceivedData();

        bool isDataReady();
    public:
      MeccanoPortReceiver(InterruptIn* a_moduleDataIn);
      ~MeccanoPortReceiver(){}

    private:
        RECEIVER_STATES receiveState;
        InterruptIn* moduleDataIn;
        Timer receiver;
        bool dataReady;
        int receiverShiftCounter;
        uint8_t receiverData;
};

#endif