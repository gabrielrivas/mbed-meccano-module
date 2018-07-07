#include "MeccanoPortController.h"
#include "MeccanoSmartModule.h"
#include <map>
#include "mbed.h"

//Mask that adds the start and stop bits 
//For sending bytes
#define START_STOP_BITS 0x0600
#define BIT_TIME 420

uint8_t MeccanoPortController::startByte = 0xFF;

MeccanoPortController::MeccanoPortController(Serial* a_moduleDataOut, InterruptIn* a_moduleDataIn, DigitalOut* a_portEnable)
{
  moduleDataOut = a_moduleDataOut;
  moduleDataIn = a_moduleDataIn;    
  portEnable = a_portEnable;

  *portEnable = 1;

  receiveState = START_BIT;
   
  controllerState = MeccanoPortController::MODULE_DISCOVERY; 
  currentModule = 0;

  checkSum = 0;

  lowTime = 0;
  receiverShiftCounter = 0;
  receiverData = 0;

  receiver.start(); 
  moduleDataIn->fall(callback(this, &MeccanoPortController::receiveDataFall));
  moduleDataIn->rise(callback(this, &MeccanoPortController::receiveDataRise));    

  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(0, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(1, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(2, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(3, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );

  engineTicker.attach(callback(this, &MeccanoPortController::ioEngine), 0.1);
}

uint8_t MeccanoPortController::calculateCheckSum(uint8_t Data1, uint8_t Data2, uint8_t Data3, uint8_t Data4, uint8_t moduleNum){
  uint16_t CS = Data1 + Data2 + Data3 + Data4;  // ignore overflow
  CS = CS + (CS >> 8);                  // right shift 8 places
  CS = CS + (CS << 4);                  // left shift 4 places
  CS = CS & 0xF0;                     // mask off top nibble
  CS = CS | moduleNum;

  return (CS & 0xFF);
}

void MeccanoPortController::receiveDataFall()
{
  //t0 = 0
  receiver.reset();
}

void MeccanoPortController::receiveDataRise()
{
  uint8_t bitValue = 0;

  //get t1
  lowTime = receiver.read_us(); 

  switch(receiveState)
  {
     case START_BIT:
       if (lowTime > 1900)
       {
         receiverShiftCounter = 0;
         receiverData = 0;
         receiveState = DATA_BITS; 
       }

       break;
     case DATA_BITS:       
       if (lowTime < 700)
       {
         bitValue = 1; 
       }     
       receiverData |= ((bitValue & 0x01) << receiverShiftCounter);

       if (receiverShiftCounter < 7)
       {
         ++receiverShiftCounter;
       }
       else 
       {
         std::map<int, MeccanoSmartModule>::iterator it = m_smartModulesMap.find(currentModule); 
         (it->second).m_inputData = receiverData; 
           
         receiveState = START_BIT;

         moduleDataIn->disable_irq(); 
         *portEnable = 1;

        /*if (currentModule < 4)
          ++currentModule;
        else
          currentModule = 0;*/   

       }   
       break;
  } 
}

void MeccanoPortController::setPresence(int servoSlot, bool presence)
{
    //Put guards in here
    std::map<int, MeccanoSmartModule>::iterator it = m_smartModulesMap.find(servoSlot); 
    (it->second).m_isPresent = presence; 
}

void MeccanoPortController::setCommand(int servoSlot, uint8_t command)
{
  std::map<int, MeccanoSmartModule>::iterator it = m_smartModulesMap.find(servoSlot); 
  (it->second).m_outputData = command; 

  *portEnable = 1;
  
  receiverShiftCounter = 0;
  receiveState = START_BIT;

  currentModule = servoSlot;

  checkSum = calculateCheckSum(m_smartModulesMap.at(0).m_outputData,
                          m_smartModulesMap.at(1).m_outputData,
                          m_smartModulesMap.at(2).m_outputData,
                          m_smartModulesMap.at(3).m_outputData,
                          currentModule);

  //Send start byte
  moduleDataOut->putc(startByte);

  //Send channel data
  for (int modCount = 0; modCount < 4; ++modCount)
    moduleDataOut->putc(m_smartModulesMap.at(modCount).m_outputData);        	          	  	
  
  //Send checksum
  moduleDataOut->putc(checkSum);

  //Thread::wait(8);
  
  //Enable receiver  
  wait(0.010);
  *portEnable = 0;
  moduleDataIn->enable_irq(); 
}


void MeccanoPortController::ioEngine()
{
    switch(controllerState)
    {
     case MODULE_DISCOVERY:
       currentModule = 0;
       setCommand(currentModule, MeccanoPortController::ID_NOT_ASSIGNED);
       controllerState = MeccanoPortController::GET_DISCOVERY_RESPONSE;

        break;
      case GET_DISCOVERY_RESPONSE:
//setInputData(currentModule, receiverData);
        if (m_smartModulesMap.at(currentModule).m_inputData == 254)
        {
            setPresence(currentModule, true);
            controllerState = MeccanoPortController::MODULE_TYPE_DISCOVERY; 
        }     
        else{
          setPresence(currentModule, false);
          controllerState = MeccanoPortController::MODULE_DISCOVERY;
        }     
/*
        if (currentModule < 3)
        {
          currentModule++;
          controllerState = MeccanoPortController::MODULE_DISCOVERY;
        }
        else
        {
          currentModule = 0;
          controllerState = MeccanoPortController::MODULE_TYPE_DISCOVERY;
          //controllerState = MeccanoPortController::MODULE_IDLE;
        }  
*/             
        break;        
      case MODULE_TYPE_DISCOVERY:
        setCommand(currentModule, MeccanoPortController::REPORT_TYPE);
        controllerState = GET_TYPE_RESPONSE;

        break;
       case GET_TYPE_RESPONSE: 
         if (m_smartModulesMap.at(currentModule).m_inputData == 1)
         {
             setPresence(currentModule, true);
            controllerState =MeccanoPortController::MODULE_IDLE; 
         }     
        else{
          controllerState = MeccanoPortController::MODULE_TYPE_DISCOVERY;
        } 
               
        break;       
      case MODULE_IDLE:
       
      
       //if (m_smartModulesMap.at(currentModule).m_isPresent)   
       //  setInputData(currentModule, receiverData);

      default:
        break;  
    }
   
}