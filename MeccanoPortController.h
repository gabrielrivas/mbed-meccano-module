#ifndef _MECCANO_PORT_CONTROLLER_H_
#define _MECCANO_PORT_CONTROLLER_H_

#include "MeccanoSmartModule.h"
#include "MeccanoPortReceiver.h"
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
        
        uint8_t setCommand(int servoSlot, uint8_t command);
        void ioEngine();
        void setPresence(int servoSlot, bool presence);
        void setType(int servoSlot, MeccanoSmartModule::TYPE_t type);
        void setInputData(int servoSlot, uint8_t data);
        void setCurrentModule(int value);

        std::map<int, MeccanoSmartModule>& getModulesMap()
        {
          return m_smartModulesMap;
        }

        PORT_CONTROLLER getState() {
            return controllerState;
        }

        int getCurrentModule()
        {
          return currentModule;
        }
    private:
        Serial* moduleDataOut;
        MeccanoPortReceiver* portReceiver; 
        DigitalOut* portEnable;


        PORT_CONTROLLER controllerState;

        std::map<int, MeccanoSmartModule> m_smartModulesMap;
        uint8_t checkSum;

        RtosTimer* engineTicker;        

        int currentModule;
};

#endif