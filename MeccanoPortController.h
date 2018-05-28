#ifndef _MECCANO_PORT_CONTROLLER_H_
#define _MECCANO_PORT_CONTROLLER_H_

#include "MeccanoSmartModule.h"
#include <map>
#include "mbed.h"

class MeccanoPortController 
{
  public:
    // ***************** Reserved commands
    enum RESERVED_COMMANDS
    {
      HEADER_BYTE = 0xFF,     // header byte (never used for anything else)
      ID_NOT_ASSIGNED = 0xFE, // ID not assigned (means no Smart Module is at this position)
      ERASE_ID = 0xFD,        // Erase ID (resets the Smart Module)
      REPORT_TYPE = 0xFC,      // Report Smart Module type
      MODULE_PRESENT = 0xF9
    };

    // ***************** Port controller
    enum PORT_CONTROLLER 
    {
      MODULE_DISCOVERY = 0,
      GET_DISCOVERY_RESPONSE,
      MODULE_TYPE_DISCOVERY,
      GET_TYPE_RESPONSE,
      MODULE_IDLE,
      INVALID
    };

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
        static void threadStarter(const void* arg);   
        void setCommand(int servoSlot, RESERVED_COMMANDS command);
        std::map<int, MeccanoSmartModule>& getModulesMap()
        {
          return m_smartModulesMap;
        }

        uint8_t getReceivedData() {
            return receiverData;
        }

    private:
      void ioControllerEngine(); 
      void setPresence(int servoSlot, bool presence);
      void setInputData(int servoSlot, uint8_t data);
      
    private:
        DigitalInOut* moduleDataOut;
        InterruptIn* moduleDataIn;

        PORT_CONTROLLER controllerState;
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

        Thread*  m_inputThread;
};

#endif