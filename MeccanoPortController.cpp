#include "MeccanoPortController.h"
#include "MeccanoSmartModule.h"
#include <map>
#include "mbed.h"

//Mask that adds the start and stop bits 
//For sending bytes
MeccanoPortController::MeccanoPortController(Serial* a_moduleDataOut, InterruptIn* a_moduleDataIn, DigitalOut* a_portEnable)
{
  moduleDataOut = a_moduleDataOut;    
  portEnable = a_portEnable;
  portReceiver = new MeccanoPortReceiver(a_moduleDataIn);

  *portEnable = 1;
   
  controllerState = MeccanoPortController::MODULE_DISCOVERY; 
  currentModule = 0;

  checkSum = 0;

  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(0, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(1, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(2, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(3, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
}

uint8_t MeccanoPortController::calculateCheckSum(uint8_t Data1, uint8_t Data2, uint8_t Data3, uint8_t Data4, uint8_t moduleNum){
  uint16_t CS = Data1 + Data2 + Data3 + Data4;  // ignore overflow
  CS = CS + (CS >> 8);                  // right shift 8 places
  CS = CS + (CS << 4);                  // left shift 4 places
  CS = CS & 0xF0;                     // mask off top nibble
  CS = CS | moduleNum;

  return (CS & 0xFF);
}

void MeccanoPortController::setPresence(int servoSlot, bool presence)
{
    //Put guards in here
    std::map<int, MeccanoSmartModule>::iterator it = m_smartModulesMap.find(servoSlot); 
    (it->second).m_isPresent = presence; 
}

void MeccanoPortController::setType(int servoSlot, MeccanoSmartModule::TYPE_t type)
{
    //Put guards in here
    std::map<int, MeccanoSmartModule>::iterator it = m_smartModulesMap.find(servoSlot); 
    (it->second).m_type = type; 
}

void MeccanoPortController::setCommand(int servoSlot, uint8_t command)
{
  std::map<int, MeccanoSmartModule>::iterator it = m_smartModulesMap.find(servoSlot); 
  (it->second).m_outputData = command; 
}

uint8_t MeccanoPortController::sendData(int returnModule)
{
  *portEnable = 1;
  portReceiver->resetFSM();
  
  if (returnModule < 4)
    currentModule = returnModule;
  else
    currentModule = 0;

  checkSum = calculateCheckSum(m_smartModulesMap.at(0).m_outputData,
                               m_smartModulesMap.at(1).m_outputData,
                               m_smartModulesMap.at(2).m_outputData,
                               m_smartModulesMap.at(3).m_outputData,
                               currentModule);

  //Send start byte
  moduleDataOut->putc(MeccanoPortController::HEADER_BYTE);

  //Send channel data for the 4 channels
  for (int modCount = 0; modCount < 4; ++modCount)
    moduleDataOut->putc(m_smartModulesMap.at(modCount).m_outputData);        	          	  	
  
  //Send checksum
  moduleDataOut->putc(checkSum);

  //Enable receiver  
  wait(0.010);
  *portEnable = 0;
  portReceiver->enableReceiver();

  wait(0.019);
  portReceiver->disableReceiver();
  *portEnable = 1;

  uint8_t receivedData = portReceiver->getReceivedData();

  if (portReceiver->isDataReady())
  {
    if (receivedData == ID_NOT_ASSIGNED)
      setCommand(currentModule, REPORT_TYPE);

    setInputData(currentModule, receivedData);
  }
  else
    setInputData(currentModule, 0);  
  
  return receivedData;
}

void MeccanoPortController::setInputData(int servoSlot, uint8_t data)
{
    //Put guards in here
    std::map<int, MeccanoSmartModule>::iterator it = m_smartModulesMap.find(servoSlot); 
    (it->second).m_inputData = data; 
}

void MeccanoPortController::setCurrentModule(int value)
{
  currentModule = value;
}
