#ifndef _MECCANO_PORT_CONTROLLER_H_
#define _MECCANO_PORT_CONTROLLER_H_

#include "MeccanoSmartModule.h"
#include <map>
#include "mbed.h"

class MeccanoPortController 
{
    public:

    // ****************** Sender FSM
    enum SENDER_STATES
    {
      START_BYTE = 0,
      DATA_BYTES
    };

    // ****************** Receiver FSM

    enum RECEIVER_STATES
    {
      START_BIT = 0,
      DATA_BITS   
    };

    public:
      MeccanoPortController(DigitalInOut* a_moduleDataOut, InterruptIn* a_moduleDataIn);
      ~MeccanoPortController(){}

    public:
        uint8_t calculateCheckSum(uint8_t Data1, uint8_t Data2, uint8_t Data3, uint8_t Data4, uint8_t moduleNum);
        uint16_t frameByte(uint8_t data);
        void receiveDataFall();
        void receiveDataRise();
        void sendData();
        void setPosition(int servoSlot, uint8_t position);
        void enableSendData();
        
        std::map<int, MeccanoSmartModule>& getModulesMap()
        {
          return m_smartModulesMap;
        }

        uint8_t getReceivedData() {
            return receiverData;
        }

    private:
        DigitalInOut* moduleDataOut;
        InterruptIn* moduleDataIn;

        RECEIVER_STATES receiveState;
        SENDER_STATES sendState;

        int senderShiftCounter;
        int moduleCounter;
        uint16_t tmpData;
        std::map<int, MeccanoSmartModule> m_smartModulesMap;
        uint8_t checkSum;

        Ticker sender;
        Timer receiver;
        int lowTime;
        int receiverShiftCounter;
        uint8_t receiverData;
        static uint8_t startByte;
        int currentModule;
};

#endif