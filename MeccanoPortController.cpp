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
  engineTicker = new RtosTimer(callback(this, &MeccanoPortController::ioEngine));
  portReceiver = new MeccanoPortReceiver(a_moduleDataIn);

  *portEnable = 1;
   
  controllerState = MeccanoPortController::MODULE_DISCOVERY; 
  currentModule = 0;

  checkSum = 0;

  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(0, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(1, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(2, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );
  m_smartModulesMap.insert ( std::pair<int, MeccanoSmartModule>(3, MeccanoSmartModule(MeccanoSmartModule::M_NONE, 0xFE)) );

//  engineTicker->start(200);
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

uint8_t MeccanoPortController::sendData()
{

  *portEnable = 1;
  portReceiver->resetFSM();

  checkSum = calculateCheckSum(m_smartModulesMap.at(0).m_outputData,
                          m_smartModulesMap.at(1).m_outputData,
                          m_smartModulesMap.at(2).m_outputData,
                          m_smartModulesMap.at(3).m_outputData,
                          currentModule);

  //Send start byte
  moduleDataOut->putc(MeccanoPortController::HEADER_BYTE);

  //Send channel data
  for (int modCount = 0; modCount < 4; ++modCount)
    moduleDataOut->putc(m_smartModulesMap.at(modCount).m_outputData);        	          	  	
  
  //Send checksum
  moduleDataOut->putc(checkSum);

  //Thread::wait(8);
  
  //Enable receiver  
  wait(0.010);
  *portEnable = 0;
  portReceiver->enableReceiver();

  wait(0.014);
  portReceiver->disableReceiver();
  *portEnable = 1;

  uint8_t receivedData = portReceiver->getReceivedData();

  if (portReceiver->isDataReady())
  {
    
    if (receivedData == 0xFE)
      setCommand(currentModule, 0xFC);

    //if (m_smartModulesMap.at(currentModule).m_outputData <= 0xE8)
    //  setInputData(currentModule, receivedData & 0x7F);
    //else
      setInputData(currentModule, receivedData);

  }
  else
    setInputData(currentModule, 0);  
  
  /*
  currentModule++;
  if (currentModule > 3)
  {
    currentModule = 0;
  }
*/
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

void MeccanoPortController::ioEngine()
{
  
         
/*  
    switch(controllerState)
    {
      case MODULE_DISCOVERY:
        setCommand(currentModule, MeccanoPortController::ID_NOT_ASSIGNED);
        controllerState = MeccanoPortController::GET_DISCOVERY_RESPONSE;

        break;
      case GET_DISCOVERY_RESPONSE:
          setInputData(currentModule, portReceiver->getReceivedData());

          if (m_smartModulesMap.at(currentModule).m_inputData == 249)
          {
            setPresence(currentModule, true);
          }     
          else
          {
            setPresence(currentModule, false);
          }     
//controllerState = MeccanoPortController::MODULE_TYPE_DISCOVERY;


          if (currentModule < 3)
          {
            controllerState = MeccanoPortController::MODULE_DISCOVERY;
            ++currentModule;
            
          }
          else
          {
            //controllerState = MeccanoPortController::MODULE_DISCOVERY;
            currentModule = 0;
            controllerState = MeccanoPortController::MODULE_TYPE_DISCOVERY;
            
          }                      
          
        break;        
      case MODULE_TYPE_DISCOVERY:
        setCommand(currentModule, MeccanoPortController::REPORT_TYPE);
        controllerState = GET_TYPE_RESPONSE;

        break;
      case GET_TYPE_RESPONSE: 
         setInputData(currentModule, portReceiver->getReceivedData());

         if (m_smartModulesMap.at(currentModule).m_isPresent)   
         {
           setType(currentModule, (MeccanoSmartModule::TYPE_t)m_smartModulesMap.at(currentModule).m_inputData);
         }                 
         //controllerState = MeccanoPortController::MODULE_IDLE;

         
         if (currentModule < 3)
         {
           currentModule++;
           controllerState = MeccanoPortController::MODULE_TYPE_DISCOVERY;
         }
         else
         {
           currentModule = 0;
           controllerState = MeccanoPortController::MODULE_IDLE;
         }
        
        break;       
      case MODULE_IDLE:      
          if (m_smartModulesMap.at(currentModule).m_isPresent)   
            setInputData(currentModule, portReceiver->getReceivedData());

          if (currentModule < 3)
          {
            currentModule++;
          }
          else
          {
            currentModule = 0;
          }  
        
      default:
        break;  
    }
  */ 
}