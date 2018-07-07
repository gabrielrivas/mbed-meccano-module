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
      
    // ****************** Receiver FSM
    enum RECEIVER_STATES
    {
      START_BIT = 0,
      DATA_BITS   
    };

    enum PORT_CONTROLLER 
    {
      MODULE_DISCOVERY = 0,
      GET_DISCOVERY_RESPONSE,
      MODULE_TYPE_DISCOVERY,
      GET_TYPE_RESPONSE,
      MODULE_IDLE,
      INVALID
    };

    public:
      MeccanoPortController(Serial* a_moduleDataOut, InterruptIn* a_moduleDataIn, DigitalOut* a_portEnable);
      ~MeccanoPortController(){}

    public:
        uint8_t calculateCheckSum(uint8_t Data1, uint8_t Data2, uint8_t Data3, uint8_t Data4, uint8_t moduleNum);
        uint16_t frameByte(uint8_t data);
        void receiveDataFall();
        void receiveDataRise();
        void setCommand(int servoSlot, uint8_t command);
        void ioEngine();
        void setPresence(int servoSlot, bool presence);

        std::map<int, MeccanoSmartModule>& getModulesMap()
        {
          return m_smartModulesMap;
        }

        uint8_t getReceivedData() {
            return receiverData;
        }

        PORT_CONTROLLER getState() {
            return controllerState;
        }
    private:
        Serial* moduleDataOut;
        InterruptIn* moduleDataIn;
        DigitalOut* portEnable;

        PORT_CONTROLLER controllerState;
        RECEIVER_STATES receiveState;

        std::map<int, MeccanoSmartModule> m_smartModulesMap;
        uint8_t checkSum;

        Ticker engineTicker;
        Timer receiver;
        int lowTime;
        int receiverShiftCounter;
        uint8_t receiverData;
        static uint8_t startByte;
        int currentModule;
};

#endif